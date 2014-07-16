#include "RID.h"
#include "derivedNode.h"
#include "BufferPool.h"

using namespace std;

RID::RID(int id)
{
	this->id = id;
}

Node* RID::getNode(BufferPool& buffer)
{
	char* input = buffer.getNodeBuffer(*this);
	if (input[0] == 'i'){
		IndexNode* temp = new IndexNode();
		temp->getNode(input);
		return temp;
	}
	else{
		DataNode* temp = new DataNode();
		temp->getNode(input);
		return temp;
	}
}

char RID::getType(BufferPool& buffer){
	char* temp = buffer.getNodeBuffer(*this);
	return temp[0];
}

size_t RID::getSize(BufferPool& buffer){
	char* temp = buffer.getNodeBuffer(*this);
	nodeInfo input;
	input=*(reinterpret_cast<nodeInfo*>(temp));
	return input.size;
}

RID RID::getPointer(BufferPool& buffer, int pos){
	//create node
	Node* temp = getNode(buffer);
	RID pointer = temp->getPointer(pos);
	delete temp;
	return pointer;
}

Tuple RID::getData(BufferPool& buffer, int pos){
	Node* temp = getNode(buffer);
	Tuple data = temp->getData(pos);
	delete temp;
	return data;
}

RID RID::insert(BufferPool& buffer, Tuple data, long int& minMax, RID current){
//	DataNode* temp = new DataNode();
	Node* temp= getNode(buffer); 
	RID newNodeRID = temp->insert(buffer, data, minMax, current);
	delete temp;
	return newNodeRID;
}

RID RID::insert(BufferPool& storage, RID newNodeRID, long int& minMax){
	Node* temp = getNode(storage);
	RID newRID = temp->insert(storage, *this, newNodeRID, minMax);
	//delete temp;
	return newRID;
}

RID RID::insertRoot(BufferPool& storage, RID N, RID P, long int minMax){
	IndexNode* temp = new IndexNode();
	RID newRID=temp->insertRoot(storage, N, P, minMax);
	delete temp;
	return newRID;
}

RID RID::traverse(BufferPool& storage, Tuple data){
	
	Node* temp = getNode(storage);
	RID child = temp->traverse(data);
	delete temp;
	return child;
}

RID RID::traverse(BufferPool& storage, Tuple data, size_t& pos){
	Node* temp = getNode(storage);
	RID child = temp->traverse(data, pos);
	delete temp;
	return child;
}

bool RID::lookUp(BufferPool& storage, Tuple& data){
	DataNode* temp = static_cast<DataNode*>(getNode(storage));
	bool test = temp->lookUp(data);
	delete temp;
	return test;
}
void RID::deleteTuple(BufferPool& storage, Tuple data){

	DataNode* temp = static_cast<DataNode*>( getNode(storage));
	temp->deleteTuple(storage, *this, data);
	delete temp;
	return;
}

void RID::shiftLR(BufferPool& storage, RID leftN, RID leftA){
	
	Node* temp = getNode(storage);
	temp->shiftLR(storage, *this, leftN, leftA);
	delete temp;
	return;
}
void RID::shiftRL(BufferPool& storage, RID rightN, RID rightA){
	Node* temp = getNode(storage);
	temp->shiftRL(storage, *this, rightN, rightA);
	delete temp;
	return;
}
void RID::mergeRight(BufferPool& storage, RID leftN, RID leftA){
	Node* temp = getNode(storage);
	temp->mergeRight(storage, *this, leftN, leftA);
	//delete temp;
	return;
}
void RID::mergeLeft(BufferPool& storage, RID rightN, RID rightA){
	Node* temp = getNode(storage);
	temp->mergeLeft(storage, *this, rightN, rightA);
	delete temp;
	return;
}

int RID::getFreeBlock(std::fstream& is){
	is.seekp(0, std::ios::beg);
	blockDef output;
	is.read(reinterpret_cast<char*>(&output), sizeof(blockDef));
	unsigned int mask = 1;
	for (int i = 0; i < FREELISTSIZE*sizeof(int)*CHAR_BIT; i++){
		if (i%(sizeof(unsigned int)*CHAR_BIT) == 0)
			mask = 1;
	if (!(output.freeBlocks[i / (sizeof(unsigned int)*CHAR_BIT)] & mask)){
		return i+1;
		}
		mask = mask << 1;
	}
	return 0;
}

void RID::updateFreeList(std::fstream& is, int i){
	is.seekp(0, std::ios::beg);
	blockDef output;
	is.read(reinterpret_cast<char*>(&output), sizeof(blockDef));
	int blockPos = (id-1) / (sizeof(int)*CHAR_BIT);
	int bitPos = (id-1)% (sizeof(int)*CHAR_BIT);
	if (i == 1)
	{//set corresponding free block to 1
		output.freeBlocks[blockPos] |= 1u << bitPos;
	}
	else
		//set block to 0
		output.freeBlocks[blockPos] &= ~(1u << bitPos);
	//write to disk
	is.seekp(0, std::ios::beg);
	is.write(reinterpret_cast<const char*>(&output), sizeof(blockDef));
}

void RID::updateRoot(std::fstream& is, RID N){
	is.seekp(0, std::ios::beg);
	blockDef output;
	is.read(reinterpret_cast<char*>(&output), sizeof(blockDef));
	output.rootRID = N.id;
	is.seekp(0, std::ios::beg);
	is.write(reinterpret_cast<const char*>(&output),sizeof(blockDef));
}

int RID::getUsedBlocks(fstream& is, int& numBlocks){
	is.seekp(0, ios::beg);
	blockDef output;
	is.read(reinterpret_cast<char*>(&output), sizeof(blockDef));
	numBlocks = output.blocks;
	int used = 0;
	unsigned int mask = 1;
	for (int i = 0; i < numBlocks; i++){
		if (i % (sizeof(unsigned int)*CHAR_BIT) == 0)
			mask = 1;
		if (output.freeBlocks[i / (sizeof(unsigned int)*CHAR_BIT)] & mask)
			used++;
		mask = mask << 1;
	}
	return used;
}


