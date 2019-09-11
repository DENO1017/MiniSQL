#include"CatalogManager.h"
#include <fstream>
#include"minisql.h"
#include<stdlib.h>
#include<math.h>
using namespace std;
#pragma warning(disable:4996)

bool CatalogManager::CreateTable(Table &tbl)
{
	for (int i = 0; i < tableList.size(); i++)
	{
		if (tableList[i].name == tbl.name)
		{
			cout << "table " << tbl.name << " already exist" << endl;
			return false;
		}
	}
	string tblName = "TABLE_" +  tbl.name + ".dat";
	ofstream fout(tblName, ios::out | ios::binary);
	/*---- 把table基础信息写入文件头 ---*/
	int attriNum = tbl.attri.size();
	int headSize = attriNum * 28 + 24;
	// 一个int 4字节，文件中占8位16进制数
	fout.write((char*)(&headSize), sizeof(int));       // 文件头大小
	fout.write((char*)(&attriNum), sizeof(int));       // attribute数量
	fout.write((char*)(&tbl.recordNum), sizeof(int));  // record数量
	fout.write((char*)(&tbl.blockNum), sizeof(int));   // block数量
	fout.write((char*)(&tbl.primaryKey), sizeof(int)); // primaryKey的位置
	int size = 0;
	for (int i = 0; i < attriNum; i++)
		size += getTypeSize(tbl.attri[i].type);
	fout.write((char*)(&size), sizeof(int));           // 每条record的字节数
	for (int i = 0; i < attriNum; i++)                 // 每个attribute的信息
	{
		Attribute a = tbl.attri[i];
		char attriName[MAX_ATTRINAME] = { 0 };
		strcpy(attriName, a.name.c_str());
		fout.write(attriName, MAX_ATTRINAME);
		fout.write((char*)(&a.type), sizeof(int));     // 类型
		if (a.isUnique)
		{
			int tmp = 1;
			fout.write((char*)(&tmp), sizeof(int));// 是否unique
		}
		else
		{
			int tmp = 0;
			fout.write((char*)(&tmp), sizeof(int));// 是否unique
		}
	}
	cout << "Success Message:: Create table " << tbl.name << " successful!" << endl;
	cout << endl;
	return true;
}


Table* CatalogManager::findTable(string tableName)
{
	Table* tmpTbl;
	int i = 0;
	for (i = 0; i < (int)tableList.size(); i++)
	{
		if (tableList[i].name != tableName)
		{
			continue;
		}
		else
		{
			return &tableList[i];
		}
	}
	if (i == (int)tableList.size())
	{
		cout << "Syntax Error: table doesn't exist!" << endl;
		return NULL;
	}
}


void CatalogManager::DropTable(string tableName)
{
	vector<Table>::iterator it;
	for (it=tableList.begin(); it!=tableList.end(); it++)
	{
		if (it->name == tableName)
		{
			tableList.erase(it);
			break;
		}
	}
}


void CatalogManager::DeleteAllTableRecord(string tableName)
{
	for (int i = 0; i < tableList.size(); i++)
	{
		if (tableList[i].name == tableName)
		{
			tableList[i].blockNum = 0;
			tableList[i].recordNum = 0;
			break;
		}
	}
}


void CatalogManager::DeleteConditionRecord(string tableName, int recordDeleted)
{
	for (int i = 0; i < tableList.size(); i++)
	{
		if (tableList[i].name == tableName)
		{
			tableList[i].recordNum -= recordDeleted;
			break;
		}
	}
}


void CatalogManager::InsertRecord(string tableName, int _recordNum, int _blockNum)
{
	for (int i = 0; i < tableList.size(); i++)
	{
		if (tableList[i].name == tableName)
		{
			tableList[i].recordNum = _recordNum;
			tableList[i].blockNum = _blockNum;
			break;
		}
	}
}



void CatalogManager::CreateIndex(string _indexName, string _tableName, string _attriName)
{
	for (int i = 0; i < indexList.size(); i++)
	{
		if (indexList[i].indexName == _indexName)
		{
			cout << "index " << _indexName << " already exist" << endl;
			return;
		}
	}
	for (int i = 0; i < tableList.size(); i++)
	{
		if (tableList[i].name == _tableName)
		{
			for (int j = 0; j < tableList[i].attri.size(); j++)
			{
				if (tableList[i].attri[j].name == _attriName)
				{
					
					tableList[i].attri[j].index = _indexName;
				}
			}
		}
	}
	Index tmpIdx;
	tmpIdx.attriName = _attriName;
	tmpIdx.indexName = _indexName;
	tmpIdx.tableName = _tableName;
	Table* tmpTable = findTable(_tableName);
	int type = GetIndexType(_tableName,  _attriName);
	tmpIdx.type = type;
	indexList.push_back(tmpIdx);
}



