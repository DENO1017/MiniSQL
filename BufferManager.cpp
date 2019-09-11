#include"BufferManager.h"
#pragma warning(disable:4996)

BufferManager::BufferManager() : total_Block(0)
{
	for (int i = 0; i < MaxBlock; i++)
	{
		initBlock(blocks[i]);
	}
}

void BufferManager::initBlock(Block &block)
{
	if (block.content != NULL)
	{
		delete[] block.content;
	}
	block.content = new char[BlockSize];
	block.head_Size = 8;
	memset(block.content, '\0', BlockSize);
	block.blockType = DefaultBlock;
	block.fileName = "";
	block.id = -1;
	block.isDirty = false;
	block.using_Size = block.head_Size;
	block.RLUcount = 0;
}


/**
*	����һ���յ�Block
*	1.���Block������û��block���򷵻ص�һ��block
*	2.���Block�������Ѿ�����block�����ص�һ����������Ϣ��block
*	3.���Block���Ѿ������ڿյ�block����ͨ��RLUѡ��blockд�ص�disk����ʼ����block
*/
Block* BufferManager::getBlock(string _fileName)
{
	Block* btmp = NULL;
	if (total_Block == 0)				//����block��Ϊ��
	{
		total_Block++;
		btmp = &blocks[0];
	}
	else if (total_Block < MaxBlock)	//�Ѿ����ڲ���block����Ϣ������Ȼ���ڿյ�block
	{

		for (int i = 0; i < MaxBlock; i++)
		{
			if (blocks[i].id != -1)
			{
				continue;
			}
			else
			{
				btmp = &blocks[i];
				total_Block++;
				break;
			}
		}
	}
	else								//���е�block�Ѿ����ˣ���Ҫѡ��һ��blockд�ص�disk��
	{
		int minRlu = 100000;
		int writeBackBlock = -1;
		for (int i = 0; i < MaxBlock; i++)			//�������е�block���ҵ�dirty���ҷ��ʴ������ٵ�һ��block��Ϊд��block
		{
			if (blocks[i].isDirty)
			{
				if (minRlu > blocks[i].RLUcount)
				{
					minRlu = blocks[i].RLUcount;
					writeBackBlock = i;
				}
				else
				{
					continue;
				}
			}
			else
			{
				continue;
			}
		}
		if (writeBackBlock == -1)		//������ȫ������û���ҵ��ܹ�д�ص�block���򱨴���
		{
			cout << "Syntex Error: there is no block available any more" << endl;
			return NULL;
		}
		//�ҵ���Ҫд�ر��ΪwriteBackPosi��block������ú���д�ز���ʼ����block
		string fileName = blocks[writeBackBlock].fileName;
		if (WriteBackToDisk(fileName, blocks[writeBackBlock].id, DataBlock))
		{
			initBlock(blocks[writeBackBlock]);
			total_Block++;
			btmp = &blocks[writeBackBlock];
		}
	}
	return btmp;
}

/**
 *	������������block�����ض�Ӧ��blockָ��
 *	@param1: block���࣬table or index
 *	@param2: block�ļ���
 *	@param3: block���
 */
Block* BufferManager::findBlock(Table* tmpTable, int _blockType, string _fileName, int _blockID)
{
	Block* btmp = NULL;
	int i = 0;
	for (i = 0; i < MaxBlock; i++)
	{
		if (blocks[i].blockType == _blockType && blocks[i].fileName == _fileName && blocks[i].id == _blockID)		//���ҵ��˷���Ҫ���block
		{
			if (blocks[i].using_Size + tmpTable->record_Size < BlockSize)			//��ǰblockĩβ���㹻�ռ�����������¼
			{
				btmp = &blocks[i];
				break;
			}
			else                             //��ǰblock�ռ䲻�㣬��Ҫ�����µĿռ�
			{
				btmp = getBlock(_fileName);
				tmpTable->blockNum++;
				blockSetValue(btmp, _blockID + 1, _blockType, _fileName);
				break;
			}
		}
	}
	if (i == MaxBlock)			//������û���ҵ����ʵ�block����Ҫ�����µ�block
	{
		btmp = getBlock(_fileName);
		tmpTable->blockNum++;
		if (tmpTable->blockNum == 1)
		{
			blockSetValue(btmp, _blockID, _blockType, _fileName);
		}
		else
		{
			blockSetValue(btmp, _blockID + 1, _blockType, _fileName);
		}
	}
	return btmp;
}

