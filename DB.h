#pragma once
#include <string>
#include <fstream>
#include"RA.h"
#include<stdexcept>

class DB
{
private:
	std::ifstream attrList;
	std::ifstream tablesList;
	std::string checkCatalog(std::string);
public:
	DB(){
		attrList.open("Attr.cat"); 
		tablesList.open("Table.cat");
	}
	DB(std::string attr, std::string table){
		attrList.open(attr);
		tablesList.open(table);
	}

	DB(std::ifstream& DBfile){

		string table, attr;
		getline(DBfile, table);
		getline(DBfile, attr);
		if (DBfile.fail())
			throw std::runtime_error("Error reading database file");
		attrList.open(attr);
		if (!attrList)
			throw std::runtime_error("Error opening system catalog: attribute files");
		tablesList.open(table);
		if (!tablesList)
			throw std::runtime_error("Error opening system catalog: table files");
	}

	void RAquery(std::string);
};

