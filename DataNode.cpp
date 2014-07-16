#include "derivedNode.h"
#include "BufferPool.h"
#include <cassert>
#include <stdexcept>
#include <iostream>
#include<mutex>
#include<condition_variable>
#include<memory>
#include<deque>
using namespace std;
static int countData = 0;
DataNode::DataNode()
{
	size = 0;
	next = 0;
//	countData++;
//	cout << "create data "<< countData<< endl;
}

DataNode::~DataNode()
{
//	cout << "delete data: "<<countData << endl;
//	countData--;
}

RID DataNode::getPointer(int dummy)
{
	return next;
}


RID DataNode::insert(BufferPool& storage, Tuple data, long int& minMax, RID current){
	//check for duplicate
	for (size_t i = 0; i < size; i++){
		if (data.key == content[i].key){
			//data is a duplicate, do nothing, return
			return current;
		}
	}
	if (size < D_MAXKEY){
		content[size] = data;
		size++;
		sort(this);
		//update in the buffer pool
		this->updateNode(storage, current);
		return current;
	}
	else{
		//split
		//check space available in file
		//get number of free blocks
		int numBlocks;
		int used = RID::getUsedBlocks(storage.is, numBlocks);
		int free = numBlocks - used;
		//number of free blocks needed
		if (free != 0 && free != numBlocks){
			int level = 0;
			int count = used;
			while (count > 0){
				count = used - int(pow(5, level));
				level++;
			}
			if (free == 0 || free < level + 1){
				throw runtime_error("Database is full");
				}
		}
		//create new node
		DataNode* N = new DataNode();
		//fill new node with 2 biggest of previous nodes and figure out where to add tuple
		minMax = content[D_MAXKEY / 2].key;
		for (int i = 0; i < D_MAXKEY / 2; i++){
			N->content[i] = content[i + D_MAXKEY / 2];
		}
		if (data.key < minMax){
			N->size = D_MAXKEY / 2;
			size = size - D_MAXKEY/2+1;
			content[size - 1] = data;
			sort(this);
		}
		else{
			size = size/2;
			N->size = size + 1;
			N->content[N->size - 1] = data;
			sort(N);
		}
		N->next = next;
		//return minMax
		minMax = N->content[0].key;
		//write N to disk
		//find location
		RID N_RID = RID::getFreeBlock(storage.is);
		N_RID.updateFreeList(storage.is,1);
		//update linked list
		N->updateNode(storage, N_RID);
		next = N_RID;
		//update current node in the cache
		this->updateNode(storage, current);
		delete N;
		//return pointer to the created node
		return N_RID;
	}
}

void DataNode::sort(DataNode* N){
	for (size_t i = 0; i < N->size; i++){
		for (size_t j = 0; j < N->size - 1; j++){
			if (N->content[j].key>N->content[j + 1].key){
				Tuple temp = N->content[j];
				N->content[j] = N->content[j + 1];
				N->content[j + 1] = temp;
			}
		}
	}
}

bool DataNode::lookUp(Tuple& data){
	for (size_t i=0; i < size; i++)	{
		if (data.key == content[i].key)
			data=content[i];
	}
	return false;
}

ostream& operator << (ostream& out, const DataNode& source)
{
	out << "[ ";
	if (source.size != 0){
		for (size_t i = 0; i < source.size - 1; i++)
		{
			out << source.content[i].key << ",";
		}
		out << source.content[source.size - 1].key;
	}
	out << "]";
	return out;
}

extern deque<mutex> q_sync;
extern deque<condition_variable> q_cond;

void DataNode::addToQueue(std::queue<char*>& q1, int threadNum){
	if (size != 0){
		for (size_t i = 0; i <size; i++)
		{
			unique_lock<mutex> qlock(q_sync[threadNum]);
			char* tempArray=new char[MAXTUPLE];
			for (int j = 0; j < MAXTUPLE; ++j){
				tempArray[j] = content[i].value[j];
			}

			q1.push(tempArray);
			qlock.unlock();
			//q_cond.notify_one();
			q_cond[threadNum].notify_one();
		}
	}
}

void DataNode::checkNode(void)
{
	assert(size <= D_MAXKEY);
	assert(size >= D_MINKEY);
	for (size_t i = 0; i < size - 1; i++)
		assert(content[i].key < content[i-1].key);
}

void DataNode::deleteTuple(BufferPool& buffer, RID current, Tuple data)
{
	for (size_t i = 0; i < size; i++){
		if (content[i].key == data.key){
			for (size_t j = i + 1; j < size; j++){//shift remaining data
				content[j - 1] = content[j];
			}
			size--;
			break; 
		}
	}
	this->updateNode(buffer, current);
}

