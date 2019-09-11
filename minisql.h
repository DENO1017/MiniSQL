#ifndef __MINISQL_H__
#define __MINISQL_H__
#include<iostream>
#include<vector>
#include<string>

#define INT -1
#define FLOAT 0
//#include "CatalogManager.h"
using namespace std;

int getTypeSize(int type);
string getFileName(string fileName);
string GetIndexFileName(string indexName);
struct Attribute
{
	string name;
	int type;
	bool isUnique;
	string index;
	Attribute(string _name, int _type, bool _ifUnique)
	{
		index = "";
		name = _name;
		type = _type;
		isUnique = _ifUnique;
	}
};

struct Table
{
	string name;
	vector <Attribute> attri;
	int recordNum;
	int blockNum;
	int primaryKey;
	int record_Size;
	Table(string _name, vector <Attribute> _attri, int _primarykey)
	{
		name = _name;
		attri = _attri;
		primaryKey = _primarykey;
		recordNum = 0;
		blockNum = 0;
		record_Size = 0;
		for (int i = 0; i < attri.size(); i++)
		{
			record_Size += getTypeSize(attri[i].type);
		}
	}
	Table(string _name, vector <Attribute> _attri, int _primarykey, int _recordNum, int _blockNum, int _record_Size)
	{
		name = _name;
		attri = _attri;
		primaryKey = _primarykey;
		recordNum = _recordNum;
		blockNum = _blockNum;
		record_Size = _record_Size;
	}
};



union myfloat
{
	float num;
	char byte[sizeof(float)];
};

union myint
{
	int num;
	char byte[sizeof(int)];
};

#endif // !__MINISQL_H__

