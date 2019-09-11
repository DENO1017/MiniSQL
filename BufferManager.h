#ifndef __BUFFERMANAGER_H__
#define __BUFFERMANAGER_H__
#include<iostream>
#include<vector>
#include<string>
#include"minisql.h"
#include <fstream>

#define MaxBlock 300/////////////////////////////////////////////////////////////////////////////////////////
#define BlockType int
#define DataBlock 0
#define IndexBlock 1
#define DefaultBlock -1
#define BlockSize 4096
//extern CatalogManager cm;
//extern RecordManager rm;

using namespace std;

typedef struct Block* PtrToblock;
struct Block
{
	BlockType blockType;
	char *content;
	int id;
	string fileName;
	bool isDirty;
	int using_Size;
	int head_Size;
	int RLUcount;
};

class BufferManager
{
private:
	Block blocks[MaxBlock];
public:
	BufferManager();
	int total_Block;
	void initBlock(Block &block);
	Block* getBlock(string _fileName);
	Block* findBlock(Table* tmpTable, int _blockType, string _fileName, int _blockID);
	void blockSetValue(Block* btmp, int _id, int _BlockType, string _fileName);
	//void print();
	Block* returnBlock(int posi);
	void SetDirty(Block* btmp);
	void CleanDirty(Block* btmp);
	int WriteBackToDisk(string fileName, int writeBackBlock, int blockType);
	Block* WriteBlockToBuffer(string fileName, int blockID, int blockType);
	void DropTable(string tableName);
	void DeleteAllRecord(string tableName);
	char* get_content(Block* btmp);
	Block* findBlockById(string tableName, int id);
	void DropIndex(string tableName);
	void WriteBackAll();
	void DeclineTotalBlock();
};




#endif // !__BUFFERMANAGER_H__

