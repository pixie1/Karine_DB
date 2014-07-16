#pragma once
#include<string>
#include<vector>
#include<queue>
#include "catalog.h"
#include "Tree.h"

using std::string;
using std::queue;
using std::vector;

class RA
{
	static vector<queue<char*>> q;
	static vector<catalog> cat;
	static void saveCatalog(size_t, catalog&);
	static void printTuple(char*, catalog&);
public:
	RA();
	~RA();
	static void project(vector<string>, size_t);
	void select(int);
	void join(int, int);
	void intersect(int, int);
	void unionTable(int, int);
	void equal(int);
	void less(int);
	void more(int);
	static void origin(string, catalog&, size_t); //bottom of tree string: name of the table to read from file
	static void display(size_t);
	

};

