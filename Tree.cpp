#include "Tree.h"
#include <typeinfo>
#include <iomanip>
#include <iostream>
using namespace std;

Tree::Tree()
{
	root = 1; //point head to first datanode created, will never change
	head = 1;
}

Tree::Tree(string filename){

	BufferPool temp(filename);
	storage = temp;
	blockDef output;
	storage.is.seekp(0, ios::beg);
	storage.is.read(reinterpret_cast<char*>(&output), sizeof(blockDef));
	root = output.rootRID;
	head = output.headRID;
}

Tree::~Tree()
{
	writeDisk = 0;
	int i = 0;
	for (int i = 0; i < storage.maxSize; ++i){
		if (storage.buffer[i].dirtyBit && storage.buffer[i].pos.id!=0)
			storage.storeToDisk(storage.buffer[i].input, storage.buffer[i].pos);
	}
	//cout << "tree destructor";
}

bool Tree::empty()
{
	if (root.getSize(storage) == 0)
		return true;
	return false;
}

void Tree::deleteAll(void){

	RID N = head;
	while (N.getSize(storage) != 0)
	{
		deleteData(N.getData(storage, 0));
		
		Node* temp = head.getNode(storage);
		delete temp;
	}
}

void Tree::insert(Tuple data)
{
	//get spot on disk where node will go
	//create new RID
	RID newNodeRID;
	long int minMax = 0;
	//read initial block to read RID for the root
	//getTreeInfo();
	newNodeRID=insert(data, root, minMax);
	//check for root split
	if (newNodeRID.id != 0){
		//root has split
		//create new index node
		IndexNode* temp = new IndexNode();
		RID newRoot = temp->insertRoot(storage, root, newNodeRID, minMax);
		root = newRoot;
		RID::updateRoot(storage.is, root);
		delete temp;
	}
}

RID Tree::insert(Tuple data, RID current,long int& minMax)
{

	RID child;
	RID newNodeRID;
	if (current.getType(storage) == 'd') //the current RID leads to a datanode
	{
		newNodeRID = current.insert(storage, data, minMax, current);
		if (current.id == root.id){
				if (newNodeRID.id!=root.id){
					//split occured, need to create first index node
					RID newRoot = newRoot.insertRoot(storage, root, newNodeRID, minMax);
					root = newRoot;
					RID::updateRoot(storage.is, root);
				}
				newNodeRID = 0;
		}
		return newNodeRID;
	}		
	else{
		child = current.traverse(storage, data);
		newNodeRID=insert(data, child, minMax);
	}
	//check the node all the way up the tree		
	if (newNodeRID.id == child.id || newNodeRID.id==0) //did not split
			return 0; 
	else{
		//split occured
		//insert the minMax
		newNodeRID = current.insert(storage, newNodeRID, minMax);
	}
	return newNodeRID;
}

void Tree::printList(){
	printList(head);
}

void Tree::printList(RID N)
{
	while (N.id != 0)
	{
		Node* temp = N.getNode(storage);
		cout << *(DataNode*)temp << " "<<N.id<<endl;
		N = temp->getPointer(0); 
		delete temp;
	}
}

void Tree::lookUpList(std::queue<char*>& q1, int threadNum){
	lookUpList(q1, threadNum, head);
}

void Tree::lookUpList(std::queue<char*>& q1, int threadNum, RID N){
	while (N.id != 0){
		Node* temp = N.getNode(storage);
		((DataNode*)temp)->addToQueue(q1, threadNum);
		N = temp->getPointer(0);
		delete temp;
	}
}
void Tree::printTree()
{
	int width = 0;
/*	ofstream myfile;
	myfile.open("tree.txt");
	cout << "Writing this to a file.\n";
	printTree(root, width, myfile);
	myfile.close();*/
	//printFreeList();
	printTree( root, width);
}

