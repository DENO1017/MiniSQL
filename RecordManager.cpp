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
*	��������file����������conditionҪ�������record������
*	@param1: condition�б���attribute��Ϣ��Ӧ��table��ָ��
*	@param2: ���������д��Ƚϵ�attribute����valueֵ��condition�Ƚ�״̬������
*	return: ��������condition������״̬��record����
*/
int RecordManager::recordAllFind(Table* tmpTable, vector<Condition>conditionVector)
{
	Block* btmp = NULL;
	int count = 0;
	int blkNum = tmpTable->blockNum;
	for (int i = 0; i < blkNum; i++)		//�������table������block
	{
		int j = 0;
		for (j = 0; j < MaxBlock; j++)
		{
			if (bm.returnBlock(j)->id == i && bm.returnBlock(j)->fileName == tmpTable->name)		//��buffer���ҵ��˶�Ӧtable�ж�Ӧ��ŵ�block
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
		if (j == MaxBlock)		//û����buffer���ҵ���Ӧblock����Ҫ��file���ҵ�����д�뵽block��
		{
			btmp = bm.WriteBlockToBuffer(tmpTable->name, i, DataBlock);		//��disk������д�뵽buffer��
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
*	��buffer��һ��block�в�������condition������record����
*	@param1: condition�б���attribute��Ϣ��Ӧ��table��ָ��
*	@param2: ���������д��Ƚϵ�attribute����valueֵ��condition�Ƚ�״̬������
*	@param3: ���Ƚϵ�blockָ��
*	return: �������block����������������
*/
int RecordManager::recordBlockFind(Table* tmpTable, vector<Condition>conditionVector, Block* btmp)
{
	int count = 0;
	int recordSize = tmpTable->record_Size;
	int recordNum = (btmp->using_Size - btmp->head_Size) / recordSize;
	char* recordBegin = btmp->content + btmp->head_Size;
	for (int i = 0; i < recordNum; i++)			//����block�����е�record
	{
		count += recordConditionFit(recordBegin, tmpTable, conditionVector);
		recordBegin += recordSize;
	}
	return count;
}

/**
*	�ж�һ��record�Ƿ�����condition�����е�ÿһ������
*	@param1: ����record�ĵ�ַ�ͳ���
*	@param2: record�����ı�ָ��
*	@Param3: ��������������������
*	return: �����Ƿ���������
*/
int RecordManager::recordConditionFit(char* addressBegin, Table* tmptable, vector<Condition> conditionVector)
{
	myint tmpint;
	myfloat tmpfloat;
	int typeSize;
	char* contentBegin = addressBegin;
	string attriName;
	int type;
	for (int i = 0; i < tmptable->attri.size(); i++)		//��������record��ÿ��attribute
	{
		typeSize = getTypeSize(tmptable->attri[i].type);		//��¼ÿ��attribute�Ĵ�С
		type = tmptable->attri[i].type;
		attriName = tmptable->attri[i].name;
		char* value = new char[typeSize];
		memcpy(value, contentBegin, typeSize);				//���������record�е�ÿһ��attribute�е�valueֵ
		int j = 0;
		for (j = 0; j < conditionVector.size(); j++)				//����conditionVector�е���������
		{
			if (conditionVector[j].attriName == attriName)
			{
				if (tmptable->attri[i].type == INT)
				{
					memcpy(tmpint.byte, value, sizeof(int));
					if (conditionVector[j].checkInt(tmpint.num))		//�ж���һ���Ƿ���������
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
					if (conditionVector[j].checkFloat(tmpfloat.num))		//�ж���һ���Ƿ���������
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
					if (conditionVector[j].checkString(value))		//�ж���һ���Ƿ���������
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
*	��ʾ����block�е���Ӧrecord����
*	@param1: ָ���Ӧblock��ָ��
*	@param2: �洢��������vector
*/
int RecordManager::recordBlockShow(Block* btmp, vector<Condition>conditionVec, Table* tmpTable)
{
	int count = 0;
	char* address = NULL;
	address = btmp->content;
	myint tmpint;
	myfloat tmpfloat;
	address += 8;
	int recordNum = (btmp->using_Size - btmp->head_Size) / tmpTable->record_Size;			//��¼���block�е�record����
	if (conditionVec.size() == 0)		//��û����������Ҫȫ�������ʱ��
	{
		for (int i = 0; i < recordNum; i++)		//����block�е�ÿһ��record
		{
			for (int j = 0; j < tmpTable->attri.size(); j++)		//����ÿ��block��ÿ��record��ÿ��attribute
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
			if (recordConditionFit(address, tmpTable, conditionVec))		//�������record��������Ҫ��
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
*	ɾ��disk�е�file�ļ�
*	@param1: ����
*/
void RecordManager::DropTable(string tableName)
{
	string file = getFileName(tableName);
	remove(file.c_str());
	return;
}


/**
*	ɾ��file�е�ȫ������
*	@param1: ָ��table��ָ��
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
	int blockNum = tmpTable->blockNum;			//��¼���tableһ���м���block
	for (int i = 0; i < blockNum; i++)
	{
		fout.seekp(fileHeadSize + i * 4096);
		ifs.read(emptyArray, 4096);
	}
	delete[] emptyArray;
}


/**
*	��������ɾ��buffer��file�е����з���������record
*	@param1: ָ���Ӧ���ָ��
*	@param2: ����ֵ
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
	for (int i = 0; i < MaxBlock; i++)			//��ֹblock��©�����Ƚ������йص�buffer��blockȫ��д��file
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



int RecordManager::CreateIndex(char* indexName,char* tableName,char* attriName, int type, int degree)		//д��index��filehead������indexname, filename, 
{
	string indexFile = GetIndexFileName(indexName);
	int blockNum = 0;
	ofstream fout(indexFile, ios::out | ios::binary );
	int headSize = MAX_ATTRINAME * 2 + MAXNAMESIZE + sizeof(int) * 4;
	fout.write((char*)(&headSize), sizeof(int));				//�ļ�ͷ��С
	fout.write((char*)(&blockNum), sizeof(int));				//block��
	fout.write(indexName, MAX_ATTRINAME);				//��������
	fout.write(tableName, MAXNAMESIZE);				//����
	fout.write(attriName, MAX_ATTRINAME);				//����
	fout.write((char*)(&type), sizeof(int));					//����
	fout.write((char*)(&degree), sizeof(int));					//��
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
		if (recordConditionFit(address, tmpTable, conditionVec))		//�ҵ��˷���������ֵ
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