/**
*	��ֵ��ʼ��һ���µ����е�block
*	contentд����ɺ���Ҫ����next��pre pointer����
*	@param1: Ҫ����ֵ��blockpointer
*	@param2: �µ�block��id
*	@param3: �µ�block��ʲô����
*	@param4: �µ�block��file��
*/
void BufferManager::blockSetValue(Block* btmp, int _id, int _BlockType, string _fileName)
{
	btmp->id = _id;
	myint tmpid;
	tmpid.num = btmp->id;
	memcpy(btmp->content, tmpid.byte, sizeof(int));		//��block��idд��content��
	myint tmpbyte;
	btmp->fileName = _fileName;
	btmp->blockType = _BlockType;
}

Block* BufferManager::returnBlock(int posi)
{
	Block* btmp;
	btmp = &blocks[posi];
	return btmp;
}

void BufferManager::SetDirty(Block* btmp)
{
	btmp->isDirty = true;
	return;
}

void BufferManager::CleanDirty(Block* btmp)
{
	btmp->isDirty = false;
	return;
}


/**
*	��λ����writeBackBlock��blockд�ص�disk��
*	@param1: ���block��Ӧ��table��
*	@param2: ���block
*/
int BufferManager::WriteBackToDisk(string fileName, int writeBackBlock, int blockType)
{
	myint tmpint1;
	myint tmpint2;
	Block* btmp = NULL;		//ָ���д�ص�block
	int posi = -1;
	int i = 0;
	for (i = 0; i < MaxBlock; i++)
	{
		if (blocks[i].fileName == fileName && blocks[i].id == writeBackBlock)
		{
			btmp = &blocks[i];
			posi = i;
			break;
		}
	}
	if (i == MaxBlock)
	{
		return 0;
	}
	if (blockType == DataBlock)
	{
		string file = getFileName(fileName);		//��ȡfile����
		int fileHeadSize;
		tmpint1.num = btmp->using_Size;
		char* address = NULL;		//����usingsize
		address = btmp->content + 4;
		memcpy(address, tmpint1.byte, 4);
		char* tmpContent = new char[4096];
		memcpy(tmpContent, btmp->content, 4096);		//ֱ�ӽ�block�е�����д��file��󼴿�
		fstream ifs(file, ios::in | ios::binary);
		char* tmp = new char[5];
		ifs.read(tmp, sizeof(int));       		//��ȡfile�ļ�ͷ��С
		memcpy(tmpint2.byte, tmp, 4);
		fileHeadSize = tmpint2.num;
		fstream fout(file, ios::binary | ios::in | ios::out);
		fout.seekp(fileHeadSize + 4096 * writeBackBlock);
		fout.write(tmpContent, 4096);
		fout.close();
		initBlock(*btmp);
		DeclineTotalBlock();
		delete[] tmpContent;
		delete[] tmp;
		return 1;
	}
	else if (blockType == IndexBlock)
	{
		tmpint1.num = btmp->using_Size;
		char* address = NULL;		//����usingsize
		address = btmp->content + 4;
		memcpy(address, tmpint1.byte, 4);
		char* tmpContent = new char[4096];
		memcpy(tmpContent, btmp->content, 4096);		//ֱ�ӽ�block�е�����д��file��󼴿�
		string file = GetIndexFileName(fileName);
		int fileHeadSize;
		fstream fout;
		fout.open(file, ios::in | ios::out | ios::binary | ios::app);
		fout.read(tmpint2.byte, sizeof(int));
		fileHeadSize = tmpint2.num;
		fout.seekp(fileHeadSize + 4096 * writeBackBlock, ios::beg);
		fout.write(btmp->content, 4096);
		fout.close();

		initBlock(*btmp);
		DeclineTotalBlock();
		delete[] tmpContent;
		return 1;
	}
}

