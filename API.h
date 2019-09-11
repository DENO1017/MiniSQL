#ifndef __API_H__
#define __API_H__
#include"CatalogManager.h"
#include"minisql.h"
#include"BufferManager.h"
#include"RecordManager.h"
#include"IndexManager.h"
#include<algorithm>
#include<iterator>



extern BufferManager bm;
extern CatalogManager cm;
extern RecordManager rm;
extern IndexManager im;

class API
{
private:

	//BufferManager bm;
public:
	CatalogManager *cm;
	RecordManager *rm;

	API();
	void Init();                      // 初始化函数，在构造函数中调用，将所有表格、索引信息读入CatalogManager cm中
	void CreateTable(Table &tbl);     // 建表函数，参数：Table
	void InsertRecord(string tableName, vector<string> recordContent);	//插入函数，参数：表名，保存了record信息的string vector
	void getContent(char* charContent, vector<string> recordContent, vector <Attribute> attri);
	int checkRecord(Table* tmpTable, vector<string>recordContent);
	void RecordAllShow(string tableName, vector<Condition> conditionVec);
	//void RecordConditionShow(string tableName, vector<Condition> conditionVec);
	void PrintAttribute(Table* tbl);
	int CheckCondition(Table* tmpTable, vector<Condition>conditionVec);
	void tableDrop(string tableName);
	void deleteAllRecord(string tableName);
	void DeleteConditionRecord(string tableName, Condition conditionValue);
	void CreateIndex(char*  indexName, char*  tableName, char*  attriName);
	int InsertAllKeys(Table* tmpTable, string attriName, string indexName);
	void IndexDrop(string indexName);
	void prepareToQuit();
	void InitProgram();
	void print();
};

#endif 
