#include "derivedNode.h"
#include <cassert>
#include <iostream>
#include "BufferPool.h"
using namespace std;

static int countNode = 0;
IndexNode::IndexNode()
{
	size = I_MINKEY;
//	countNode++;
//	cout << "create indexNode"<<countNode << endl;
}

IndexNode::~IndexNode()
{
	//cout << "delete index: "<<countNode << endl;
//	countNode--;
}

//fill up first value in root node
RID IndexNode::insertRoot(BufferPool& storage, RID N, RID P, long int minMax){
	key[0] = minMax;
	nodePtr[0] = N;
	nodePtr[1] = P;
	size = 1;
	//save node to file
	RID freeRID = RID::getFreeBlock(storage.is);
	freeRID.updateFreeList(storage.is, 1);
	this->updateNode(storage, freeRID);
	return freeRID;
}

RID IndexNode::insert(BufferPool& storage, RID current, RID N, long int& minMax)
{
	RID SplitNode;
	//compare size to MaxKey
	if (size < I_MAXKEY)
	{
		//insert key and pointer in correct spot
		int i = size - 1;
		bool done = false;
		while (i >= 0 && !done)	{
			if (minMax>key[i]){
				key[i + 1] = minMax;
				nodePtr[i + 2] = N;
				done = true;
			}
			else{
				//shift
				key[i + 1] = key[i];
				nodePtr[i + 2] = nodePtr[i+1];
				i--;
			}
		}
		if (!done){
			key[0] = minMax;
			nodePtr[1] = N;
		}
		size++;
		//write node to file
		this->updateNode(storage, current);
		return 0;
	}
	else{
		//node will split
		if (minMax < key[I_MAXKEY / 2 - 1]){
			SplitNode = unevenSplit(storage, N, minMax);
			this->updateNode(storage, current);			
			return SplitNode;
		}
		else{
			SplitNode = evenSplit(storage, N, minMax);
			this->updateNode(storage, current);
			return SplitNode;
		}
	}
}

RID IndexNode::evenSplit(BufferPool& storage, RID N, long int& minMax)
{
	IndexNode* newNode = new IndexNode();
	//copied content to create node
	for (size_t i = size / 2; i < size; i++){
		newNode->key[i - size / 2] = key[i];
		newNode->nodePtr[i - size / 2 + 1] = nodePtr[i + 1];
	}
	//update size 
	size = I_MAXKEY / 2;
	newNode->size = size;
	//insert minMax and pointer in place
	int j = size - 1;
	bool done = false;
	while (j>= 0 && !done){
		if (minMax>newNode->key[j])	{
			newNode->key[j+ 1] = minMax;
			newNode->nodePtr[j+ 2] = N;
			done = true;
		}
		else{
			//shift
			newNode->key[j + 1] = newNode->key[j];
			newNode->nodePtr[j + 2] = newNode->nodePtr[j + 1];
			j--;
		}
	}
	if (!done){
		newNode->key[0] = minMax;
		newNode->nodePtr[1] = N;
	}
	minMax = newNode->key[0];
	//removes minMax from the newNode
	for (size_t i = 0; i < size; i++){
		newNode->key[i] = newNode->key[i + 1];
		newNode->nodePtr[i] = newNode->nodePtr[i + 1];
	}
	newNode->nodePtr[size] = newNode->nodePtr[size + 1];
	RID freeRID = RID::getFreeBlock(storage.is);
	freeRID.updateFreeList(storage.is, 1);
	newNode->updateNode(storage, freeRID);
	delete newNode;
	return freeRID;
}

