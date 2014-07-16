#include "catalog.h"
using namespace std;

catalog::catalog(string tablename)
{
	//create catalog object for tablename from system catalog
		//read info from attribute table
		ifstream attribute("Attr.cat");
		string _pos;
		string _name;
		string _type;
		string test;
		TableInfo temp;
		getline(attribute, test, ',');
		while (test != tablename && attribute){
			getline(attribute, test);
			getline(attribute, test, ',');
		}
		if (!attribute){
			std::cout << "Error, table not found in system catalog";
			cin.get();
			exit(1);
		}
		while (test == tablename && attribute){
			getline(attribute, _name, ',');
			getline(attribute, _type, ',');
			getline(attribute, _pos);
			temp.name = _name;
			temp.type = _type;
			temp.pos = stoi(_pos);
			TableCatalog.push_back(temp);
			getline(attribute, test, ',');
		}
}


catalog::catalog(vector<string> attributes, catalog oldCatalog){
	//create new catalog from list of attributes and catalog information (for projection)
	for (int i = 0; i < attributes.size(); i++){
		for (int j = 0; j < oldCatalog.TableCatalog.size(); j++){
			if (oldCatalog.TableCatalog[j].name == attributes[i]){
				TableCatalog.push_back(oldCatalog.TableCatalog[j]);
				break; 
			}
		}
	}
}

catalog::catalog(catalog catalog1, catalog catalog2){ //join

}

catalog::~catalog()
{
}

