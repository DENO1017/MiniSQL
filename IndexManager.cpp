#include"IndexManager.h"



/**
*	建立索引，建立B+树的根节点，并添加到响应的map中
*	@param1: 索引名称
*	@param2: 索引类型
*/
void IndexManager::CreateIndex(string indexName, int type, int degree)
{
	if (type == INT)
	{
		BPlusTree<int> *tree = new BPlusTree<int>(degree);
		intIdxTrees.insert(intIndexMap::value_type(indexName, tree));
	}
	else if (type == FLOAT)
	{
		BPlusTree<float> *tree = new BPlusTree<float>(degree);
		floatIdxTrees.insert(floatIndexMap::value_type(indexName, tree));
	}
	else
	{
		BPlusTree<string> *tree = new BPlusTree<string>(degree);
		stringIdxTrees.insert(stringIndexMap::value_type(indexName, tree));
	}
}



/**
*	插入值，查找到响应的b+树并调用函数插入值
*	@param1: table中的key值
*	@param2: key值对应的blockid
*	@param3: 索引类型
*/
void IndexManager::InsertKey(string indexName, string attriContent, int blockId, int type)
{
	if (type == INT)
	{
		myint tmpint;
		memcpy(tmpint.byte, attriContent.c_str(), sizeof(int));
		intIndexMap::iterator itInt = intIdxTrees.find(indexName);
		if (itInt == intIdxTrees.end())									//没有找到报错返回
		{
			cout << "Error:in search index, no index " << indexName << " exits" << endl;
			return;
		}
		else
		{
			cout << "tmpint.num=" << tmpint.num << endl;
			itInt->second->insertKey(tmpint.num, blockId);////////////////待修改
		}
	}
	else if (type == FLOAT)
	{
		myfloat tmpfloat;
		memcpy(tmpfloat.byte, attriContent.c_str(), sizeof(float));
		floatIndexMap::iterator itFloat = floatIdxTrees.find(indexName);
		if (itFloat == floatIdxTrees.end())
		{
			cout << "Error:in search index, no index " << indexName << " exits" << endl;
			return;
		}
		else
		{
			itFloat->second->insertKey(tmpfloat.num, blockId);
		}
	}
	else
	{
		stringIndexMap::iterator itString = stringIdxTrees.find(indexName);
		if (itString == stringIdxTrees.end())
		{
			cout << "Error:in search index, no index " << indexName << " exits" << endl;
			return;
		}
		else
		{
			itString->second->insertKey(attriContent, blockId);
		}
	}
}



/**
*	查找值，按照索引查找值所在的blockid
*	@param1: 要查找的内容
*	@param2: 索引名
*	@param3: 类型
*/
set<OffSetNumber> IndexManager::SearchByIndex(string recordContent, string indexName, int type, int operation)
{
	set<OffSetNumber>setnum;
	if (type == INT)
	{
		int content;
		content = stoi(recordContent);
		intIndexMap::iterator itInt = intIdxTrees.find(indexName);
		if (itInt == intIdxTrees.end())									//没有找到报错返回
		{
			cout << "Error:in search index, no index " << indexName << " exits" << endl;
			return setnum;
		}
		else
		{
			set<OffSetNumber>temp =  itInt->second->searchKey(content, operation);
			return temp;
		}
	}
	else if (type == FLOAT)
	{
		float content;
		content = stof(recordContent);
		floatIndexMap::iterator itFloat = floatIdxTrees.find(indexName);
		if (itFloat == floatIdxTrees.end())
		{
			cout << "Error:in search index, no index " << indexName << " exits" << endl;
			return setnum;
		}
		else
		{
			return itFloat->second->searchKey(content, operation);
		}
	}
	else
	{
		stringIndexMap::iterator itString = stringIdxTrees.find(indexName);
		if (itString == stringIdxTrees.end())
		{
			cout << "Error:in search index, no index " << indexName << " exits" << endl;
			return setnum;
		}
		else
		{
			return itString->second->searchKey(recordContent, operation);
		}
	}

}


