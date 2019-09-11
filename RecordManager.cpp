#include"RecordManager.h"


int RecordManager::Insert(Block* btmp, char* address, char* content, int recordSize)
{
	myint tmpint;
	memcpy(address, content, recordSize);
	btmp->using_Size += recordSize;
	char* tmp = btmp->content + 4;
	tmpint.num = btmp->using_Size;
	memcpy(tmp, tmpint.byte, sizeof(int));
	return btmp->id;
}


/**
*	遍历整个file，返回满足condition要求的所有record的条数
*	@param1: condition中保存attribute信息对应的table的指针
*	@param2: 保存了所有待比较的attribute名，value值和condition比较状态的向量
*	return: 返回满足condition中所有状态的record条数
*/
int RecordManager::recordAllFind(Table* tmpTable, vector<Condition>conditionVector)
{
	Block* btmp = NULL;
	int count = 0;
	int blkNum = tmpTable->blockNum;
	for (int i = 0; i < blkNum; i++)		//遍历这个table的所有block
	{
		int j = 0;
		for (j = 0; j < MaxBlock; j++)
		{
			if (bm.returnBlock(j)->id == i && bm.returnBlock(j)->fileName == tmpTable->name)		//在buffer中找到了对应table中对应编号的block
			{
				btmp = bm.returnBlock(j);
				count += recordBlockFind(tmpTable, conditionVector, btmp);
				break;
			}
			else
			{
				continue;
			}
		}
		if (j == MaxBlock)		//没有在buffer中找到对应block，需要到file中找到并且写入到block中
		{
			btmp = bm.WriteBlockToBuffer(tmpTable->name, i, DataBlock);		//将disk中内容写入到buffer中
			count += recordBlockFind(tmpTable, conditionVector, btmp);
		}
	}
	if (btmp != NULL)
	{
		bm.SetDirty(btmp);
		btmp->RLUcount++;
	}
	return count;
}

/**
*	在buffer的一个block中查找满足condition条件的record数量
*	@param1: condition中保存attribute信息对应的table的指针
*	@param2: 保存了所有待比较的attribute名，value值和condition比较状态的向量
*	@param3: 待比较的block指针
*	return: 返回这个block中满足条件的条数
*/
int RecordManager::recordBlockFind(Table* tmpTable, vector<Condition>conditionVector, Block* btmp)
{
	int count = 0;
	int recordSize = tmpTable->record_Size;
	int recordNum = (btmp->using_Size - btmp->head_Size) / recordSize;
	char* recordBegin = btmp->content + btmp->head_Size;
	for (int i = 0; i < recordNum; i++)			//遍历block中所有的record
	{
		count += recordConditionFit(recordBegin, tmpTable, conditionVector);
		recordBegin += recordSize;
	}
	return count;
}

/**
*	判断一条record是否满足condition向量中的每一个条件
*	@param1: 传入record的地址和长度
*	@param2: record所属的表指针
*	@Param3: 保存了满足条件的向量
*	return: 返回是否满足条件
*/
int RecordManager::recordConditionFit(char* addressBegin, Table* tmptable, vector<Condition> conditionVector)
{
	myint tmpint;
	myfloat tmpfloat;
	int typeSize;
	char* contentBegin = addressBegin;
	string attriName;
	int type;
	for (int i = 0; i < tmptable->attri.size(); i++)		//遍历整个record的每个attribute
	{
		typeSize = getTypeSize(tmptable->attri[i].type);		//记录每个attribute的大小
		type = tmptable->attri[i].type;
		attriName = tmptable->attri[i].name;
		char* value = new char[typeSize];
		memcpy(value, contentBegin, typeSize);				//保存了这个record中的每一个attribute中的value值
		int j = 0;
		for (j = 0; j < conditionVector.size(); j++)				//遍历conditionVector中的所有条件
		{
			if (conditionVector[j].attriName == attriName)
			{
				if (tmptable->attri[i].type == INT)
				{
					memcpy(tmpint.byte, value, sizeof(int));
					if (conditionVector[j].checkInt(tmpint.num))		//判断这一条是否满足条件
					{
						continue;
					}
					else
					{
						return 0;
					}
					
				}
				else if (tmptable->attri[i].type == FLOAT)
				{
					memcpy(tmpfloat.byte, value, sizeof(int));
					if (conditionVector[j].checkFloat(tmpfloat.num))		//判断这一条是否满足条件
					{
						continue;
					}
					else
					{
						return 0;
					}
				}
				else
				{
					if (conditionVector[j].checkString(value))		//判断这一条是否满足条件
					{
						continue;
					}
					else
					{
						return 0;
					}
				}
			}
		}
		contentBegin += typeSize;
		delete[] value;
	}
	return 1;
}

