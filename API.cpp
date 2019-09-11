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
*	API���뺯�����ɽ���������
*	@param1: ����
*	@param2: ����record��Ϣ
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
	for (i = 0; i < tmpTable->attri.size(); i++)				//����Ƿ��������
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
	getContent(charContent, recordContent, tmpTable->attri);			//��record����charContent���鲢ת��Ϊ������
	Block* btmp;
	if (tmpTable->blockNum == 0)
	{
		btmp = bm.findBlock(tmpTable, DataBlock, tmpTable->name, tmpTable->blockNum);
	}
	else
	{
		btmp = bm.findBlock(tmpTable, DataBlock, tmpTable->name, tmpTable->blockNum - 1);		//�ҵ����һ��block
	}
	int blockOffsetNum = rm->Insert(btmp, btmp->content + btmp->using_Size, charContent, tmpTable->record_Size);
	int type;
	int typeSize;
	char* address = charContent;
	for (int i = 0; i < tmpTable->attri.size(); i++)			//��record�е�������index���ݲ��뵽B+����
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
*	����ת����������������vector�е�record��Ϣ�洢��char*������
*	@param1: Ŀ��char*����
*	@param2: ����������
*	#param3: ���ݶ�Ӧ��attribute��Ϣ
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
		if (type == -1)			//type -1 Ϊint
		{
			tmpi.num = stoi(recordContent[i]);
			memcpy(address, tmpi.byte, sizeof(int));
			address += sizeof(int);
		}
		else if (type == 0)      //type 0 Ϊfloat
		{
			tmpf.num = stof(recordContent[i]);
			memcpy(address, tmpf.byte, sizeof(float));
			address += sizeof(float);
		}
		else                     //Ϊchar
		{
			strncpy(address, recordContent[i].c_str(), getTypeSize(type));
			address += getTypeSize(type);
		}
	}
}

/**
*	�������record�Ƿ�Ϊ�Ϸ�record
*	@param1: Ҫ����ı�ָ��
*	@param2: ���������
*	return 0: ��������������attribute��Ŀ����
*	return 1: �������ȷ
*	return -1: 
*/
int API::checkRecord(Table* tmpTable, vector<string>recordContent)
{
	if (tmpTable->attri.size() != recordContent.size())
	{
		return 0;
	}
	else                                                         //���û���������������attribute�ϣ��ͱ���primary��unique������Ƿ����ظ������ݣ�����оͷ��ش���
	{
		vector<Attribute> attri = tmpTable->attri;
		vector<Condition> conditionVec;
		for (int i = 0; i < attri.size(); i++)
		{
			if (attri[i].isUnique == true)		//���attribute��unique�ģ���Ҫ����Ƿ��ظ�
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
*	���ݱ�����ʾ���е�������Ϣ select * from Book
*	@param1: ����
*/
void API::RecordAllShow(string tableName, vector<Condition> conditionVec)
{
	int count = 0;
	Block* btmp = NULL;
	Table* tmpTable = NULL;
	int blockNum;
	int i = 0;
	for (i = 0; i < cm->tableList.size(); i++)		//ʹ��ָ��ָ���Ӧ��table
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
		for (int i = 0; i < conditionVec.size(); i++)			//�������е�condition vector
		{
			for (int j = 0; j < tmpTable->attri.size(); j++)
			{
				if (tmpTable->attri[j].name == conditionVec[i].attriName && tmpTable->attri[j].index!="")			//�ҵ��˷��ϵ�attribute
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
	for (int i = 0; i < blockNum; i++)				//�������table�е����е�block��������ָ��ָ�����table��ӵ�е�����block
	{
		int j = 0;
		for (j = 0; j < MaxBlock; j++)			//����buffer���Ƿ���ڱ��Ϊi��block
		{
			if (bm.returnBlock(j)->fileName == tableName && bm.returnBlock(j)->id == i)			//�ҵ������block
			{
				btmp = bm.returnBlock(j);
				break;
			}
		}
		if (j == MaxBlock)			//blockû����buffer��ߣ���Ҫ��disk��д��
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
*	��������condition�Ƿ�ȫ������tableҪ��
*	@param1: condition����Ӧ�ĵ�tableָ��
*	@param2: ������condition������
*/
int API::CheckCondition(Table* tmpTable, vector<Condition>conditionVec)
{
	vector<Attribute> attri;
	attri = tmpTable->attri;
	for (int i = 0; i < conditionVec.size(); i++)
	{
		int j = 0;
		for (j = 0; j < attri.size(); j++)		//����table�е����е�attribute��Ϣ�����condition����Ϣ�Ƿ�����
		{
			if (conditionVec[i].attriName == attri[j].name)
			{
				break;
			}
		}
		if (j == attri.size())		//˵��û���ҵ���ȵ�attribute
		{
			return 0;
		}
	}
	return 1;
}

/**
*	ɾ��������ɾ���������ű��������Ϣ
*	@param1: ����
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
*	ɾ����Ӧ�������record����
*	@param1: ����
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
*	��������ɾ��file��block�е�record
*	@param1: ��Ӧ����
*	@param2: ����ֵ
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
		if (tmpTable->attri[i].name == conditionAttri && tmpTable->attri[i].index != "")			//��table���ҵ�������attribute���Ҵ���index
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
*	��������
*	@param1: ������
*	@param2: ����
*	@param3: ����
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
	if (rm->CreateIndex(indexName, tableName, attriName, attriType, 5))		//���������ļ���
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
		im.CreateIndex(indexName, attriType);				//������������B+��ͷ���������ṹ
		cout << "Success Message:: Create index " << indexName << " successful!" << endl;
		cout << endl;
		int count = InsertAllKeys(tmpTable, attriName, indexName);
		cout << "Success Message:: " << count <<" keys have been inserted into index " << indexName << endl;
		cout << endl;
	}
}


/**
*	��������ʱ���������keyֵȫ������������
*	@param1: ���������ڵı�ָ��
*	@param2: ��������Ӧ������
*	@param3: ��������
*/
int API::InsertAllKeys(Table* tmpTable, string attriName, string indexName)
{
	int count = 0;
	Block* btmp = NULL;
	int blockNum = tmpTable->blockNum;
	for (int i = 0; i < blockNum; i++)			//���������������block
	{
		btmp = bm.findBlockById(tmpTable->name, i);
		if (btmp == NULL)						//û����buffer���ҵ���Ӧ��block
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
		for (j = 0; j < tmpTable->attri.size(); j++)		//��addressָ��ָ��Ҫ����������attributeλ��
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
		//typeSize������attriName�Ĵ�С
		//type������attriName������
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
*	����debug�����buffer�е�block������content��Ϣ
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
				if (cm->tableList[j].name == fileName)		//z�ҵ���Ӧ���block��table
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