/**
*	根据键值删除B+树leaf
*	@param1: 索引名
*	@param2: 删除信息
*	@param3: 索引类型
*/
void IndexManager::DeleteIndexByKey(string indexName, char* attriContent, int type)
{
	if (type == INT)
	{
		myint tmpint;
		memcpy(tmpint.byte, attriContent, sizeof(int));
		intIndexMap::iterator itInt = intIdxTrees.find(indexName);
		if (itInt == intIdxTrees.end())									//没有找到报错返回
		{
			cout << "Error:in search index, no index " << indexName << " exits" << endl;
			return;
		}
		else
		{
			itInt->second->deleteKey(tmpint.num);
		}
	}
	else if (type == FLOAT)
	{
		myfloat tmpfloat;
		memcpy(tmpfloat.byte, attriContent, sizeof(float));
		floatIndexMap::iterator itFloat = floatIdxTrees.find(indexName);
		if (itFloat == floatIdxTrees.end())
		{
			cout << "Error:in search index, no index " << indexName << " exits" << endl;
			return;
		}
		else
		{
			itFloat->second->deleteKey(tmpfloat.num);
		}
	}
	else
	{
		stringIndexMap::iterator itString = stringIdxTrees.find(indexName);
		if (itString == stringIdxTrees.end())
		{
			cout << "Error:in search index, no index " << indexName << " exits" << endl;
			return;
		}
		else
		{
			itString->second->deleteKey(attriContent);
		}
	}
}


/**
*	删除一棵树的所有节点
*	@param1: 索引名
*	@param2: 索引类型
*/
void IndexManager::DeleteAllNode(string indexName, int type)
{
	if (type == INT)
	{
		intIndexMap::iterator itInt = intIdxTrees.find(indexName);
		itInt->second->init();
	}
	else if (type == FLOAT)
	{
		floatIndexMap::iterator itFloat = floatIdxTrees.find(indexName);
		itFloat->second->init();

	}
	else
	{
		stringIndexMap::iterator itString = stringIdxTrees.find(indexName);
		itString->second->init();
	}
}



void IndexManager::DropIndex(string indexName, int type)
{
	if (type == INT)
	{
		intIndexMap::iterator itInt = intIdxTrees.find(indexName);
		if (itInt == intIdxTrees.end())
		{
			cout << "Error：didn't find the int B+ tree!" << endl;
			return;
		}
		itInt->second->dropTree(itInt->second->root);
	}
	else if (type == FLOAT)
	{
		floatIndexMap::iterator itFloat = floatIdxTrees.find(indexName);
		if (itFloat == floatIdxTrees.end())
		{
			cout << "Error：didn't find the float B+ tree!" << endl;
			return;
		}
		itFloat->second->dropTree(itFloat->second->root);
	}
	else
	{
		stringIndexMap::iterator itString = stringIdxTrees.find(indexName);
		if (itString == stringIdxTrees.end())
		{
			cout << "Error：didn't find the string B+ tree!" << endl;
			return;
		}
		itString->second->dropTree(itString->second->root);
	}
}




