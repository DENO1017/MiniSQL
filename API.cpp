#include"API.h"

API::API()
{
	Init();
}

void API::Init()
{

}

void API::CreateTable(Table &tbl)
{
	if (cm->CreateTable(tbl))
		cm->tableList.push_back(tbl);
}


/**
*	API插入函数，由解释器调用
*	@param1: 表名
*	@param2: 输入record信息
*/
void API::InsertRecord(string tableName, vector<string> recordContent)
{
	Table* tmpTable;
	tmpTable = cm->findTable(tableName);
	if (tmpTable == NULL)
	{
		return;
	}
	int i = 0;
	int re;
	for (i = 0; i < tmpTable->attri.size(); i++)				//检查是否存在索引
	{
		if (tmpTable->attri[i].index != "")
		{
			re = im.SearchByIndex(recordContent[i], tmpTable->attri[i].index, tmpTable->attri[i].type, Condition::OPERATOR_EQUAL).size();
			if (re != 0)
			{
				cout << "Failure Message:: Wrong insert content" << endl;
				cout << "Failure Message:: The result should be unique!" << endl;
				cout << endl;
				return;
			}
		}
		else
		{
			continue;
		}
	}
	if (i == tmpTable->attri.size())
	{
		re = checkRecord(tmpTable, recordContent);
		if (re == -1)
		{
			cout << "Failure Message:: Wrong insert content" << endl;
			cout << "Failure Message:: The result should be unique!" << endl;
			cout << endl;
			return;
		}
		else if (re == 0)
		{
			cout << "Failure Message:: Wrong insert content" << endl;
			cout << "Failure Message:: The input doesn't have the same attribute as the " << tmpTable->name << " table!" << endl;
			cout << endl;
			return;
		}
	}
	char* charContent = new char[4096];
	memset(charContent, '\0', 4096);
	getContent(charContent, recordContent, tmpTable->attri);			//将record读入charContent数组并转换为二进制
	Block* btmp;
	if (tmpTable->blockNum == 0)
	{
		btmp = bm.findBlock(tmpTable, DataBlock, tmpTable->name, tmpTable->blockNum);
	}
	else
	{
		btmp = bm.findBlock(tmpTable, DataBlock, tmpTable->name, tmpTable->blockNum - 1);		//找到最后一个block
	}
	int blockOffsetNum = rm->Insert(btmp, btmp->content + btmp->using_Size, charContent, tmpTable->record_Size);
	int type;
	int typeSize;
	char* address = charContent;
	for (int i = 0; i < tmpTable->attri.size(); i++)			//将record中的所有有index内容插入到B+树中
	{
		type = tmpTable->attri[i].type;
		typeSize = getTypeSize(type);
		if (tmpTable->attri[i].index != "")
		{
			char* attriContent = new char[typeSize];
			memcpy(attriContent, address, typeSize);

			im.InsertKey(tmpTable->attri[i].index, attriContent, blockOffsetNum, type);

			delete[] attriContent;
		}
		address += typeSize;
	}
	tmpTable->recordNum++;
	cm->InsertRecord(tmpTable->name, tmpTable->recordNum, tmpTable->blockNum);
	printf("Success Message:: Insert record into table %s successful\n\n", tableName.c_str());
	bm.SetDirty(btmp);
	btmp->RLUcount++;
	delete[] charContent;
}


/**
*	类型转换函数，将保存在vector中的record信息存储在char*数组中
*	@param1: 目标char*数组
*	@param2: 待保存数据
*	#param3: 数据对应的attribute信息
*/
void API::getContent(char* charContent, vector<string> recordContent, vector <Attribute> attri)
{
	int type;
	myint tmpi;
	myfloat tmpf;
	char* address = charContent;
	for (int i = 0; i < recordContent.size(); i++)
	{
		type = attri[i].type;
		if (type == -1)			//type -1 为int
		{
			tmpi.num = stoi(recordContent[i]);
			memcpy(address, tmpi.byte, sizeof(int));
			address += sizeof(int);
		}
		else if (type == 0)      //type 0 为float
		{
			tmpf.num = stof(recordContent[i]);
			memcpy(address, tmpf.byte, sizeof(float));
			address += sizeof(float);
		}
		else                     //为char
		{
			strncpy(address, recordContent[i].c_str(), getTypeSize(type));
			address += getTypeSize(type);
		}
	}
}

