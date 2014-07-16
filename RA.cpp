#include "RA.h"
#include <thread>
#include<mutex>
#include <atomic>
#include <condition_variable>
#include <memory>
#include<deque>

using namespace std;

vector<queue<char*>> RA::q;
vector<catalog> RA::cat;

mutex catalog_sync;
mutex prod_sync;
deque<condition_variable> q_cond;
//deque<unique_ptr<condition_variable>> q_cond;
//vector<unique_ptr<mutex>> q_sync;
deque<mutex> q_sync;


vector<size_t> producing;

RA::RA()
{
}


RA::~RA()
{
}

void RA::project(vector<string> attributes, size_t threadNum){
	
	queue<char*> tempQueue;
	q.push_back(tempQueue);
	//unique_ptr<condition_variable> temp(new condition_variable);
	//q_cond.push_back(temp);
	//unique_ptr<mutex> tempMutex(new mutex);
	q_sync.emplace_back();
	q_cond.emplace_back();
	//retrieve catalog object from vector (thread# -1) tupleInfo
	catalog tupleInfo = cat[threadNum - 1];
	//create new catalog object
	catalog newCatalog (attributes, tupleInfo);
	//save in vector
	saveCatalog(threadNum, newCatalog);
	bool done = false;
	while (!done){
		unique_lock<mutex> pLock(prod_sync);
		if (producing.size() == threadNum){ //ensure catalog inserted in correct position
			producing.push_back(1);
			done = true;
		}
		else if (producing.size() > threadNum){ throw runtime_error("runtime error, producer tracking error"); }
		pLock.unlock();
	}

	for (;;){
		unique_lock<mutex> qlck(q_sync[threadNum - 1]);
		q_cond[threadNum - 1].wait(qlck, [threadNum](){return !q[threadNum - 1].empty() || !producing[threadNum - 1]; });
		//q_cond[threadNum - 1].wait(qlck, [threadNum](){return !q[threadNum - 1].empty(); });
		//check if queue is empty and child thread still producing
		if (q[threadNum - 1].empty() && !producing[threadNum - 1]){
			break;
		}
		auto data = q[threadNum - 1].front();
		q[threadNum - 1].pop();
		qlck.unlock();
		//process item
		char* projection = new char[MAXTUPLE];
		char* ptr = projection;
		char* oldPtr = data;
		for (size_t i = 0; i < tupleInfo.TableCatalog.size(); ++i){
			for (size_t j = 0; j < newCatalog.TableCatalog.size()&& j<i+1; ++j){
				if (tupleInfo.TableCatalog[i].name == newCatalog.TableCatalog[j].name){
					if (tupleInfo.TableCatalog[i].type == "Int"){
						memcpy(ptr, oldPtr, sizeof(int));
						ptr = ptr + sizeof(int);
						oldPtr = oldPtr + sizeof(int);
					}
					else{
						memcpy(ptr, oldPtr, 20);
						ptr = ptr + 20;
						oldPtr = oldPtr + 20;
					}
					break;
				}
				else{
					if (tupleInfo.TableCatalog[i].type == "Int"){
						oldPtr = oldPtr + sizeof(int);
					}
					else
						oldPtr = oldPtr + 20; //other type is char 20
				}
			}
		}
		unique_lock<mutex> qlock2 (q_sync[threadNum]);
		q[threadNum].push(projection);
		qlock2.unlock();
		q_cond[threadNum].notify_one();
		delete data;
	}
	//figure out how to access the right buffer/queue
	//get char array from queue
	//read char[]
	//save new char[]
	//put in output queue
	producing[threadNum]--;
}

void RA::origin(string tableName, catalog& catalogInfo, size_t threadNum){

	queue<char*> tempQueue;
	q.push_back(tempQueue);
	//unique_ptr<condition_variable> temp(new condition_variable);
	//q_cond.push_back(temp);
	q_sync.emplace_back();
	q_cond.emplace_back();
	bool done = false;
	while (!done){
		unique_lock<mutex> pLock(prod_sync);
		if (producing.size() == threadNum){ //ensure value inserted in correct position
			producing.push_back(1);
			done = true;
		}
		else if (producing.size() > threadNum){ throw runtime_error("runtime error, producer tracking error"); }
		pLock.unlock();
	}
	//create catalog object
	//catalog catalogInfo(tableName);
	//save in vector position corresponds to thread number
	saveCatalog(threadNum, catalogInfo);
	Tree myTree(tableName);
	myTree.lookUpList(q[threadNum], threadNum);
	producing[threadNum]--;
	//q_cond.notify_all();
	q_cond[threadNum].notify_all();
}

void RA::join(int pos1, int pos2){
	//create catalog object

}

void RA::display(size_t threadNum){
	//retrieve catalog info
	catalog tupleInfo;
	for (;;){
		if (cat.size() > threadNum - 1){
			tupleInfo = cat[threadNum - 1];
			break;
		}
	}
	//retrieve item from queue
	for (;;){
		unique_lock<mutex> qlck(q_sync[threadNum - 1]);
		q_cond[threadNum - 1].wait(qlck, [threadNum](){return !q[threadNum - 1].empty()||!producing[threadNum-1]; });
		//check if queue is empty and child thread still producing
		if (q[threadNum - 1].empty() && !producing[threadNum - 1]){
			break;
		}
		auto myTuple = q[threadNum-1].front();
		q[threadNum - 1].pop();
		qlck.unlock();
		//print item
		printTuple(myTuple, tupleInfo);
	}
}

void   RA::saveCatalog(size_t threadNum, catalog& catalogInfo){
	bool done = false;
	while (!done){
		unique_lock<mutex> catLock(catalog_sync);
		if (cat.size() == threadNum){ //ensure catalog inserted in correct position
			cat.push_back(catalogInfo);
			done = true;
		}
		else if (cat.size() > threadNum){ throw runtime_error("runtime error, can't save catalog info correctly"); }
		catLock.unlock();
	}
}

void RA::printTuple(char* ptrData , catalog& tupleInfo){
	//std::cout << tupleInfo.TableCatalog[0].name << ": " << myTuple.key << ", ";
	//char* ptr = &data;
	char* ptr = ptrData;
	for (size_t i = 0; i < tupleInfo.TableCatalog.size(); ++i){
		std::cout << " " << tupleInfo.TableCatalog[i].name << ": ";

		if (tupleInfo.TableCatalog[i].type == "Int"){
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
	delete ptrData;
}