void IndexManager::WriteBackAll(string indexName, int type)
{
	myint tmpint;
	int count = 1;
	string file = GetIndexFileName(indexName);
	Block* btmp = NULL;
	if (type == INT)
	{
		intIndexMap::iterator itInt = intIdxTrees.find(indexName);
		btmp = bm.getBlock(indexName);
		bm.SetDirty(btmp);
		btmp->RLUcount++;
		bm.blockSetValue(btmp, count - 1, IndexBlock, indexName);
		BPlusTree<int>* tmpTree = itInt->second;
		TreeNode<int> *ntmp = tmpTree->HeadLeaf;
		char* address = btmp->content + btmp->head_Size;
		while (ntmp != NULL)
		{
			for (int i = 0; i < ntmp->count; i++)
			{
				if (btmp->using_Size + 2 * sizeof(int) <= 4096)
				{
					tmpint.num = ntmp->value[i];
					memcpy(address, tmpint.byte, sizeof(int));
					address += sizeof(int);
					tmpint.num = ntmp->blockId[i];
					memcpy(address, tmpint.byte, sizeof(int));
					address += sizeof(int);
					btmp->using_Size += 2 * sizeof(int);
				}
				else
				{
					bm.WriteBackToDisk(indexName, count - 1, IndexBlock);
					count++;
					btmp = bm.getBlock(indexName);
					bm.SetDirty(btmp);
					btmp->RLUcount++;
					bm.blockSetValue(btmp, count - 1, IndexBlock, indexName);
					address = btmp->content + btmp->head_Size;
				}
			}
			ntmp = ntmp->next;
		}
		bm.WriteBackToDisk(indexName, count - 1, IndexBlock);
	}
	else if (type == FLOAT)
	{
		myfloat tmpfloat;
		floatIndexMap::iterator itFloat = floatIdxTrees.find(indexName);
		btmp = bm.getBlock(indexName);
		bm.SetDirty(btmp);
		btmp->RLUcount++;
		bm.blockSetValue(btmp, count - 1, IndexBlock, indexName);
		BPlusTree<float>* tmpTree = itFloat->second;
		TreeNode<float> *ntmp = tmpTree->HeadLeaf;
		char* address = btmp->content + btmp->head_Size;
		while (ntmp != NULL)
		{
			for (int i = 0; i < ntmp->count; i++)
			{
				if (btmp->using_Size + sizeof(float) + sizeof(int) <= 4096)
				{
					tmpfloat.num = ntmp->value[i];
					memcpy(address, tmpfloat.byte, sizeof(float));
					address += sizeof(float);
					tmpint.num = ntmp->blockId[i];
					memcpy(address, tmpint.byte, sizeof(int));
					address += sizeof(int);
					btmp->using_Size += sizeof(int) + sizeof(float);
				}
				else
				{
					bm.WriteBackToDisk(indexName, count - 1, IndexBlock);
					count++;
					btmp = bm.getBlock(indexName);
					bm.SetDirty(btmp);
					btmp->RLUcount++;
					bm.blockSetValue(btmp, count - 1, IndexBlock, indexName);
					address = btmp->content + btmp->head_Size;
				}
			}
			ntmp = ntmp->next;
		}
		bm.WriteBackToDisk(indexName, count - 1, IndexBlock);
	}
	else
	{
		stringIndexMap::iterator itString = stringIdxTrees.find(indexName);
		btmp = bm.getBlock(indexName);
		bm.SetDirty(btmp);
		btmp->RLUcount++;
		bm.blockSetValue(btmp, count - 1, IndexBlock, indexName);
		BPlusTree<string>* tmpTree = itString->second;
		TreeNode<string> *ntmp = tmpTree->HeadLeaf;
		char* address = btmp->content + btmp->head_Size;
		int typeSize = getTypeSize(type);
		while (ntmp != NULL)
		{
			for (int i = 0; i < ntmp->count; i++)
			{
				if (btmp->using_Size + typeSize + sizeof(int) <= 4096)
				{
					memcpy(address, ntmp->value[i].c_str(), typeSize);
					address += typeSize;
					tmpint.num = ntmp->blockId[i];
					memcpy(address, tmpint.byte, sizeof(int));
					address += sizeof(int);
					btmp->using_Size += sizeof(int) + typeSize;
				}
				else
				{
					bm.WriteBackToDisk(indexName, count - 1, IndexBlock);
					count++;
					btmp = bm.getBlock(indexName);
					bm.SetDirty(btmp);
					btmp->RLUcount++;
					bm.blockSetValue(btmp, count - 1, IndexBlock, indexName);
					address = btmp->content + btmp->head_Size;
				}
			}
			ntmp = ntmp->next;
		}
		bm.WriteBackToDisk(indexName, count - 1, IndexBlock);
	}
	string tmpfile = GetIndexFileName(indexName);
	fstream fout;
	fout.open(tmpfile, ios::binary | ios::in | ios::out);
	fout.seekp(4, ios::beg);
	fout.write((char*)(&count), sizeof(int));
	fout.close();
}