/**
*	检查输入record是否为合法record
*	@param1: 要插入的表指针
*	@param2: 输入的内容
*	return 0: 输入内容列数与attribute数目不符
*	return 1: 检查结果正确
*	return -1: 
*/
int API::checkRecord(Table* tmpTable, vector<string>recordContent)
{
	if (tmpTable->attri.size() != recordContent.size())
	{
		return 0;
	}
	else                                                         //如果没有索引建立在这个attribute上，就遍历primary和unique，检查是否有重复的内容，如果有就返回错误
	{
		vector<Attribute> attri = tmpTable->attri;
		vector<Condition> conditionVec;
		for (int i = 0; i < attri.size(); i++)
		{
			if (attri[i].isUnique == true)		//这个attribute是unique的，需要检查是否重复
			{
				Condition tmpCondition(attri[i].name, recordContent[i], Condition::OPERATOR_EQUAL);
				conditionVec.push_back(tmpCondition);
			}
		}
		if (rm->recordAllFind(tmpTable, conditionVec))
		{
			return 0;
		}
		return 1;
	}
}


/**
*	根据表名显示表中的所有信息 select * from Book
*	@param1: 表名
*/
void API::RecordAllShow(string tableName, vector<Condition> conditionVec)
{
	int count = 0;
	Block* btmp = NULL;
	Table* tmpTable = NULL;
	int blockNum;
	int i = 0;
	for (i = 0; i < cm->tableList.size(); i++)		//使用指针指向对应的table
	{
		if (cm->tableList[i].name == tableName)
		{
			tmpTable = &cm->tableList[i];
			break;
		}
	}
	if (i == cm->tableList.size())
	{
		cout << "Failure Message::  table seleted doesn't exist" << endl;
		cout << endl;
		return;
	}
	if (CheckCondition(tmpTable, conditionVec) == 0)
	{
		cout << "Failure Message::  condition is not sutable" << endl;
		cout << endl;
		return;
	}
	PrintAttribute(tmpTable);
	blockNum = tmpTable->blockNum;			
	if (blockNum == 0)
	{
		cout << "Success Message:: The table selected is empty!!" << endl;
		cout << endl;
		return;
	}
	set<set<OffSetNumber>>Allsets;
	set<OffSetNumber>tmpSet;
	if (conditionVec.size() != 0)
	{
		for (int i = 0; i < conditionVec.size(); i++)			//遍历所有的condition vector
		{
			for (int j = 0; j < tmpTable->attri.size(); j++)
			{
				if (tmpTable->attri[j].name == conditionVec[i].attriName && tmpTable->attri[j].index!="")			//找到了符合的attribute
				{
					tmpSet = im.SearchByIndex(conditionVec[i].value, tmpTable->attri[j].index, tmpTable->attri[j].type, conditionVec[i].condition);
					Allsets.insert(tmpSet);
				}
			}
		}
		if (!Allsets.empty())
		{
			cout << "**************" << endl;
			set<OffSetNumber>result;
			set<OffSetNumber>tmp;
			set<set<OffSetNumber>>::iterator ite1 = Allsets.begin();
			set<set<OffSetNumber>>::iterator ite2 = Allsets.end();
			result = *ite1;
			for (; ite1 != ite2; ite1++)
			{
				tmp = *ite1;
				set<OffSetNumber>tmp2;
				set_intersection(result.begin(), result.end(), tmp.begin(), tmp.end(), insert_iterator<set<OffSetNumber> >(tmp2, tmp2.begin()));
				result = tmp2;
			}
			if (!result.empty())
			{
				set<OffSetNumber>::iterator it = result.begin();
				for (; it != result.end(); it++)
				{
					btmp = bm.findBlockById(tableName, *it);
					count += rm->recordBlockShow(btmp, conditionVec, tmpTable);
					bm.SetDirty(btmp);
					btmp->RLUcount++;
				}
				cout << "Success Message: There are " << count << " records are selected by index" << endl;
				cout << endl;
				return;
			}
		}
	}
	for (int i = 0; i < blockNum; i++)				//遍历这个table中的所有的block，首先用指针指向这个table所拥有的所有block
	{
		int j = 0;
		for (j = 0; j < MaxBlock; j++)			//查找buffer中是否存在编号为i的block
		{
			if (bm.returnBlock(j)->fileName == tableName && bm.returnBlock(j)->id == i)			//找到了这个block
			{
				btmp = bm.returnBlock(j);
				break;
			}
		}
		if (j == MaxBlock)			//block没有在buffer里边，需要从disk中写回
		{
			btmp = bm.WriteBlockToBuffer(tableName, i, DataBlock);
			myint tmpint;
			int headSize = btmp->head_Size;
			char* address = btmp->content + headSize;
			memcpy(tmpint.byte, address, sizeof(int));
		}
		count += rm->recordBlockShow(btmp, conditionVec, tmpTable);
		bm.SetDirty(btmp);
		btmp->RLUcount++;
	}
	cout << "Success Message:: There are " << count << " records are selected by non index" << endl;
	cout << "====================SELECT END====================" << endl;
	cout << endl;
	return;
}