int CatalogManager::GetIndexType(string _tableName, string _attriName)
{
	int type = -2;
	Table* tmpTable;
	tmpTable = findTable(_tableName);
	for (int i = 0; i < tmpTable->attri.size(); i++)
	{
		if (tmpTable->attri[i].name == _attriName)
		{
			type = tmpTable->attri[i].type;
			break;
		}
	}
	return type;
}



Index* CatalogManager::FindIndex(string indexName)
{
	Index* tmpIdx;
	for (int i = 0; i < indexList.size(); i++)
	{
		if (indexList[i].indexName == indexName)
		{
			tmpIdx = &indexList[i];
			return tmpIdx;
		}
	}
	return NULL;
}



void CatalogManager::DropIndex(string indexName)
{
	vector<Index>::iterator it;
	for (it = indexList.begin(); it != indexList.end(); it++)
	{
		if (it->indexName == indexName)
		{
			indexList.erase(it);
			break;
		}
	}
}



void CatalogManager::prepareToQuit()
{
	tableList.clear();
	indexList.clear();
}



void CatalogManager::SaveAllFiles()
{
	string File = "Files_All.dat";
	fstream fout(File, ios::binary | ios::out);
	int tableNum = tableList.size();
	int indexNum = indexList.size();
	myint tmpint;
	tmpint.num = tableNum;
	fout.write(tmpint.byte, sizeof(int));
	tmpint.num = indexNum;
	fout.seekp(4, ios::beg);
	fout.write(tmpint.byte, sizeof(int));
	char* tmpName = new char[NAMELENGTH];
	char* tmpIndex = new char[MAX_ATTRINAME];
	memset(tmpName, '\0', NAMELENGTH);
	memset(tmpIndex, '\0', MAX_ATTRINAME);
	for (int i = 0; i < tableList.size(); i++)
	{
		memcpy(tmpName, tableList[i].name.c_str(), NAMELENGTH);
		fout.write(tmpName, NAMELENGTH);
		int recordNum = tableList[i].recordNum;
		int blockNum = tableList[i].blockNum;
		fstream f1;
		string file = getFileName(tableList[i].name);
		f1.open(file, ios::binary | ios::in | ios::out);
		f1.seekp(8, ios::beg);
		f1.write((char*)(&recordNum), sizeof(int));
		f1.seekp(12, ios::beg);
		f1.write((char*)(&blockNum), sizeof(int));
	}
	for (int i = 0; i < indexList.size(); i++)
	{
		memcpy(tmpIndex, indexList[i].indexName.c_str(), MAX_ATTRINAME);
		fout.seekp(8 + NAMELENGTH * tableList.size() + i * MAX_ATTRINAME, ios::beg);
		fout.write(tmpIndex, MAX_ATTRINAME);
	}
}



