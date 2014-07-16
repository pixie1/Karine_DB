#pragma once
#ifndef TREE_H
#define TREE_H

#include "derivedNode.h"
#include "BufferPool.h"
#include <vector>
#include <fstream>
#include <string>
#include <queue>

class Tree
{
private:
	BufferPool storage;
	RID root;
	
	RID head; //head of linked list for the leaves

	RID insert(Tuple, RID, long int&);
	void printList( RID);
	void printTree(RID, int);
	void printTree( RID, int, std::ofstream&);
//	void destroy(Node*);
	void deleteData(Tuple, RID, RID, RID, RID, RID, int, int, int);
	void rebalance(int, RID, RID, RID, RID, RID, int, int, long int);
	void getTreeInfo();
	void printFreeList(void);
	void lookUpList(std::queue<char*>&, int, RID);

public:

	Tree();
	Tree(std::string);
	~Tree();
	void insert(Tuple);
	void printList();
	void printTree();
	Tuple lookUp( long int);
	bool lookUp(Tuple&, RID);
	void deleteData(Tuple);
	bool empty();
	void deleteAll(void);
	static void writeEmptyRoot(std::fstream&);
	void lookUpList(std::queue<char*>&, int);//int: thread number, to know what queue to lock

};

#endif