/**
*	显示传入block中的相应record内容
*	@param1: 指向对应block的指针
*	@param2: 存储了条件的vector
*/
int RecordManager::recordBlockShow(Block* btmp, vector<Condition>conditionVec, Table* tmpTable)
{
	int count = 0;
	char* address = NULL;
	address = btmp->content;
	myint tmpint;
	myfloat tmpfloat;
	address += 8;
	int recordNum = (btmp->using_Size - btmp->head_Size) / tmpTable->record_Size;			//记录这个block中的record数量
	if (conditionVec.size() == 0)		//当没有条件，需要全部输出的时候
	{
		for (int i = 0; i < recordNum; i++)		//遍历block中的每一条record
		{
			for (int j = 0; j < tmpTable->attri.size(); j++)		//对于每个block中每条record的每个attribute
			{
				int type = tmpTable->attri[j].type;
				int typeSize = getTypeSize(type);
				if (type == INT)
				{
					memcpy(tmpint.byte, address, sizeof(int));
					cout << tmpint.num << '\t';
				}
				else if (type == FLOAT)
				{
					memcpy(tmpfloat.byte, address, sizeof(float));
					cout << tmpfloat.num << '\t';
				}
				else
				{
					char* content = new char[typeSize];
					memcpy(content, address, typeSize);
					cout << content << '\t';
					delete[] content;
				}
				address += typeSize;
			}
			count++;
			cout << endl;
		}
	}
	else
	{
		for (int i = 0; i < recordNum; i++)
		{
			if (recordConditionFit(address, tmpTable, conditionVec))		//如果这条record满足条件要求
			{
				for (int k = 0; k < tmpTable->attri.size(); k++)
				{
					int type = tmpTable->attri[k].type;
					int typeSize = getTypeSize(type);
					if (type == INT)
					{
						memcpy(tmpint.byte, address, sizeof(int));
						cout << tmpint.num << '\t';
					}
					else if (type == FLOAT)
					{
						memcpy(tmpfloat.byte, address, sizeof(float));
						cout << tmpfloat.num << '\t';
					}
					else
					{
						char* content = new char[typeSize];
						memcpy(content, address, typeSize);
						cout << content << '\t';
						delete[] content;
					}
					address += typeSize;
				}
				count++;
				cout << endl;
			}
			else
			{
				address += tmpTable->record_Size;
			}
		}
	}
	return count;
}


/**
*	删除disk中的file文件
*	@param1: 表名
*/
void RecordManager::DropTable(string tableName)
{
	string file = getFileName(tableName);
	remove(file.c_str());
	return;
}


/**
*	删除file中的全部内容
*	@param1: 指向table的指针
*/
void RecordManager::DeleteAllTableRecord(Table* tmpTable)
{
	Block* btmp = NULL;
	string file = getFileName(tmpTable->name);
	ofstream fout(file, ios::out | ios::binary);
	ifstream ifs(file, ios::binary);
	myint tmpint;
	char* emptyArray = new char[4096];
	memset(emptyArray, '\0', 4096);
	ifs.read(tmpint.byte, sizeof(int));
	int fileHeadSize = tmpint.num;
	int blockNum = tmpTable->blockNum;			//记录这个table一共有几个block
	for (int i = 0; i < blockNum; i++)
	{
		fout.seekp(fileHeadSize + i * 4096);
		ifs.read(emptyArray, 4096);
	}
	delete[] emptyArray;
}


