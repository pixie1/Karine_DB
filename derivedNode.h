#pragma once

//#include "IndexNode.h"
//#include "DataNode.h"

#include "Node.h"
//#include "Tuple.h"
#include <ostream>
#include<queue>
// set maxKey and minKey for data node and index node, put different constant minKey=maxKey/2;


class DataNode : public Node
{
private:
	Tuple content[D_MAXKEY];
	void sort(DataNode*);
	RID next;
	
	struct dataNodeRec{
		char id;
		int size;
		Tuple content[D_MAXKEY];
		RID next;
	};
public:
	virtual RID insert(BufferPool&, Tuple, long int&, RID);
	virtual RID traverse(Tuple) { return 0; };
	virtual RID traverse(Tuple, size_t&){ return 0; };
	virtual RID insert(BufferPool&, RID, RID, long int&) { return 0; };
	friend std::ostream& operator << (std::ostream& out, const DataNode& source);
	void addToQueue(std::queue<char*>&, int);
	DataNode();
	~DataNode();
	RID getPointer(int);
	//	size_t size;
	bool lookUp(Tuple&);
	void checkNode(void);
	void deleteTuple(BufferPool&, RID, Tuple);

	void shiftLR(BufferPool&, RID, RID, RID);
	void shiftRL(BufferPool&, RID, RID, RID);
	long int updateAnchorShift(BufferPool& , RID, Node*, Node*, long int){ return -1; };
	virtual long int updateAnchorMerge(BufferPool&, RID, Node*, Node*){ return -1; }; // for update after merge on data node
	virtual long int getKey(int);
	Tuple getData(int);
	virtual void mergeRight(BufferPool&, RID, RID, RID);
	virtual void mergeLeft(BufferPool&, RID, RID, RID);
	//virtual char* writeNode();
	//void writeNode(std::fstream&, RID);
	void readNode(char*);
	void updateNode(BufferPool&, RID);
	void updateNode(std::fstream&, RID);
	void getNode(char*);
};

class IndexNode : public Node
{
private:

	RID unevenSplit(BufferPool&, RID, long int&);
	RID evenSplit(BufferPool&, RID, long int&);
	RID nodePtr[I_MAXKEY + 1];
	long int key[I_MAXKEY];
	struct indexNodeRec{
		char id;
		int size;
		long int key[I_MAXKEY];
		RID nodePtr[I_MAXKEY + 1];
	};

public:
	IndexNode();
	~IndexNode();
	virtual RID traverse(Tuple);
	virtual RID traverse(Tuple, size_t&);
	virtual RID insert(BufferPool&, Tuple, long int&, RID){ return 0; };
	virtual RID insert(BufferPool&, RID, RID, long int&);
	RID insertRoot(BufferPool&, RID, RID, long int); //used to create new root in case of split
	friend std::ostream& operator << (std::ostream& out, const IndexNode& source);

	RID getPointer(int);
	void checkNode(void);
	virtual void rebalance(int, Node*, Node*, Node*, Node*, int, int, long int){};
	virtual void shiftLR(BufferPool&, RID, RID, RID);
	virtual void shiftRL(BufferPool&, RID, RID, RID);
	long int updateAnchorShift(BufferPool&, RID, Node*, Node*, long int);	// for update after shift
	long int updateAnchorMerge(BufferPool&, RID, Node*, Node*); //for update after merge on index node
	virtual long int getKey(int);
	virtual Tuple getData(int){ return 0; };
	virtual void mergeRight(BufferPool&, RID, RID, RID);
	virtual void mergeLeft(BufferPool&, RID, RID, RID);
	//virtual char* writeNode();
	void readNode(char*);
	void getNode(char*);
	void updateNode(BufferPool&, RID);
};