void Tree::printFreeList(){

	storage.is.seekp(0, std::ios::beg);
	blockDef output;
	storage.is.read(reinterpret_cast<char*>(&output), sizeof(blockDef));
	for (int i = 1; i <= output.blocks; i++)
		cout << i % 10 << ' ';
	cout << endl;
	for (int i = 1; i <= output.blocks; i++){
		int blockPos = (i - 1) / (sizeof(int)*CHAR_BIT);
		int bitPos = (i - 1) % (sizeof(int)*CHAR_BIT);
		cout<<((output.freeBlocks[blockPos] & 1u << bitPos)>>bitPos)<<' ';
	}
	cout << endl;
}

//print to a file (for debugging)
/*void Tree::printTree(Node* N, int width, ofstream& myfile)
{
	if (typeid(*N) == typeid(DataNode))
	{
		myfile << setw(width);
		myfile << *(DataNode*)N << endl;
		return;
	}
	else
	{
		myfile << setw(width);
		myfile << *(IndexNode*)N << endl;
		width = width + 8;
		for (size_t i = 0; i < ((IndexNode*)N)->size + 1; i++)
		{
			if (N != root)N->checkNode(); //debugging
			printTree(N->getPointer(i), width, myfile);
		}
		return;
	}
	return;
}
*/
//tree dump
void Tree::printTree (RID N, int width)
{
	if (N.getType(storage) == 'd')
	{
		Node* P = N.getNode(storage);
		cout << setw(width);
		cout << *(DataNode*)P<<" "<<N.id<<endl;
		delete P;
		return;
	}
	else
	{
		cout << setw(width);
		Node* P = N.getNode(storage);
		cout << *(IndexNode*)P<<" "<<N.id<<endl;
		width = width + 8;
		for (size_t i = 0; i < ((IndexNode*)P)->size+1; i++)
		{
			printTree(P->getPointer(i), width);
		}
		delete P;
		return;
	}
	return;
}

//search value in the tree (for debugging)
Tuple Tree::lookUp(long int key)
{
	Tuple data(key);
	lookUp(data, root);
	return data;
}

bool Tree::lookUp( Tuple& key, RID current){
	
	RID child;
	if (current.getType(storage) == 'd'){
		return current.lookUp(storage, key);
	}
	else{
		child = (current.traverse(storage, key));//P next node down the tree
		return lookUp(key, child);
	}
}

void Tree::deleteData(Tuple n){
	//check if root is a data node

	//initialize neighbors, anchors...
	RID RightAnchor = 0;
	RID LeftAnchor = 0;
	RID RightNeighbor = 0;
	RID LeftNeighbor = 0;
	int currentLevel = -1;
	RID current = root;
	int rightAnchorLevel = -1;
	int leftAnchorLevel = -1;
	deleteData(n, current, RightAnchor, LeftAnchor, RightNeighbor, LeftNeighbor, currentLevel, rightAnchorLevel, leftAnchorLevel);
}

