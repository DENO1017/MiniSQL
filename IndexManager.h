#ifndef __INDEXMANAGER_H__
#define __INDEXMANAGER_H__

#include<iostream>
#include<string>
#include<map>
#include<set>
#include"BufferManager.h"
#include"Condition.h"
#include"minisql.h"
#include"BPlusTree.h"
#include"Condition.h"
#include<fstream>

using namespace std;
typedef int OffSetNumber;

class IndexManager
{
private:
	typedef map<string, BPlusTree<int> *> intIndexMap;				//用于保存各个类型的b+树
	typedef map<string, BPlusTree<string> *> stringIndexMap;
	typedef map<string, BPlusTree<float> *> floatIndexMap;

	intIndexMap intIdxTrees;
	stringIndexMap stringIdxTrees;
	floatIndexMap floatIdxTrees;

public:
	void CreateIndex(string indexName, int type, int degree = 5);
	void InsertKey(string indexName, string attriContent, int blockId, int type);
	set<OffSetNumber> SearchByIndex(string recordContent, string indexName, int type, int operation);
	void DeleteIndexByKey(string indexName, char* attriContent, int type);
	void DeleteAllNode(string indexName, int type);
	void DropIndex(string indexName, int type);
	void WriteBackAll(string indexName, int type);
	void InitTree(string indexName, int blockNum, int type, int degree);
};







#endif // !__INDEXMANAGER_H__

