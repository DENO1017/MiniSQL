#include"minisql.h"

int getTypeSize(int type)
{
	if (type == -1)
	{
		return sizeof(int);
	}
	else if (type == 0)
	{
		return sizeof(float);
	}
	else if (type > 0)
	{
		return type;
	}
	else
	{
		cout << "Syntax Error for wrong type" << endl;
		return -2;
	}
}

string getFileName(string fileName)
{
	return "TABLE_" + fileName + ".dat";
}

string GetIndexFileName(string indexName)
{
	string fileName = "INDEX_" + indexName + ".dat";
	return fileName;
}