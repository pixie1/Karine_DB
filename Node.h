#pragma once
#ifndef NODE_H
#define NODE_H
#include "RID.h"
//#include "Tuple.h"
#include<fstream>

class Node
{ 
public:
	
	Node();
	virtual ~Node();
	virtual RID insert(BufferPool&, Tuple, long int&, RID) = 0;
	virtual RID insert(BufferPool&, RID, RID, long int&) = 0;
	virtual RID traverse(Tuple) = 0; 
	virtual RID traverse(Tuple, size_t&) = 0;
	virtual RID getPointer(int) = 0;
	virtual void checkNode(void) = 0;
	size_t size; 
	virtual void shiftLR(BufferPool&, RID, RID, RID) = 0;  //shiftLR(leftN, leftA)
	virtual void shiftRL(BufferPool&, RID, RID, RID) = 0;  //shiftLR(rightN, rightA)
	virtual long int updateAnchorShift(BufferPool&, RID, Node*, Node*, long int) = 0; //after shift on datanode
	virtual long int updateAnchorMerge(BufferPool&, RID, Node*, Node*) = 0; // for update after merge on data node
	virtual long int getKey(int)=0;
	virtual Tuple getData(int) = 0;
	virtual void mergeRight(BufferPool&, RID, RID, RID) = 0;
	virtual void mergeLeft(BufferPool&, RID, RID, RID) = 0;
	//virtual char* writeNode() = 0;
	virtual void readNode(char*) = 0;
	virtual void updateNode(BufferPool&, RID) = 0;
	friend std::ostream& operator << (std::ostream& out, const Node& source){
		return out;
	};
};

#endif