void IndexManager::InitTree(string indexName, int blockNum, int type, int degree)
{
	string file = GetIndexFileName(indexName);
	fstream ifs;
	ifs.open(file, ios::binary | ios::in);
	myint tmpint1;
	myint tmpint2;
	int fileHeadSize;
	ifs.read(tmpint1.byte, sizeof(int));
	fileHeadSize = tmpint1.num;
	Block* btmp = NULL;
	char* address;
	CreateIndex(indexName, type, degree);
	int blockID;
	if (type == INT)
	{
		for (int i = 0; i < blockNum; i++)
		{
			btmp = bm.WriteBlockToBuffer(indexName, i, IndexBlock);
			int recordSize = sizeof(int) + getTypeSize(type);
			int recordNum = (btmp->using_Size - btmp->head_Size) / recordSize;
			address = btmp->content + btmp->head_Size;
			for (int j = 0; j < recordNum; j++)
			{
				ifs.seekg(fileHeadSize + btmp->head_Size + recordSize * j);
				ifs.read(tmpint1.byte, sizeof(int));
				ifs.seekg(fileHeadSize + btmp->head_Size + recordSize * j + sizeof(int));
				ifs.read(tmpint2.byte, sizeof(int));
				blockID = tmpint2.num;
				char* tmpchar = new char[4];
				memcpy(tmpchar, tmpint1.byte, 4);
				InsertKey(indexName, tmpchar, blockID, type);
			}
			bm.initBlock(*btmp);
			bm.DeclineTotalBlock();
		}
	}
	else if (type == FLOAT)
	{
		myfloat tmpfloat;
		for (int i = 0; i < blockNum; i++)
		{
			btmp = bm.WriteBlockToBuffer(indexName, i, IndexBlock);
			int recordSize = sizeof(int) + getTypeSize(type);
			int recordNum = (btmp->using_Size - btmp->head_Size) / recordSize;
			address = btmp->content + btmp->head_Size;
			for (int j = 0; j < recordNum; j++)
			{
				ifs.seekg(fileHeadSize + btmp->head_Size + recordSize * j);
				ifs.read(tmpfloat.byte, sizeof(float));
				ifs.seekg(fileHeadSize + btmp->head_Size + recordSize * j + sizeof(float));
				ifs.read(tmpint2.byte, sizeof(int));
				blockID = tmpint2.num;
				char* tmpchar = new char[4];
				memcpy(tmpchar, tmpfloat.byte, 4);
				InsertKey(indexName, tmpchar, blockID, type);
			}
			bm.initBlock(*btmp);
			bm.DeclineTotalBlock();
		}
	}
	else
	{
		char* tmpchar = new char[getTypeSize(type)];
		for (int i = 0; i < blockNum; i++)
		{
			btmp = bm.WriteBlockToBuffer(indexName, i, IndexBlock);
			int recordSize = sizeof(int) + getTypeSize(type);
			int recordNum = (btmp->using_Size - btmp->head_Size) / recordSize;
			address = btmp->content + btmp->head_Size;
			for (int j = 0; j < recordNum; j++)
			{
				ifs.seekg(fileHeadSize + btmp->head_Size + recordSize * j);
				ifs.read(tmpchar, getTypeSize(type));
				ifs.seekg(fileHeadSize + btmp->head_Size + recordSize * j + getTypeSize(type));
				ifs.read(tmpint2.byte, sizeof(int));
				blockID = tmpint2.num;
				InsertKey(indexName, tmpchar, blockID, type);
			}
			bm.initBlock(*btmp);
			bm.DeclineTotalBlock();
		}
	}
}