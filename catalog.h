#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <iostream>

using std::string;
struct TableInfo{
	string name;
	string type;
	int pos;
};

//class catalog
//contains catalog info for the tuple that is passed to the next operation
class catalog
{
private:
	
	bool catalogCheck(string); //check that table is in system catalog
public:
	std::vector<TableInfo> TableCatalog;
	catalog(){};
	catalog(std::string);
	catalog(std::vector<string>, catalog);
	catalog(catalog, catalog);
	~catalog();
	bool unionCompatible(catalog);

};