/**
*	按照条件删除buffer与file中的所有符合条件的record
*	@param1: 指向对应表的指针
*	@param2: 条件值
*/
int RecordManager::DeleteConditionRecord(Table* tmpTable, Condition conditionValue)
{
	int count = 0;
	string tableName = tmpTable->name;
	string file = getFileName(tableName);
	Block* btmp = NULL;
	vector<Condition> conditionVec;
	conditionVec.push_back(conditionValue);
	int blockNum = tmpTable->blockNum;
	int recordSize = tmpTable->record_Size;
	char* recordContent = new char[4096];
	memset(recordContent, '\0', 4096);
	for (int i = 0; i < MaxBlock; i++)			//防止block遗漏，首先将所有有关的buffer中block全部写回file
	{
		if (bm.returnBlock(i)->fileName == tableName)
		{
			bm.WriteBackToDisk(tableName, i, DataBlock);
		}
	}
	int fileHeadSize;
	ifstream ifs(file, ios::binary);
	myint tmpint;
	ifs.read(tmpint.byte, sizeof(int));
	fileHeadSize = tmpint.num;
	for (int i = 0; i < blockNum; i++)
	{
		int recordNum;
		btmp = bm.WriteBlockToBuffer(tableName, i, DataBlock);
		recordContent = btmp->content + btmp->head_Size;
		recordNum = (btmp->using_Size - btmp->head_Size) / recordSize;
		for (int j = 0; j < recordNum; j++)
		{
			if (recordConditionFit(recordContent, tmpTable, conditionVec))
			{
				char* tmp = recordContent;
				for (int k = 0; k < recordNum - j; k++)
				{
					memcpy(tmp, tmp + recordSize, recordSize);
					tmp += recordSize;
				}
				btmp->using_Size -= recordSize;
				char* changeSize = btmp->content + 4;
				tmpint.num = btmp->using_Size;
				memcpy(changeSize, tmpint.byte, sizeof(int));
				count++;
			}
			else
			{
				recordContent += recordSize;
			}
		}
		bm.SetDirty(btmp);
		btmp->RLUcount++;
	}
	return count;
}



int RecordManager::CreateIndex(char* indexName,char* tableName,char* attriName, int type, int degree)		//写入index的filehead，包括indexname, filename, 
{
	string indexFile = GetIndexFileName(indexName);
	int blockNum = 0;
	ofstream fout(indexFile, ios::out | ios::binary );
	int headSize = MAX_ATTRINAME * 2 + MAXNAMESIZE + sizeof(int) * 4;
	fout.write((char*)(&headSize), sizeof(int));				//文件头大小
	fout.write((char*)(&blockNum), sizeof(int));				//block数
	fout.write(indexName, MAX_ATTRINAME);				//索引名称
	fout.write(tableName, MAXNAMESIZE);				//表名
	fout.write(attriName, MAX_ATTRINAME);				//列名
	fout.write((char*)(&type), sizeof(int));					//类型
	fout.write((char*)(&degree), sizeof(int));					//度
	//fout.close();
	return 1;
}



int RecordManager::DeleteConditionRecordByBlock(Table* tmpTable, int blockId, Condition conditionValue)
{
	myint tmpint;
	int count = 0;
	Block* btmp = NULL;
	int i = 0;
	for (i = 0; i < MaxBlock; i++)
	{
		if (bm.returnBlock(i)->fileName == tmpTable->name && bm.returnBlock(i)->id == blockId)
		{
			btmp = bm.returnBlock(i);
			bm.SetDirty(btmp);
			btmp->RLUcount++;
			break;
		}
	}
	if (i == MaxBlock)
	{
		btmp = bm.WriteBlockToBuffer(tmpTable->name, blockId, DataBlock);
	}
	char* address = btmp->content;
	int recordSize = tmpTable->record_Size;
	int recordNum = (btmp->using_Size - btmp->head_Size) / recordSize;
	address += btmp->head_Size;
	int type;
	int typeSize;
	vector<Condition> conditionVec;
	conditionVec.push_back(conditionValue);
	for (int i = 0; i < recordNum; i++)
	{
		if (recordConditionFit(address, tmpTable, conditionVec))		//找到了符合条件的值
		{
			char* tmpChar = address;
			for (int j = 0; j < tmpTable->attri.size(); j++)
			{
				int tmpType = tmpTable->attri[j].type;
				int tmpTypeSize = getTypeSize(tmpType);
				if (tmpTable->attri[j].index != "")
				{
					char* recordContent = new char[tmpTypeSize];
					memcpy(recordContent, tmpChar, tmpTypeSize);
					im.DeleteIndexByKey(tmpTable->attri[j].index, recordContent, tmpType);
					delete[] recordContent;
				}
				tmpChar += tmpTypeSize;
			}
			char* tmp = address;
			for (int k = 0; k < recordNum - i; k++)
			{
				memcpy(tmp, tmp + recordSize, recordSize);
				tmp += recordSize;
			}
			btmp->using_Size -= recordSize;
			char* changeSize = btmp->content + 4;
			tmpint.num = btmp->using_Size;
			memcpy(changeSize, tmpint.byte, sizeof(int));
			count++;
		}
		else
		{
			address += recordSize;
		}
	}
	return count;
}



void RecordManager::DropIndex(string indexName)
{
	string file = GetIndexFileName(indexName);
	remove(file.c_str());
	return;
}