void API::PrintAttribute(Table* tbl)
{
	cout << "====================" << tbl->name << "====================" << endl;
	for (int i = 0; i < tbl->attri.size(); i++)
	{
		cout << tbl->attri[i].name<<"   ";
	}
	cout << endl;
}


/**
*	检查输入的condition是否全部满足table要求
*	@param1: condition所对应的的table指针
*	@param2: 保存了condition的向量
*/
int API::CheckCondition(Table* tmpTable, vector<Condition>conditionVec)
{
	vector<Attribute> attri;
	attri = tmpTable->attri;
	for (int i = 0; i < conditionVec.size(); i++)
	{
		int j = 0;
		for (j = 0; j < attri.size(); j++)		//遍历table中的所有的attribute信息，检查condition中信息是否满足
		{
			if (conditionVec[i].attriName == attri[j].name)
			{
				break;
			}
		}
		if (j == attri.size())		//说明没有找到相等的attribute
		{
			return 0;
		}
	}
	return 1;
}

/**
*	删除表函数，删除关于这张表的所有信息
*	@param1: 表名
*/
void API::tableDrop(string tableName)
{
	int i = 0;
	Table* tmpTable = NULL;
	for (i = 0; i < cm->tableList.size(); i++)
	{
		if (cm->tableList[i].name == tableName)
		{
			
			tmpTable = &cm->tableList[i];
			break;
		}
	}
	if (i == cm->tableList.size())
	{
		cout << "Failure Message:: Table "<< tableName << " doesn't exist!" << endl;
		cout << "Failure Message:: Table drop failed!" << endl;
		cout << endl;
		return;
	}
	bm.DropTable(tableName);
	rm->DropTable(tableName);
	cm->DropTable(tableName);
	cout << "Success Message:: Table " << tableName << "drop successful!" << endl;
}


/**
*	删除对应表的所有record函数
*	@param1: 表名
*/
void API::deleteAllRecord(string tableName)
{
	Table* tmpTable = NULL;
	int i = 0;
	for (i = 0; i < cm->tableList.size(); i++)
	{
		if (cm->tableList[i].name == tableName)
		{
			tmpTable = &cm->tableList[i];
			break;
		}
	}
	if (i == cm->tableList.size())
	{
		cout << "Failure Message:: no table to delete" << endl;
		return;
	}
	for (int i = 0; i < tmpTable->attri.size(); i++)
	{
		if (tmpTable->attri[i].index != "")
		{
			im.DeleteAllNode(tmpTable->attri[i].index, tmpTable->attri[i].type);
		}
	}
	bm.DeleteAllRecord(tableName);
	rm->DeleteAllTableRecord(tmpTable);
	cm->DeleteAllTableRecord(tableName);
	cout << "Success Message:: All records in the " << tableName << " table are deleted successful!" << endl;
	cout << endl;
}


