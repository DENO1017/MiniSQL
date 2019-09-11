#ifndef __RECORDMANAGER_H__
#define __RECORDMANAGER_H__
#include"BufferManager.h"
#include"CatalogManager.h"
#include"Condition.h"
#include"minisql.h"
#include"IndexManager.h"
#include<cstdio>
#pragma warning(disable:4996)

#define MAXNAMESIZE 100


extern BufferManager bm;
extern IndexManager im;
//extern CatalogManager cm;
//extern RecordManager rm;

class RecordManager
{
public:
	//RecordManager();
	//~RecordManager();

	int Insert(Block* btmp, char* address, char* content, int recordSize);
	int recordAllFind(Table* tmpTable, vector<Condition>conditionVector);
	int recordBlockFind(Table* tmpTable, vector<Condition>conditionVector, Block* btmp);
	int recordConditionFit(char* addressBegin, Table* tmptable, vector<Condition> conditionVector);
	int recordBlockShow(Block* btmp, vector<Condition>conditionVec, Table* tmpTable);
	void DropTable(string tableName);
	void DeleteAllTableRecord(Table* tmpTable);
	int DeleteConditionRecord(Table* tmpTable, Condition conditionValue);
	int CreateIndex(char* indexName, char* tableName, char* attriName, int type, int degree);
	int DeleteConditionRecordByBlock(Table* tmpTable, int blockId, Condition conditionValue);
	void DropIndex(string indexName);
};


#endif // !__RECORDMANAGER_H__