void Tree::deleteData(Tuple n, RID current, RID RightAnchor, RID LeftAnchor, RID RightNeighbor, RID LeftNeighbor, int currentLevel, int rightAnchorLevel, int leftAnchorLevel)
{	
	RID NextLeftAnchor, NextRightAnchor;
	RID NextLeftNeighbor, NextRightNeighbor;
	int NextLeftAnchorLevel, NextRightAnchorLevel; 
	size_t pos;
	++currentLevel;	
	//check if we are at leaf node
	long int keyint=0; //???? what is this supposed to be
	if (current.getType(storage) == 'd')
	{
		current.deleteTuple(storage, n);
		if (currentLevel!=0)
			rebalance(currentLevel,current, LeftNeighbor, RightNeighbor, LeftAnchor, RightAnchor, leftAnchorLevel, rightAnchorLevel,keyint);
		return;
	}
	else{
		//figure out next node to visit
		RID Next = (current.traverse(storage, n, pos));
		if (pos == 0) //left most position
		{
			NextLeftAnchor = LeftAnchor;
			NextLeftAnchorLevel = leftAnchorLevel;
			NextRightAnchor = current;
			NextRightAnchorLevel = currentLevel;
			if (LeftNeighbor.id != 0){
				NextLeftNeighbor = LeftNeighbor.getPointer(storage, LeftNeighbor.getSize(storage)); // return right most in left neighbor
			}
			else
				NextLeftNeighbor = 0;
			NextRightNeighbor = (current).getPointer(storage, pos + 1);// return RID to node in pos+1 in array
		}
		else if (pos == current.getSize(storage))// right most position
		{
			NextLeftAnchor = current;
			NextLeftAnchorLevel = currentLevel;
			NextRightAnchor = RightAnchor;
			NextRightAnchorLevel = rightAnchorLevel;
			NextLeftNeighbor = current.getPointer(storage, pos - 1); //to do return pointer to pos-1 in current node
			if (RightNeighbor.id != 0){
				NextRightNeighbor = RightNeighbor.getPointer(storage, 0);  //to do return pointer to node in pos+1 in array
			}
			else
				NextRightNeighbor = 0;
		}
		else //middle node
		{
			NextLeftAnchor = current;
			NextLeftAnchorLevel = currentLevel;
			NextRightAnchor = current;
			NextRightAnchorLevel = currentLevel;
			NextLeftNeighbor = current.getPointer(storage, pos - 1); //to do return pointer to pos-1 in current node
			NextRightNeighbor = current.getPointer(storage, pos + 1);
		}
		deleteData(n, Next, NextRightAnchor, NextLeftAnchor, NextRightNeighbor, NextLeftNeighbor, currentLevel, NextRightAnchorLevel, NextLeftAnchorLevel);

	}
	int sizeNode = current.getSize(storage);
	if (sizeNode<I_MINKEY && currentLevel!=0) //underflow and not at root
		rebalance(currentLevel, current, LeftNeighbor, RightNeighbor, LeftAnchor, RightAnchor, leftAnchorLevel, rightAnchorLevel, keyint);
	else
	if (currentLevel == 0 && sizeNode == 0)
	{
		if (current.getType(storage) != 'd')
		{
			RID temp = current.getPointer(storage, 0);
			current = 0;
			root.updateFreeList(storage.is, 0);
			root = temp;
			RID::updateRoot(storage.is, root);
		}
	}
}

void Tree::rebalance(int currentLevel, RID current, RID leftN, RID rightN, RID leftA, RID rightA, int leftAlevel, int rightAlevel, long int key)
{
	//check if underflow
	if (current.getSize(storage) < D_MINKEY)
	{
		//rebalance
		//look at neighbors
		//add something to handle null neighbors
		int leftSize, rightSize;
		if (leftN.id == 0)
			leftSize = 0;
		else{
			leftSize = leftN.getSize(storage);
		}
		if (rightN.id == 0)
			rightSize = 0;
		else{
			rightSize = rightN.getSize(storage);
		}
		if (leftSize>D_MINKEY || rightSize > D_MINKEY)
			//shift
		{
			if (leftSize>D_MINKEY && rightSize > D_MINKEY)
			{//both are above minkey
				//determines which one has closest anchor
				if (leftAlevel > rightAlevel){
					//steal from right neighbor
					current.shiftLR(storage, leftN, leftA);
				}
				else{
					//steal from left neighbor
					current.shiftRL(storage, rightN, rightA);
				}
			}
			else if (leftSize > D_MINKEY){
				//steal from right neighbor
				current.shiftLR(storage, leftN, leftA);
			}
			else {
				//steal from left neighbor
				current.shiftRL(storage, rightN, rightA);
 			}
		}
		else
			//merge,both neighbors have minkey
		{
			if (rightN.id == 0){
				current.mergeLeft(storage, leftN, leftA);
				current = 0;
			}
			else if (leftN.id == 0){
				current.mergeRight(storage, rightN, rightA);
				rightN = 0;
			}
			else if (leftAlevel > rightAlevel){
				current.mergeLeft(storage, leftN, leftA);
			}
			else{
				current.mergeRight(storage, rightN, rightA);
				rightN = 0;
			}
		}
	}
}

void Tree::writeEmptyRoot(fstream& is){
	//create node struct
	DataNode* rootNode = new DataNode();
	rootNode->updateNode(is, 1);
}

void Tree::getTreeInfo(){
	storage.is.seekp(0, ios::beg);
	blockDef output;
	storage.is.read(reinterpret_cast<char*>(&output), sizeof(blockDef));
	root = output.rootRID;
	head = output.headRID;
}