RID IndexNode::unevenSplit(BufferPool& storage, RID N, long int& minMax)
{
	//create new node
	IndexNode* newNode = new IndexNode();
	//copied content to created node
	for (size_t i = size / 2-1; i < size; i++)
	{
		newNode->key[i - size / 2+1] = key[i];
		newNode->nodePtr[i - size / 2 + 2] = nodePtr[i + 1];
	}
	//update size 
	size = I_MAXKEY / 2-1;
	newNode->size = size+2;
	//set minMax and pointer in place

	if (minMax>key[0])
	{
		key[size] = minMax;
		nodePtr[size + 1] = N;
	}
	else
	{
		key[size] = key[0];
		key[0] = minMax;
		nodePtr[size + 1] = nodePtr[size];
		nodePtr[size] = N;
	}
	size++;
	minMax = newNode->key[0];
	//removes minMax from the newNode
	for (size_t i = 0; i < size; i++)
	{
		newNode->key[i] = newNode->key[i + 1];
		newNode->nodePtr[i] = newNode->nodePtr[i + 1];
	}
	newNode->nodePtr[size] = newNode->nodePtr[size + 1];
	newNode->size = I_MAXKEY / 2;
	RID freeRID = RID::getFreeBlock(storage.is);
	freeRID.updateFreeList(storage.is, 1);
	newNode->updateNode(storage, freeRID);
	delete newNode;
	return freeRID;
}

RID IndexNode::traverse(Tuple data)
{
	size_t i = 0; 
	bool found = false;
	while (i < size && found == false)
	{
		if (data.key < key[i])
			found = true;
		else
			i++;
	}
	return nodePtr[i];
}

//traverse the tree, used for delete
//return the next node and the position in the array
RID IndexNode::traverse(Tuple data, size_t& pos)
{
	size_t i = 0;
	bool found = false;
	while (i < size && found == false)
	{
		if (data.key < key[i])
			found = true;
		else
			i++;
	}
	pos = i;
	return nodePtr[i];
}

ostream& operator << (ostream& out, const IndexNode& source)
{
	out << "(*";
	for (size_t i = 0; i < source.size; i++)
	{
		out << " , "<<source.key[i] << ", * ";
	}
	out << ")";
	return out;
}

RID IndexNode::getPointer(int i)
{
	return nodePtr[i];
}

void IndexNode::checkNode(void)
{
	assert(size <= I_MAXKEY);
	assert(size >= I_MINKEY);
	for (size_t i = 0; i < size - 1; i++)
		assert(key[i] < key[i + 1]);
}

long int IndexNode::updateAnchorShift(BufferPool& storage, RID anchor, Node* current, Node* neighbor, long int minMax)
{
	long int pivot=0;
	//look for pivot
	for (size_t i = 0; i <size; i++)
	{
		if (key[i]>current->getKey(0) && key[i] <= neighbor->getKey(neighbor->size-1) || (key[i]<=current->getKey(current->size-1) && key[i] > neighbor->getKey(0)))

		{	pivot = key[i];
			key[i]= minMax ;
			break;
		}
	}
	//storage.updateNode(this, anchor);
	this->updateNode(storage, anchor);
	return pivot;
}


long int IndexNode:: getKey(int i)  //return the key at position i
{
	return key[i];
}

long int IndexNode::updateAnchorMerge(BufferPool& storage, RID anchor, Node* current, Node* neighbor){
	int pivotIndex=-1;
	long int pivot=-1;
	//look for pivot
	for (size_t i = 0; i <size; ++i)
	{
		if (key[i]>current->getKey(0) && key[i] <= neighbor->getKey(neighbor->size-1) || (key[i]<=current->getKey(current->size-1) && key[i] > neighbor->getKey(0))){
			pivot = key[i];
			pivotIndex = i;
			nodePtr[i + 1] = 0;
			break;
		}
	}
	//shift keys to remove pivot
	for (size_t i = pivotIndex; i < size - 1; ++i){
		key[i] = key[i + 1];
		nodePtr[i+1] = nodePtr[i + 2];
	}
	--size;
	
	this->updateNode(storage, anchor);
	return pivot;
}

void IndexNode::shiftLR(BufferPool& storage, RID current, RID leftN, RID leftA)
{
	IndexNode* N = static_cast<IndexNode*>(leftN.getNode(storage));
	
	long int max = N->key[N->size - 1];

	//shift values in current node
	for (size_t i = size; i > 0; i--){
	key[i] = key[i - 1];
	nodePtr[i + 1] = nodePtr[i];
	}
	nodePtr[1] = nodePtr[0];
	key[0] = max;
	nodePtr[0] = N->nodePtr[N->size];
	--N->size;
	++size;
	long int minMax = key[0];//minimum from current
	IndexNode* leftANode = static_cast<IndexNode*>(leftA.getNode(storage));
	long int pivot=leftANode->updateAnchorShift(storage, leftA, this, N, minMax);//set pivot to min
	key[0] = pivot;//finish the switch
	this->updateNode(storage, current);
	N->updateNode(storage, leftN);
	delete N;
	delete leftANode;
	return;
}

