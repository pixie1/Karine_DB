#pragma once
#ifndef RID_H
#define RID_H

#include <fstream>
#include <climits>
#include "Tuple.h"


int const BLOCKSIZE = 512;
int const FREELISTSIZE = 120;
int const BUFFERSIZE = 20;

const int D_MAXKEY = 4;
const int D_MINKEY = D_MAXKEY / 2;
const int I_MAXKEY = 4;
const int I_MINKEY = I_MAXKEY / 2;

struct blockDef{
	int blocks;
	unsigned int freeBlocks[FREELISTSIZE];
	int rootRID;
	int headRID;
};	

struct nodeInfo {
	char type;
	int size;
};


class Node;
class IndexNode;
class DataNode;
class BufferPool;

class RID
{

public:

	int id;
	RID(){ id = 0; };
	RID(int);
	char getType(BufferPool&);
	size_t getSize(BufferPool&);
	Node* getNode(BufferPool&);
	DataNode* getDataNode(BufferPool&);
	IndexNode* getIndexNode(BufferPool&);
	Tuple getData(BufferPool&,int);
	RID getPointer(BufferPool&, int);
	RID insert(BufferPool&,  RID, long int&);
	RID insert(BufferPool&, Tuple, long int&, RID);
	RID insertRoot(BufferPool&, RID, RID, long int);
	RID traverse(BufferPool&, Tuple);
	RID traverse(BufferPool&, Tuple, size_t&);
	bool lookUp(BufferPool&, Tuple& );
	void deleteTuple(BufferPool&, Tuple);
	void shiftLR(BufferPool&, RID, RID);  //shiftLR(leftN, leftA)
	void shiftRL(BufferPool&, RID, RID);
	void mergeRight(BufferPool&, RID, RID);
	void mergeLeft(BufferPool&, RID, RID);
	static int getFreeBlock(std::fstream&);
	void updateFreeList(std::fstream&, int);
	static void updateRoot(std::fstream&, RID);
	static int getUsedBlocks(std::fstream&, int& numBlocks);
};
#endif
