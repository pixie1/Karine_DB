//Karine Josien
//CS 4100

#include <iostream>
#include <istream>
#include <sstream>
//#include <fstream>
#include "Tree.h"
#include <cstdlib>
#include <time.h>
#include <climits>
#include <cassert>
#include <string>
#include <ctime>
#include <vector>
#include <thread>
#include "DB.h"
int countBufferAccess;
int countDiskAccess;
int writeDisk; 
int writeBuffer;

using namespace std;

/*struct TableInfo{
	string name;
	string type;
	int pos;
};*/


void menu()
{
	std::cout << "0- Exit" << endl;
	std::cout << "1- GET / query" << endl;
	std::cout << "2- PUT" << endl;
	std::cout << "3- DELETE" << endl;
}

int main(int argc, char* argv[])
{

	//command line argument: name of the database file (file containing table and attribute catalog file names);

	//need name of system catalog
	string table = "Table.cat";
	string attr = "Attr.cat";

	//parse instruction
	//use recursion for parsing, should return a relation
	string instruction;

	//how to store the temporary relations? should I save on disk as a temporary b+tree or something else
	//if I save to disk, how do I read these, don't want to add them to catalog
	//need to keep the schema with temp relations to be able to read
	//remember no duplicates in RA
	//should I use a set somewhere to take care of duplicate, set of Nodes?g
	//any issues with using brackets for select instead of parens

	bool done = false;
	Tuple mytuple;
	string filename;
	int numBlocks;

	//check that a file name was passed
/*	if (argc != 2){
		std::cout << "Error: no file name";
		cin.get();
		exit(1);
	}
	else
		filename = argv[1];*/

	filename = "database.db";
	ifstream dbFileStream (filename);

	if (!dbFileStream){
		cout << "No database with that name";
		cin.get();
		exit(1);
	}

	DB database;
	try
	{
		DB database(dbFileStream);
	}
	catch (runtime_error &e){
		cout << e.what();
		cin.get();
		exit(1);
	}
	/*
	//check if file exist
	//string table = "Table.cat";
	//string attr = "Attr.cat";

	//check that filename exist in system catalog
	ifstream tableFile(table);
	string Tname;
	string Fname;
	getline(tableFile, Tname, ',');
	getline(tableFile, Fname);
	bool found = false;
	while (tableFile && !found){
	if (Fname == filename)
	found = true;
	else{
	getline(tableFile, Tname, ',');
	getline(tableFile, Fname);
	}
	}
	if (!found){
	std::cout << "File not in system catalog";
	cin.get();
	exit(1);
	}
	ifstream treeFile;
	treeFile.open(filename);
	bool opencheck;
	if (treeFile)
	opencheck = true;
	else
	opencheck = false;
	treeFile.close();
	//bool opencheck = false;
	if (!opencheck){//file does not exist create it
	std::cout << "New file. Enter the number of blocks to create:";
	cin >> numBlocks;
	fstream newfile(filename, ios::in | ios::out | ios::binary | ios::trunc);
	blockDef initBlock;
	initBlock.blocks = numBlocks;
	initBlock.rootRID = 1;
	initBlock.headRID = 1;
	initBlock.freeBlocks[0] = 1;
	for (int i = 1; i < FREELISTSIZE; i++)
	initBlock.freeBlocks[i] = 0;
	newfile.write(reinterpret_cast<char*>(&initBlock), sizeof(blockDef));
	//get tree class to write empty node for root
	Tree::writeEmptyRoot(newfile);
	newfile.close();
	}
	Tree myTree(filename);
	//read system catalog
	vector<TableInfo> systemInfo;
	ifstream attribute("Attr.cat");
	string _pos;
	string _name;
	string _type;
	string test;
	TableInfo temp;
	getline(attribute, test, ',');
	while (test != Tname && attribute){
	getline(attribute, test);
	getline(attribute, test, ',');
	}
	if (!attribute){
	std::cout << "Error, table not found in system catalog";
	cin.get();
	exit(1);
	}
	while (test == Tname && attribute){
	getline(attribute, _name, ',');
	getline(attribute, _type, ',');
	getline(attribute, _pos);
	temp.name = _name;
	temp.type = _type;
	temp.pos = stoi(_pos);
	systemInfo.push_back(temp);
	getline(attribute, test, ',');
	}
	//vector systemInfo should now contain all the attributes for the table we are looking at
	//maybe should sort it
	*/
	while (!done){

		menu();
		char choice;
		cin >> choice;
		while (choice != '1' && choice != '2' && choice != '3' && choice != '0'){
			std::cout << "invalid input\n Please enter 0, 1, 2, 3. ";
			cin >> choice;
		}
		if (choice == '1'){ //GET
			cin.ignore();
			//read system catalog
			cout << "Query:";
			string query;
			getline(cin, query);
			try{
				database.RAquery(query);
			}
			catch (runtime_error &e){
				cout << e.what();
				cout << endl;
			}
			/*
			long int value = 0;

			while (value != -99){
			std::cout << systemInfo[0].name << ","<<systemInfo[0].type<<","<<systemInfo[0].pos<<": ";
			string valueStr;

			getline(cin, valueStr);
			stringstream valueStream(valueStr);
			while (!(valueStream>>value)){
			std::cout << "\nError, need to enter an integer.\n";
			std::cout << systemInfo[0].name << "," << systemInfo[0].type << "," << systemInfo[0].pos << ": ";
			getline(cin, valueStr);
			valueStream.clear();
			valueStream.str(valueStr);
			}
			//cin.ignore();
			countBufferAccess = 0;
			countDiskAccess = 0;
			if (value != -99){

			try{
			mytuple= myTree.lookUp(value);
			}
			catch (runtime_error &e){
			std::cout<<e.what() << endl;
			break;
			}
			//print out the return
			if (mytuple.value[0] == '\0')
			std::cout << "key not found\n";
			else{
			std::cout << systemInfo[0].name << ": "<<mytuple.key<<", ";
			char* ptr = mytuple.value;
			for (int i = 1; i < systemInfo.size(); ++i){
			std::cout << " "<<systemInfo[i].name<<": ";

			if (systemInfo[i].type == "Int"){
			cout << *(reinterpret_cast<int*>(ptr));
			ptr = ptr + sizeof(int);
			}
			else{
			int j = 0;
			while (*ptr != '\0' && j<20){
			std::cout << *ptr;
			++ptr;
			j++;
			}
			while (j < 20){
			++ptr;
			++j;
			}
			}
			}
			cout << endl;
			}
			}
			}*/
		}
		/*
		else if (choice == '2')//PUT
		{
		cin.ignore();
		std::cout << "entries:" << endl;
		long int key = 0;
		while (key != -99){
		string valueStr;
		std::cout << systemInfo[0].name << "," << systemInfo[0].type << "," << systemInfo[0].pos << ": ";
		getline(cin, valueStr);
		stringstream valueStream(valueStr);
		while (!(valueStream >> key)){
		std::cout << "\nError, need to enter an integer.\n";
		std::cout << systemInfo[0].name << "," << systemInfo[0].type << "," << systemInfo[0].pos << ": ";
		getline(cin, valueStr);
		valueStream.clear();
		valueStream.str(valueStr);
		}
		//std::cout << endl;
		char data[96];
		char* dataPtr = data;
		string temp;
		if (key != -99){
		for (int i = 0; i < systemInfo.size(); ++i){
		std::cout << systemInfo[i].name << "," << systemInfo[i].type << "," << systemInfo[i].pos << ": ";
		if (systemInfo[i].type == "Int"){
		int value;
		getline(cin, temp);
		stringstream valueStream(temp);
		while (!(valueStream >> value)){
		std::cout << "\nError, need to enter an integer.\n";
		std::cout << systemInfo[i].name << "," << systemInfo[i].type << "," << systemInfo[i].pos << ": ";
		getline(cin, temp);
		valueStream.clear();
		valueStream.str(temp);
		}
		char* entry = reinterpret_cast<char*>(&value);
		for (int j = 0; j < sizeof(int); ++j){
		*dataPtr = *entry;
		++dataPtr;
		++entry;
		}
		}

		else{
		do{
		getline(cin, temp);
		if (temp.length()>20){
		std::cout << "\n Error maximum length is 20 characters. \n ";
		std::cout << systemInfo[i].name << "," << systemInfo[i].type << "," << systemInfo[i].pos << ": ";
		}
		} while (temp.length() > 20);
		for (int j = 0; j < 20; ++j){
		if (j < temp.length())
		*dataPtr = temp[j];
		if (j == temp.length())
		*dataPtr = '\0';
		++dataPtr;
		}
		}
		}
		mytuple.set(key, data);
		try{
		myTree.insert(mytuple);
		}
		catch (runtime_error &e){
		cout << e.what() << endl;
		break;
		}
		std::cout << "Tree dump:" << endl;;
		myTree.printTree();
		std::cout << "Leaves:" << endl;
		myTree.printList();
		//	cout << "buffer access:" << countBufferAccess << endl;
		//	cout << "disk access: " << countDiskAccess << endl;
		}
		}
		}

		else if (choice == '3') //delete
		{
		long int value = 0;
		cin.ignore();
		while (value != -99){
		string valueStr;
		std::cout << systemInfo[0].name << "," << systemInfo[0].type << "," << systemInfo[0].pos << ": ";
		getline(cin, valueStr);
		stringstream valueStream(valueStr);
		while (!(valueStream >> value)){
		std::cout << "\nError, need to enter an integer.\n";
		std::cout << systemInfo[0].name << "," << systemInfo[0].type << "," << systemInfo[0].pos << ": ";
		getline(cin, valueStr);
		valueStream.clear();
		valueStream.str(valueStr);
		}
		//cin.ignore();
		countBufferAccess = 0;
		countDiskAccess = 0;
		mytuple.set(value);
		myTree.deleteData(mytuple);
		cout << "Tree dump:" << endl;;
		myTree.printTree();
		cout << "Leaves:" << endl;
		myTree.printList();
		//	cout << "buffer access:" << countBufferAccess << endl;
		//	cout << "disk access: " << countDiskAccess << endl;
		}
		}
		else done = true;
		}

		/*else if (choice == '4')
		{
		long int size = 0;
		cout << "How many keys:";
		cin >> size;
		cout << endl;
		countBufferAccess = 0;
		countDiskAccess = 0;
		if (!myTree.empty())
		{
		cout << "Removing keys from the tree..." << endl;
		myTree.deleteAll();
		cout << "Tree dump:" << endl;;
		myTree.printTree();
		cout << "Leaves:" << endl;
		myTree.printList();
		}
		cout << "adding keys to the tree";

		//generate keys
		//srand(clock());
		srand(0);
		long int* data = new long int[size];
		for (long int i = 0; i < size; i++) {
		data[i] = double(rand())*double(rand()) / RAND_MAX/RAND_MAX*LONG_MAX; //number between 0 and max number represented by long int
		// Check and remove duplicate Keys
		for (long int j = 0; j < i; j++) {
		if (data[j] == data[i]) {
		// Generate a new random Key
		data[i] = double(rand())*double(rand()) / RAND_MAX/RAND_MAX*LONG_MAX;
		j = -1; // auto inc will set to 0
		}
		}
		mytuple.set(data[i]);
		try{
		myTree.insert(mytuple);
		}
		catch (runtime_error &e){
		cout << e.what() << endl;
		cout << i << " keys entered" << endl << endl;
		size = i;
		break;
		}
		}

		cout << "Tree dump:" << endl;
		myTree.printTree();
		cout << "Leaves:" << endl;
		myTree.printList();
		//cout << "hit a key to continue";
		//	system("PAUSE");

		//remove keys
		cout << "removing data...";
		for (long int i = 0; i < size; i++){
		mytuple.set(data[i]);
		myTree.deleteData(mytuple);
		data[i] = -99;
		for (long int j = i + 1; j < size; j++){
		assert(myTree.lookUp(data[j]));
		}
		cout << "Tree dump:" << endl;;
		myTree.printTree();
		cout << "Leaves:" << endl;
		myTree.printList();
		}
		delete[]data;
		cout << "buffer access:" << countBufferAccess << endl;
		cout << "disk access: " << countDiskAccess << endl;
		}
		else if (choice == '5'){
		cout << "entries:" << endl;
		long int value = 0;
		bool found;
		while (value != -99)
		{
		countBufferAccess = 0;
		countDiskAccess = 0;
		cin >> value;
		if (value != -99)
		{
		found= myTree.lookUp(value);
		if (found)
		cout << "Value was found" << endl;
		else
		cout << "Value was not found" << endl;
		cout << "buffer access:" << countBufferAccess << endl;
		cout << "disk access: " << countDiskAccess << endl;
		}
		}
		}
		else if (choice == '6')
		{
		vector<int> time;
		vector<int>access;
		vector<int>write;
		vector<int>bufferAccess;
		vector<int>bufferUpdate;
		for (int k = 0; k < 50; ++k)
		{
		cout << "insert 2000 in group of 100 values\n";
		int size = 100;
		srand(clock());
		clock_t start, end;
		Tuple toDelete[2000];
		//get random values
		for (int i = 0; i < 2000; ++i){
		int key = rand() % (2000 * 100);
		mytuple.set(key);
		toDelete[i] = mytuple;
		for (int j = 0; j < i; j++) {
		//make sure key not in data and not in tree
		if (toDelete[i] == toDelete[j]){ //|| myTree.lookUp(key)) {
		// Generate a new random Key
		key = rand() % (2000 * 100);
		mytuple.set(key);
		toDelete[i] = mytuple;
		j = -1; // auto inc will set to 0
		}
		}
		}
		//start clock
		countBufferAccess = 0;
		countDiskAccess = 0;
		writeBuffer = 0;
		writeDisk = 0;
		start = clock();
		for (int i = 0; i < 2000; ++i){
		try{
		myTree.insert(toDelete[i]);
		}
		catch (runtime_error &e){
		cout << e.what() << endl;
		cout << i << " keys entered" << endl << endl;
		size = i;
		break;
		}
		}
		end = clock();
		cout << "time: " << end - start << endl;
		cout << "buffer access:" << countBufferAccess << endl;
		cout << "disk access: " << countDiskAccess << endl;
		cout << "write to disk" << writeDisk << endl;
		cout << "Updates written to buffer" << writeBuffer << endl;


		time.push_back(end-start);
		access.push_back(countDiskAccess);
		write.push_back(writeDisk);
		bufferAccess.push_back(countBufferAccess);
		bufferUpdate.push_back(writeBuffer);
		countBufferAccess = 0;
		countDiskAccess = 0;
		writeDisk = 0;
		writeBuffer = 0;
		cout << "look up 1000 data\n";
		countBufferAccess = 0;
		countDiskAccess = 0;
		writeDisk = 0;
		writeBuffer = 0;
		start = clock();
		for (int i = 0; i < 1000; i++){
		myTree.lookUp(toDelete[rand() % 2000].key);
		}
		end = clock();
		cout << "time: " << end - start << endl;
		cout << "buffer access:" << countBufferAccess << endl;
		cout << "disk access: " << countDiskAccess << endl;
		cout << "write to disk" << writeDisk << endl;
		cout << "Updates written to buffer " << writeBuffer << endl;

		time.push_back(end-start);
		access.push_back(countDiskAccess);
		write.push_back(writeDisk);
		bufferAccess.push_back(countBufferAccess);
		bufferUpdate.push_back(writeBuffer);
		cout << "\n delete 2000 data in group of 100" << endl;

		countBufferAccess = 0;
		countDiskAccess = 0;
		writeDisk = 0;
		writeBuffer = 0;
		start = clock();
		for (int i = 0; i < 2000; ++i){
		myTree.deleteData(toDelete[i]);
		}
		end = clock();

		cout << "time: " << end - start << endl;
		cout << "buffer access:" << countBufferAccess << endl;
		cout << "disk access: " << countDiskAccess << endl;
		cout << "write to disk" << writeDisk << endl;
		cout << "Updates written to buffer " << writeBuffer << endl;

		time.push_back(end - start);
		access.push_back(countDiskAccess);
		write.push_back(writeDisk);
		bufferAccess.push_back(countBufferAccess);
		bufferUpdate.push_back(writeBuffer);

		}
		for (int i = 0; i < 150; ++i){
		if (i % 3 == 0)
		cout << "insert: ";
		if (i % 3 == 1)
		cout << "look up: ";
		if (i % 3 == 2)
		cout << "delete: ";
		cout << "time: " << time[i] << ":buffer access: "<<bufferAccess[i]<<": buffer update: "<<bufferUpdate[i]<<": disk access: " << access[i] << " :disk write: " << write[i] << endl;
		}
		}*/


	}

	/*
	Relation parse(stringstream& value){

	vector<thread> operations;
	string name1;
	char c;
	c = value.get();
	//read stringstream

	//check if first char is a (
	//recursive call
	//parse(value); //make sure character read have been consumed
	//else
	//get name of first table
	while (isalnum(c)){
	name1 += c;
	c = value.get();
	}
	value.unget();
	//somehow get relation corresponding to that name
	//Relation relation1(name1);
	//read operator
	//remove whitespace
	c = value.get();
	if (c == '('){
	//select
	}
	else if (c == '['){


	//project
	vector<string> projection;
	//remove whitespace
	//read the attributes that need to be projected, stored in vector of strings
	c = value.get();
	while (c != ']'){
	if (c == '\''){
	string temp;
	getline(value, temp, '\'');
	projection.push_back(temp);
	}
	else{
	//add throwing error???
	cout << "ERROR";
	}
	}
	//start first thread to read table
	operations.push_back(thread(&RA::origin,name1, 0));
	operations.push_back(thread(&RA::project, projection, 1));
	operations.push_back(thread(&RA::display, 2));


	}
	else if (c == '*'){
	//join
	}
	else if (c == '|'){
	//union
	}
	else if (c == '&'){
	//intersection
	}
	else if (c == '-'){
	//difference
	}



	}

	bool catalog::catalogCheck(string tableName){

	string table = "Table.cat";
	string attr = "Attr.cat";

	//check that filename exist in system catalog
	ifstream tableFile(table);
	string Tname;
	string Fname;
	getline(tableFile, Tname, ',');
	getline(tableFile, Fname);
	bool found = false;
	while (tableFile && !found){
	if (Fname == tableName)
	found = true;
	else{
	getline(tableFile, Tname, ',');
	getline(tableFile, Fname);
	}
	}
	if (!found){
	std::cout << "File not in system catalog";
	cin.get();
	exit(1);
	}
	return true;
	}*/
}