void IndexNode::shiftRL(BufferPool& storage, RID current,RID rightN, RID rightA)
{
	IndexNode* N = static_cast<IndexNode*>(rightN.getNode(storage));
	
	long int min = N->key[0];

	key[size] = min;
	++size;
	long int minMax = N->getKey(0);
	IndexNode* rightANode = static_cast<IndexNode*>(rightA.getNode(storage));
	long int pivot=rightANode->updateAnchorShift(storage, rightA, this, N, minMax);
	key[size - 1] = pivot;
	nodePtr[size] = N->nodePtr[0];
	//shift values and ptr in neighbor
	for (size_t i = 0; i < N->size; i++){
		N->key[i] = N->key[i + 1];
		N->nodePtr[i] = N->nodePtr[i + 1];
	}
	--N->size;
	this->updateNode(storage, current);
	N->updateNode(storage, rightN);
	delete N;
	delete rightANode;
	return;
}

void IndexNode::mergeLeft(BufferPool& storage, RID current, RID leftN, RID leftA){
	//copy current into left neighbor
	IndexNode* N = static_cast<IndexNode*>(leftN.getNode(storage));
	IndexNode* leftANode = static_cast<IndexNode*>(leftA.getNode(storage));
	long int pivot = leftANode->updateAnchorMerge(storage, leftA, this, N);
	N->key[N->size] = pivot;
	for (size_t i = 0; i < size;i++){
		N->key[N->size + 1 + i] = key[i];
		N->nodePtr[N->size + 1 + i] = nodePtr[i];
	}
	N->nodePtr[size + N->size + 1] = nodePtr[size];
	N->size += size+1;
	size = 0;
	N->updateNode(storage, leftN);
	current.updateFreeList(storage.is, 0);
	storage.deleteNode(current);
	delete N;
	delete leftANode;
}

void IndexNode::mergeRight(BufferPool& storage, RID current, RID rightN, RID rightA){
	
	IndexNode* N = static_cast<IndexNode*>(rightN.getNode(storage));
	IndexNode* rightANode = static_cast<IndexNode*>(rightA.getNode(storage));

	long int pivot = rightANode->updateAnchorMerge(storage, rightA, this, N);
	key[size] = pivot;
	for (size_t i = 0; i < N->size; i++){
		key[size + 1 + i] = N->key[i];
		nodePtr[size + 1 + i] = N->nodePtr[i];
	}
	nodePtr[size + N->size + 1] = N->nodePtr[N->size];
	size += N->size+1;
	N->size = 0;
	
	this->updateNode(storage, current);
	rightN.updateFreeList(storage.is, 0);
	storage.deleteNode(rightN);
	delete N;
	delete rightANode;
}

void IndexNode::updateNode(BufferPool& storage, RID current){

	indexNodeRec node;
	node.id = 'i';
	node.size = this->size;
	for (size_t i = 0; i < size; i++){
		node.key[i] = key[i];
		node.nodePtr[i] = nodePtr[i];
	}
	node.nodePtr[size] = nodePtr[size];
	char* output = reinterpret_cast<char*>(&node);
	storage.storeInBuffer(output, current, true);
}
//interpret char* as an index node
void IndexNode::readNode(char* input){
	indexNodeRec* nodeStruct;
	nodeStruct = reinterpret_cast<indexNodeRec*>(input);
	size = nodeStruct->size;
	for (size_t i = 0; i < size; i++){
		key[i] = nodeStruct->key[i];
		nodePtr[i] = nodeStruct->nodePtr[i];
	}
	nodePtr[size] = nodeStruct->nodePtr[size];
}

void IndexNode::getNode(char* input){
	indexNodeRec* nodeStruct;
	nodeStruct = reinterpret_cast<indexNodeRec*>(input);
	size = nodeStruct->size;
	for (size_t i = 0; i < size; i++){
		key[i] = nodeStruct->key[i];
		nodePtr[i] = nodeStruct->nodePtr[i];
	}
	nodePtr[size] = nodeStruct->nodePtr[size];
}