#pragma once
#include<string>
#include<vector>
#include<set>

class Relation
{
public:
	Relation();
	Relation(std::string);
	~Relation();
	Relation project(std::vector<std::string>);
private:
	//how to read the relation
	std::vector <std::string> schema;
	//data
	std::set<char*> data;
	
};

