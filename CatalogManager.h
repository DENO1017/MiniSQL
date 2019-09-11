#ifndef __CatalogManager_H__
#define __CatalogManager_H__
#include<iostream>
#include<vector>
#include"minisql.h"
#include"BufferManager.h"
#include"IndexManager.h"
using namespace std;

#define MAX_TABLENAME 50
#define MAX_ATTRINAME 20
#define NAMELENGTH 100
extern IndexManager im;


struct Index
{
	string tableName;
	string attriName;
	string indexName;
	int type;
};



class CatalogManager
{
public:
	vector<Table> tableList;
	vector<Index> indexList;
	bool CreateTable(Table &tbl);
	Table* findTable(string tableName);
	void DropTable(string tableName);
	void DeleteAllTableRecord(string tableName);
	void DeleteConditionRecord(string tableName, int recordDeleted);
	void InsertRecord(string tableName, int _recordNum, int _blockNum);
	void CreateIndex(string _indexName, string _tableName, string _attriName);
	int GetIndexType(string _tableName, string _attriName);
	Index* FindIndex(string indexName);
	void DropIndex(string indexName);
	void prepareToQuit();
	void SaveAllFiles();
	void InitCatalog();
	int CheckAttri(string tableNae, string indexName, string attriName);
	BufferManager bm;
};


#endif 