/**
*	按照条件删除file和block中的record
*	@param1: 对应表名
*	@param2: 条件值
*/
void API::DeleteConditionRecord(string tableName, Condition conditionValue)
{
	Table* tmpTable = NULL;
	int i = 0;
	for (i = 0; i < cm->tableList.size(); i++)
	{
		if (cm->tableList[i].name == tableName)
		{
			tmpTable = &cm->tableList[i];
			break;
		}
	}
	if (i == cm->tableList.size())
	{
		cout << "Failure Message:: Table selected doesn't exist" << endl;
		cout << endl;
		return;
	}
	set<OffSetNumber> tmpSet;
	string conditionAttri = conditionValue.attriName;
	i = 0;
	for (i = 0; i < tmpTable->attri.size(); i++)
	{
		if (tmpTable->attri[i].name == conditionAttri && tmpTable->attri[i].index != "")			//在table中找到条件的attribute并且存在index
		{
			tmpSet = im.SearchByIndex(conditionValue.value, tmpTable->attri[i].index, tmpTable->attri[i].type, conditionValue.condition);
			cout << "size = " << tmpSet.size() << endl;
			int recordDeleted = rm->DeleteConditionRecordByBlock(tmpTable, *tmpSet.begin(), conditionValue);
			cm->DeleteConditionRecord(tableName, recordDeleted);
			cout << "Success Message:: " << recordDeleted << " records are deleted by index" << endl;
			cout << endl;
			return;
		}
	}
	if (i == tmpTable->attri.size())
	{
		int recordDeleted = rm->DeleteConditionRecord(tmpTable, conditionValue);
		cm->DeleteConditionRecord(tableName, recordDeleted);
		cout << "Success Message:: " << recordDeleted << " records are deleted by non index" << endl;
		cout << endl;
	}
}


/**
*	建立索引
*	@param1: 索引名
*	@param2: 表名
*	@param3: 列名
*/
void API::CreateIndex(char* indexName, char*  tableName, char*  attriName)
{
	Table* tmpTable;
	int flag = cm->CheckAttri(tableName, attriName, attriName);
	if (flag == 0)
	{
		cout << "Failure Message:: Attribute " << attriName << " of table " << tableName << " is not unique" << endl;
		cout << "Failure Message:: Index create failed!" << endl;
		cout << endl;
		return;
	}
	else if (flag == -1)
	{
		cout << "Failure Message:: Table " << tableName << " doesn't exist!" << endl;
		cout << "Failure Message:: Index create failed!" << endl;
		cout << endl;
		return;
	}
	else if (flag == -2)
	{
		cout << "Failure Message:: Index " << indexName << " already exist!" << endl;
		cout << "Failure Message:: Index create failed!" << endl;
		cout << endl;
		return;
	}
	else if (flag == -3)
	{
		cout << "Failure Message:: Attribute " << attriName << " doesn't exist!" << endl;
		cout << "Failure Message:: Index create failed!" << endl;
		cout << endl;
		return;
	}
	tmpTable = cm->findTable(tableName);
	int attriType = cm->GetIndexType(tableName, attriName);
	if (attriType == -2)
	{
		cout << "Failure Message:: Index type error" << endl;
		cout << "Failure Message:: Index create failed!" << endl;
		cout << endl;
		return;
	}
	if (rm->CreateIndex(indexName, tableName, attriName, attriType, 5))		//建立索引文件夹
	{
		int i = 0;
		for (i = 0; i < tmpTable->attri.size(); i++)
		{
			if (tmpTable->attri[i].name == attriName)
			{
				tmpTable->attri[i].index = indexName;
				break;
			}
		}
		cm->CreateIndex(indexName, tableName, attriName);
		im.CreateIndex(indexName, attriType);				//建立索引，即B+树头结点和索引结构
		cout << "Success Message:: Create index " << indexName << " successful!" << endl;
		cout << endl;
		int count = InsertAllKeys(tmpTable, attriName, indexName);
		cout << "Success Message:: " << count <<" keys have been inserted into index " << indexName << endl;
		cout << endl;
	}
}


/**
*	建立索引时将表的所有key值全部传入索引中
*	@param1: 索引列所在的表指针
*	@param2: 索引所对应的列名
*	@param3: 索引命名
*/
int API::InsertAllKeys(Table* tmpTable, string attriName, string indexName)
{
	int count = 0;
	Block* btmp = NULL;
	int blockNum = tmpTable->blockNum;
	for (int i = 0; i < blockNum; i++)			//遍历整个表的所有block
	{
		btmp = bm.findBlockById(tmpTable->name, i);
		if (btmp == NULL)						//没有在buffer中找到对应的block
		{
			btmp = bm.WriteBlockToBuffer(tmpTable->name, i, DataBlock);
		}
		int recordSize = tmpTable->record_Size;
		int headSize = btmp->head_Size;
		int recordNum = (btmp->using_Size - headSize) / recordSize;
		char* address = btmp->content + headSize;
		int type;
		int typeSize;
		int j;
		for (j = 0; j < tmpTable->attri.size(); j++)		//将address指针指向要建立索引的attribute位置
		{
			if (tmpTable->attri[j].name == attriName)
			{
				break;
			}
			type = tmpTable->attri[j].type;
			typeSize = getTypeSize(type);
			address += typeSize;
		}
		type = tmpTable->attri[j].type;
		typeSize = getTypeSize(type);
		//typeSize保存了attriName的大小
		//type保存了attriName的类型
		for (int j = 0; j < recordNum; j++)
		{
			char* attriContent = new char[typeSize];
			memcpy(attriContent, address, typeSize);
			address += recordSize;
			im.InsertKey(indexName, attriContent, btmp->id, type);
			count++;
			delete[] attriContent;
		}
	}
	return count;
}



