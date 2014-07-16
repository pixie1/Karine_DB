#include "DB.h"
#include <thread>
#include <vector>
#include <sstream>
#include <iostream>
using namespace std;

void DB::RAquery(string query){

	//parse query
	vector<thread> operations;
	stringstream value(query);
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

	//check that is belong in catalog
	string fileName1 = checkCatalog(name1);

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
				c = value.get();
			}
			else{
				//add throwing error???
				cout << "ERROR";
			}
		}
		//get catalog for table
		catalog catalogInfo(name1);
		//start first thread to read table
		operations.push_back(thread(&RA::origin, fileName1, catalogInfo, 0u));
		operations.push_back(thread(&RA::project, projection, 1u));
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

	for (size_t i = 0; i < operations.size(); ++i){
		operations[i].join();
	}
	//operations[0].join();
}

string DB::checkCatalog(string tableName){


	//check that filename exist in system catalog
	string Tname;
	string Fname;
	getline(tablesList, Tname, ',');
	getline(tablesList, Fname);
	bool found = false;
	while (tablesList && !found){
		if (Tname == tableName)
			found = true;
		else{
			getline(tablesList, Tname, ',');
			getline(tablesList, Fname);
		}
	}
	if (!found){
		tablesList.clear();
		throw runtime_error("Table does not exist in system catalog");
	}
	return Fname;
}