void DataNode::shiftLR(BufferPool& storage, RID current, RID leftN, RID leftA)
{
	long int minMax;
	DataNode* leftNNode = static_cast<DataNode*>(leftN.getNode(storage));
	Tuple max = leftNNode->content[leftNNode->size - 1];
	--leftNNode->size;
	content[size] = max;
	++size;
	sort(this);
	minMax = content[0].key;
	IndexNode* leftANode = static_cast< IndexNode*>(leftA.getNode(storage));
	leftANode->updateAnchorShift(storage, leftA, this, leftNNode, minMax);
	leftNNode->updateNode(storage, leftN);
	this->updateNode(storage, current);
	delete leftANode;
	delete leftNNode;
	return;
}

void DataNode::shiftRL(BufferPool& storage, RID current, RID rightN, RID rightA)
{
	DataNode* rightNNode = static_cast<DataNode*>(rightN.getNode(storage));
	content[size] = rightNNode->content[0];
	++size;
	sort(this);
	rightNNode->content[0] = rightNNode->content[rightNNode->size - 1];
	rightNNode->size--;
	sort(rightNNode);
	long int minMax = rightNNode->content[0].key;
	IndexNode* rightANode = static_cast< IndexNode*>(rightA.getNode(storage));
	rightANode->updateAnchorShift(storage, rightA, this, rightNNode, minMax);
	rightNNode->updateNode(storage, rightN);
	this->updateNode(storage, current);
	delete rightANode;
	delete rightNNode;
	return;
}

void DataNode::mergeLeft(BufferPool& storage, RID current, RID leftN, RID leftA)
{
	//copy current into left neighbor
	DataNode* N = static_cast<DataNode*>(leftN.getNode(storage));
	IndexNode* leftANode = static_cast<IndexNode*>(leftA.getNode(storage));
	leftANode->updateAnchorMerge(storage, leftA, this, N);
	N->content[N->size] = content[0];
	N->size += size;
	size = 0;
	N->next = next;	
	N->updateNode(storage, leftN);
	current.updateFreeList(storage.is, 0);
	storage.deleteNode(current);
	delete N;	
	delete leftANode;
}

void DataNode::mergeRight(BufferPool& storage, RID current, RID rightN, RID rightA)
{
	DataNode* N = static_cast<DataNode*>(rightN.getNode(storage));
	IndexNode* rightANode = static_cast<IndexNode*>(rightA.getNode(storage));
	rightANode->updateAnchorMerge(storage, rightA, this, N);
	for (size_t i = 0; i < N->size; i++)
		content[size+i] = N->content[i];
	next = N->next;
	size += N->size;
	N->size = 0;
	this->updateNode(storage, current);
	rightN.updateFreeList(storage.is, 0);
	storage.deleteNode(rightN);
	delete N;
	delete rightANode;
}

long int DataNode::getKey(int pos)
{
	return content[pos].key;
}

Tuple DataNode::getData(int pos)
{
	return content[pos];
}

void DataNode::readNode(char* input){
	dataNodeRec* nodeStruct;
	nodeStruct = reinterpret_cast<dataNodeRec*>(input);
	size = nodeStruct->size;
	for (size_t i = 0; i < size; i++){
		content[i] = nodeStruct->content[i];
	}
	next = nodeStruct->next;
}

void DataNode::updateNode(BufferPool& storage, RID current){
	dataNodeRec node;
	node.id = 'd';
	node.size = this->size;
	for (size_t i = 0; i < size; i++){
		node.content[i] = content[i];
	}
	node.next = next;
	char* output = reinterpret_cast<char*>(&node);
	storage.storeInBuffer(output, current, true);
}

void DataNode::updateNode(fstream& is, RID current){
	dataNodeRec node;
	node.id = 'd';
	node.size = this->size;
	for (size_t i = 0; i < size; i++){
		node.content[i] = content[i];
	}
	node.next = next;
	is.seekp(0, ios::beg);
	is.seekp(current.id*BLOCKSIZE, ios::cur);
	is.write(reinterpret_cast<char*>(&node), BLOCKSIZE);
}

void DataNode::getNode(char* input){
	dataNodeRec* nodeStruct;
	nodeStruct = reinterpret_cast<dataNodeRec*>(input);
	size = nodeStruct->size;
	for (size_t i = 0; i < size; i++){
		content[i] = nodeStruct->content[i];
	}
	next = nodeStruct->next;
}