#include "BufferPool.h"
#include "RID.h"
#include "derivedNode.h"
#include <string>
#include<iostream>
#include <fstream>
#include<vector>
#include<cassert>

using namespace std;

bufferNode::bufferNode(){  pos = 0;  dirtyBit = false; } 
bufferNode::bufferNode(RID r, char* N, bool bit){ 
	pos = r; 
	for (int i = 0; i < BLOCKSIZE; i++){
		input[i] = N[i];
	}
	dirtyBit = bit; 
}

BufferPool::BufferPool(){
	buffer = new bufferNode[BUFFERSIZE]; 
}

BufferPool::~BufferPool(){ 
	delete[]buffer; 
}

BufferPool::BufferPool (std::string filename){

	is.open(filename, ios::in | ios::out | ios::binary);
	is.seekp(0, ios::beg);
	blockDef output;
	is.read(reinterpret_cast<char*>(&output), sizeof(blockDef));
	maxSize = output.blocks / 10;
	if (maxSize > BUFFERSIZE) maxSize = BUFFERSIZE;
  	buffer = new bufferNode[maxSize];
	bufferCount = 0;

}

char* BufferPool::getNodeBuffer(RID r){
	//lookup the RID in buffer
	int found = -1;
	for (int i = 0; i < maxSize; ++i){
		if (buffer[i].pos.id == r.id){
			countBufferAccess++;
			return buffer[i].input;
		}
	}
	//node not in cache, read it from disk
	countDiskAccess++;
	is.seekp(0, ios::beg);
	is.seekp(BLOCKSIZE*r.id, std::ios::beg);
	char temp[512];
	is.read(temp, BLOCKSIZE);
	//store in the cache
	storeInBuffer(temp, r, false);
	for (int i = 0; i < maxSize; ++i){
		if (buffer[i].pos.id == r.id)
			return buffer[i].input;
	}
}

void BufferPool::storeInBuffer(char* N, RID r, bool mod){
	//find if node already in buffer
	int i = 0;
	if (mod) ++writeBuffer;
	while (buffer[i].pos.id != r.id&& i<maxSize)
		i++;
	if (i < maxSize){
		buffer[i] = bufferNode(r, N, mod);
		return;
	}
	if (bufferCount < maxSize){
		//there is still space in buffer
		i = 0;
		while (buffer[i].pos.id != 0)
			i++;
		buffer[i] = bufferNode(r, N, mod);		
		++bufferCount;
	}
	else{
		//find position to replace
		i = replace();
		if (buffer[i].dirtyBit)
			storeToDisk(buffer[i].input, buffer[i].pos);
		buffer[i] = bufferNode(r, N,mod );
	}
}
void BufferPool::storeToDisk(char* input, RID r){

	assert(r.id != 0);
	++countDiskAccess;
	++writeDisk;
	is.seekp(r.id*BLOCKSIZE, ios::beg);
	is.write(input, BLOCKSIZE);
}

int BufferPool::replace(){
	//random cache replacement
	return rand() % maxSize;
}

int BufferPool::replace2(){
	//prioritize removing datanode over index node
	vector<int> leaf;
	for (int i = 0; i < maxSize; ++i){
		if (buffer[i].input[0] == 'd')
			leaf.push_back(i);
	}
	if (leaf.size() == 0){
		return rand() % maxSize;
	}
	else{
		int j = rand() % leaf.size();
		return leaf[j];
	}
}

void BufferPool::deleteNode(RID r){
	//make sure RID is in cache
	int i = 0;
	while (buffer[i].pos.id != r.id && i<maxSize)
		i++;
	//delete buffer[i].node;
	if (i < maxSize){
		buffer[i].pos = 0;
		--bufferCount;
	}
}

BufferPool& BufferPool::operator=(BufferPool& source) {
	for (int i = 0; i < maxSize; ++i){
		buffer[i] = source.buffer[i];
	}
	is.swap(source.is);
	maxSize = source.maxSize;
	bufferCount = source.bufferCount;
	bufferCount = source.bufferCount;
	return *this;
}