void CatalogManager::InitCatalog()
{
	fstream ifs;
	ifs.open("Files_All.dat", ios::in | ios::binary);
	if (!ifs)
	{
		cout << "There is no table and index in the program" << endl;
		cout << endl;
		return;
	}
	myint tmpint;
	char* tmpc = new char[NAMELENGTH];
	int indexNum;
	int tableNum;
	ifs.read(tmpint.byte, 4);
	tableNum = tmpint.num;
	ifs.seekg(4);
	ifs.read(tmpint.byte, 4);
	indexNum = tmpint.num;
	cout << "there are " << tableNum << " tables and " << indexNum << " indexs in the program" << endl;
	cout << endl;
	for (int i = 0; i < tableNum; i++)
	{
		ifs.seekg(8 + i * NAMELENGTH);
		ifs.read(tmpc, NAMELENGTH);			//读取表名
		string tmpTableName = tmpc;
		string file = getFileName(tmpTableName);
		fstream pTblFile;
		pTblFile.open(file, ios::in | ios::binary);
		pTblFile.read(tmpint.byte, sizeof(int));
		int fileHeadSize = tmpint.num;
		pTblFile.seekg(4);
		pTblFile.read(tmpint.byte, sizeof(int));
		int attriNum = tmpint.num;
		pTblFile.seekg(8);
		pTblFile.read(tmpint.byte, sizeof(int));
		int recordNum = tmpint.num;
		pTblFile.seekg(12);
		pTblFile.read(tmpint.byte, sizeof(int));
		int blockNum = tmpint.num;
		pTblFile.seekg(16);
		pTblFile.read(tmpint.byte, sizeof(int));
		int primaryPosi = tmpint.num;
		pTblFile.seekg(20);
		pTblFile.read(tmpint.byte, sizeof(int));
		int recordSize = tmpint.num;
		vector<Attribute> tmpAttriVec;
		for (int j = 0; j < attriNum; j++)
		{
			pTblFile.seekg(24 + j * (MAX_ATTRINAME + sizeof(int) + sizeof(int)));
			char* tmpAttriName = new char[MAX_ATTRINAME];
			pTblFile.read(tmpAttriName, MAX_ATTRINAME);
			pTblFile.seekg(24 + j * (MAX_ATTRINAME + sizeof(int) + sizeof(int)) + MAX_ATTRINAME);
			pTblFile.read(tmpint.byte, sizeof(int));
			int type = tmpint.num;
			pTblFile.seekg(24 + j * (MAX_ATTRINAME + sizeof(int) + sizeof(int)) + MAX_ATTRINAME + sizeof(int));
			bool tmpUnique;
			pTblFile.read(tmpint.byte, sizeof(int));
			if (tmpint.num == 1)
			{
				tmpUnique = true;
			}
			else
			{
				tmpUnique = false;
			}
			Attribute tmpAttri(tmpAttriName, type, tmpUnique);
			tmpAttriVec.push_back(tmpAttri);
		}
		Table tmpTable(tmpc, tmpAttriVec, primaryPosi, recordNum, blockNum, recordSize);
		tableList.push_back(tmpTable);
	}
	for (int i = 0; i < indexNum; i++)
	{
		ifs.seekg(8 + tableNum * NAMELENGTH + i * MAX_ATTRINAME);
		ifs.read(tmpc, MAX_ATTRINAME);			//读取index名
		string file = GetIndexFileName(tmpc);
		fstream pIdxlFile;
		pIdxlFile.open(file, ios::in | ios::binary);
		pIdxlFile.read(tmpint.byte, sizeof(int));
		int fileHeadSIze = tmpint.num;
		pIdxlFile.seekg(4);
		pIdxlFile.read(tmpint.byte, sizeof(int));
		int blockNum = tmpint.num;
		pIdxlFile.seekg(8);
		char* tmpIndexName = new char[MAX_ATTRINAME];
		pIdxlFile.read(tmpIndexName, MAX_ATTRINAME);
		char* tmpTableName = new char[NAMELENGTH];
		pIdxlFile.seekg(8 + MAX_ATTRINAME);
		pIdxlFile.read(tmpTableName, NAMELENGTH);
		pIdxlFile.seekg(8 + MAX_ATTRINAME + NAMELENGTH);
		char* tmpAttriName = new char[MAX_ATTRINAME];
		pIdxlFile.read(tmpAttriName, MAX_ATTRINAME);
		pIdxlFile.seekg(8 + 2 * MAX_ATTRINAME + NAMELENGTH);
		pIdxlFile.read(tmpint.byte, sizeof(int));
		int type = tmpint.num;
		pIdxlFile.seekg(8 + 2 * MAX_ATTRINAME + NAMELENGTH + sizeof(int));
		pIdxlFile.read(tmpint.byte, sizeof(int));
		int degree = tmpint.num;
		Index tmpIndex;
		tmpIndex.attriName = tmpAttriName;
		tmpIndex.indexName = tmpIndexName;
		tmpIndex.tableName = tmpTableName;
		tmpIndex.type = type;
		indexList.push_back(tmpIndex);
		im.InitTree(tmpc, blockNum, type, degree);

	}
	for (int i = 0; i < tableList.size(); i++)
	{
		for (int j = 0; j < tableList[i].attri.size(); j++)
		{
			for (int k = 0; k < indexList.size(); k++)
			{
				if (indexList[k].attriName == tableList[i].attri[j].name)
				{
					tableList[i].attri[j].index = indexList[k].indexName;
				}
			}
		}
	}
}


/**
*	检查attribute是否存在或存在索引，建立索引时调用
*	@param1: 表名
*	@param2: 索引名
*	@param3: 列名
*	return 0: 所在列不是unique
*	return -1: 所选表不存在
*	return -2: 索引已经存在
*	return -3: 所选列不存在
*	return 1: 可以建立索引
*/
int CatalogManager::CheckAttri(string tableName, string indexName, string attriName)
{
	int flag = 0;
	Table* tmpTable = NULL;
	int i = 0;
	for (i = 0; i < tableList.size(); i++)
	{
		if (tableList[i].name == tableName)
		{
			tmpTable = &tableList[i];
			break;
		}
	}
	if (i == tableList.size())
	{
		return -1;				//所选表不存在
	}
	for (i = 0; i < tmpTable->attri.size(); i++)
	{
		if (tmpTable->attri[i].name == attriName)
		{
			if (tmpTable->attri[i].index != "")
			{
				return -2;
			}
			if (i == tmpTable->primaryKey)
			{
				return 1;			//在primarykey上可以直接建立索引，如果索引不存在
			}
			if (tmpTable->attri[i].isUnique == false)
			{
				return 0;			//attribute不是unique
			}
			break;
		}
	}
	if (i == tmpTable->attri.size())
	{
		return -3;
	}
	else
	{
		return 1;
	}
}