/**
*	��file�ж�Ӧ��blockд�뵽buffer��
*	@param1: block��Ӧ��table��
*	@param2: file�е�blockid
*	return: ����д�ص�block
*/

Block* BufferManager::WriteBlockToBuffer(string fileName, int blockID, int blockType)
{
	Block* btmp = getBlock(fileName);
	string file;
	if (blockType == DataBlock)
	{
		file = getFileName(fileName);
	}
	else if (blockType == IndexBlock)
	{
		file = GetIndexFileName(fileName);
	}
	char* address = NULL;
	myint tmpint;
	int headSize;
	char* buffer = new char[sizeof(int)];
	fstream ifs(file, ios::binary | ios::in);		//��ȡfileͷ�Ĵ�С
	ifs.read(buffer, 4);
	memcpy(tmpint.byte, buffer, sizeof(int));		//��fileͷ�Ĵ�Сת��Ϊʮ����
	headSize = tmpint.num;
	int usingSize;
	char* content = new char[4096];
	memset(content, '\0', 4096);
	ifs.seekg(headSize + 4096 * blockID, ios::beg);		//��ָ��ָ��Ҫ��ȡ��blockID��λ��
	ifs.read(content, 4096);			//��file��ߵ�contentд�����content������
	address = content + sizeof(int);		//addressָ��usingSize
	memcpy(tmpint.byte, address, sizeof(int));
	usingSize = tmpint.num;
	btmp->using_Size = usingSize;
	blockSetValue(btmp, blockID, DataBlock, fileName);
	memcpy(btmp->content, content, 4096);
	SetDirty(btmp);
	btmp->RLUcount++;
	delete[] buffer;
	delete[] content;
	return btmp;
}

/**
*	ɾ��block�е������������table����Ϣ
*	@param1: ����
*/
void BufferManager::DropTable(string tableName)
{
	for (int i = 0; i < MaxBlock; i++)
	{
		if (blocks[i].fileName == tableName)
		{
			initBlock(blocks[i]);
			DeclineTotalBlock();
		}
	}
}


/**
*	ɾ����Ӧ������м�¼
*	@param1: ����
*/
void BufferManager::DeleteAllRecord(string tableName)
{
	for (int i = 0; i < MaxBlock; i++)
	{
		if (blocks[i].fileName == tableName)		//����ҵ��˱��Ӧ��block��ֱ��ɾ�����block�е��������ݼ���
		{
			initBlock(blocks[i]);
			DeclineTotalBlock();
		}
	}
}



char* BufferManager::get_content(Block* btmp)
{
	return btmp->content;
}



Block* BufferManager::findBlockById(string tableName, int id)
{
	Block* btmp = NULL;
	for (int i = 0; i < MaxBlock; i++)
	{
		if (blocks[i].fileName == tableName && blocks[i].id == id)
		{
			btmp = &blocks[i];

			return btmp;
		}
	}
	if (btmp == NULL)
	{
		btmp = WriteBlockToBuffer(tableName, id, DataBlock);
	}
	return btmp;
}



void BufferManager::DropIndex(string tableName)
{
	for (int i = 0; i < MaxBlock; i++)
	{
		if (blocks[i].blockType == IndexBlock && blocks[i].fileName == tableName)
		{
			initBlock(blocks[i]);
			DeclineTotalBlock();
		}
	}
}


void BufferManager::WriteBackAll()
{
	for (int i = 0; i < MaxBlock; i++)
	{
		if (blocks[i].blockType == IndexBlock)
		{
			initBlock(blocks[i]);
			DeclineTotalBlock();
			continue;
		}
		else
		{
			if (blocks[i].id != -1)
			{
				WriteBackToDisk(blocks[i].fileName, blocks[i].id, DataBlock);
			}
			continue;
		}
	}
}


void BufferManager::DeclineTotalBlock()
{
	if (total_Block > 0)
	{
		total_Block--;
	}
}