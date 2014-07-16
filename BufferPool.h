#pragma once
#include<fstream>
#include<string>
#include"RID.h"
//class RID;
class Node;

extern int countBufferAccess;
extern int countDiskAccess;
extern int writeDisk;
extern int writeBuffer;

struct bufferNode{
	RID pos;
	char input[512];
	//Node* node;
	bool dirtyBit;
	bufferNode();
	bufferNode(RID, char*, bool);
};

class BufferPool //contains both the buffer and the file
{

public:
	const int BUFFERSIZE = 200;
	int maxSize;
	bufferNode* buffer;
	std::fstream is;
	int bufferCount;
	BufferPool();
	BufferPool(std::string);
	~BufferPool();
	char* getNodeBuffer(RID);
	void storeInBuffer(char*, RID, bool);
	int replace();
	int replace2();
//	void updateNode(Node*, RID);
	void deleteNode(RID);
	void storeToDisk(char*, RID);
	BufferPool& operator=(BufferPool&);
};