void API::IndexDrop(string indexName)
{
	Index* tmpIndex = NULL;
	tmpIndex = cm->FindIndex(indexName);
	if (tmpIndex == NULL)
	{
		cout << "Failure message:: Index " << indexName << " doesn't found!" << endl;
		cout << "Failure message:: Drop index failed!" << endl;
		cout << endl;
		return;
	}
	string tableName = tmpIndex->tableName;
	Table* tmpTable = cm->findTable(tableName);
	for (int i = 0; i < tmpTable->attri.size(); i++)
	{
		if (tmpTable->attri[i].index == indexName)
		{
			tmpTable->attri[i].index = "";
			break;
		}
	}
	cm->DropIndex(indexName);
	bm.DropIndex(tmpIndex->tableName);
	rm->DropIndex(indexName);
	im.DropIndex(indexName, tmpIndex->type);
	cout << "Success Message:: Drop index " << indexName << " successful" << endl;
	cout << endl;
}




void API::prepareToQuit()
{
	Index* tmpIndex = NULL;
	for (int i = 0; i < cm->indexList.size(); i++)
	{
		tmpIndex = cm->FindIndex(cm->indexList[i].indexName);
		if (tmpIndex == NULL)
		{
			cout << "Failure message:: Index " << cm->indexList[i].indexName << " is not found!" << endl;
			cout << "Failure message:: Saving index failed!" << endl;
			cout << endl;
			continue;
		}
		im.WriteBackAll(tmpIndex->indexName, tmpIndex->type);
	}
	cm->SaveAllFiles();
	bm.WriteBackAll();
	cm->prepareToQuit();
}



void API::InitProgram()
{
	cm->InitCatalog();
}



/**
*	用于debug，输出buffer中的block中所有content信息
*/
void API::print()
{
	Table * tmpTable = NULL;
	myint tmpint;
	myfloat tmpfloat;

	vector <Attribute> attri;
	for (int i = 0; i < MaxBlock; i++)
	{
		if (bm.returnBlock(i)->id != -1)
		{
			cout << "id: " << bm.returnBlock(i)->id << endl;
			cout << "i: " << i << endl;
			string fileName = bm.returnBlock(i)->fileName;
			for (int j = 0; j < cm->tableList.size(); j++)
			{
				if (cm->tableList[j].name == fileName)		//z找到对应这个block的table
				{
					tmpTable = &cm->tableList[j];
					attri = cm->tableList[j].attri;
					break;
				}
			}
			char* address = bm.returnBlock(i)->content;
			memcpy(tmpint.byte, address, sizeof(int));
			address += 4;
			memcpy(tmpint.byte, address, sizeof(int));
			address += 4;
			for (int k = 0; k < (bm.returnBlock(i)->using_Size - bm.returnBlock(i)->head_Size) / tmpTable->record_Size; k++)
			{
				for (int j = 0; j < attri.size(); j++)
				{
					int recordSize = getTypeSize(attri[j].type);
					if (attri[j].type == INT)
					{
						memcpy(tmpint.byte, address, recordSize);
						address += recordSize;
						cout << tmpint.num << endl;
					}
					else if (attri[j].type == FLOAT)
					{
						memcpy(tmpfloat.byte, address, recordSize);
						address += recordSize;
						cout << tmpfloat.num << endl;
					}
					else
					{
						char* tmpchar = new char[4096];
						memcpy(tmpchar, address, recordSize);
						address += recordSize;
						cout << tmpchar << endl;
						delete[] tmpchar;
					}
				}
			}
		}
	}
}

