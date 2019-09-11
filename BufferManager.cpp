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
*	返回一个空的Block
*	1.如果Block数组中没有block，则返回第一个block
*	2.如果Block数组中已经存在block，返回第一个不包含信息的block
*	3.如果Block中已经不存在空的block，则通过RLU选择block写回到disk并初始化该block
*/
Block* BufferManager::getBlock(string _fileName)
{
	Block* btmp = NULL;
	if (total_Block == 0)				//所有block均为空
	{
		total_Block++;
		btmp = &blocks[0];
	}
	else if (total_Block < MaxBlock)	//已经存在部分block有信息，但仍然存在空的block
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
	else								//所有的block已经满了，需要选择一个block写回到disk中
	{
		int minRlu = 100000;
		int writeBackBlock = -1;
		for (int i = 0; i < MaxBlock; i++)			//遍历所有的block，找到dirty并且访问次数最少的一个block设为写回block
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
		if (writeBackBlock == -1)		//遍历了全部还是没有找到能够写回的block，则报错返回
		{
			cout << "Syntex Error: there is no block available any more" << endl;
			return NULL;
		}
		//找到了要写回编号为writeBackPosi的block，则调用函数写回并初始化该block
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
 *	根据条件查找block，返回对应的block指针
 *	@param1: block种类，table or index
 *	@param2: block文件名
 *	@param3: block序号
 */
Block* BufferManager::findBlock(Table* tmpTable, int _blockType, string _fileName, int _blockID)
{
	Block* btmp = NULL;
	int i = 0;
	for (i = 0; i < MaxBlock; i++)
	{
		if (blocks[i].blockType == _blockType && blocks[i].fileName == _fileName && blocks[i].id == _blockID)		//查找到了符合要求的block
		{
			if (blocks[i].using_Size + tmpTable->record_Size < BlockSize)			//当前block末尾有足够空间增加这条记录
			{
				btmp = &blocks[i];
				break;
			}
			else                             //当前block空间不足，需要开辟新的空间
			{
				btmp = getBlock(_fileName);
				tmpTable->blockNum++;
				blockSetValue(btmp, _blockID + 1, _blockType, _fileName);
				break;
			}
		}
	}
	if (i == MaxBlock)			//遍历后没有找到合适的block，需要开辟新的block
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
*	按值初始化一个新的入列的block
*	content写入完成后需要增加next与pre pointer内容
*	@param1: 要更新值的blockpointer
*	@param2: 新的block的id
*	@param3: 新的block是什么类型
*	@param4: 新的block的file名
*/
void BufferManager::blockSetValue(Block* btmp, int _id, int _BlockType, string _fileName)
{
	btmp->id = _id;
	myint tmpid;
	tmpid.num = btmp->id;
	memcpy(btmp->content, tmpid.byte, sizeof(int));		//将block的id写到content中
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
*	将位置在writeBackBlock的block写回到disk中
*	@param1: 这个block对应的table名
*	@param2: 这个block
*/
int BufferManager::WriteBackToDisk(string fileName, int writeBackBlock, int blockType)
{
	myint tmpint1;
	myint tmpint2;
	Block* btmp = NULL;		//指向待写回的block
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
		string file = getFileName(fileName);		//获取file名称
		int fileHeadSize;
		tmpint1.num = btmp->using_Size;
		char* address = NULL;		//更新usingsize
		address = btmp->content + 4;
		memcpy(address, tmpint1.byte, 4);
		char* tmpContent = new char[4096];
		memcpy(tmpContent, btmp->content, 4096);		//直接将block中的内容写在file最后即可
		fstream ifs(file, ios::in | ios::binary);
		char* tmp = new char[5];
		ifs.read(tmp, sizeof(int));       		//读取file文件头大小
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
		char* address = NULL;		//更新usingsize
		address = btmp->content + 4;
		memcpy(address, tmpint1.byte, 4);
		char* tmpContent = new char[4096];
		memcpy(tmpContent, btmp->content, 4096);		//直接将block中的内容写在file最后即可
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
*	将file中对应的block写入到buffer中
*	@param1: block对应的table名
*	@param2: file中的blockid
*	return: 返回写回的block
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
	fstream ifs(file, ios::binary | ios::in);		//读取file头的大小
	ifs.read(buffer, 4);
	memcpy(tmpint.byte, buffer, sizeof(int));		//将file头的大小转化为十进制
	headSize = tmpint.num;
	int usingSize;
	char* content = new char[4096];
	memset(content, '\0', 4096);
	ifs.seekg(headSize + 4096 * blockID, ios::beg);		//将指针指向要读取的blockID的位置
	ifs.read(content, 4096);			//将file里边的content写到这个content数组中
	address = content + sizeof(int);		//address指向usingSize
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
*	删除block中的所有属于这个table的信息
*	@param1: 表名
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
*	删除对应表的所有记录
*	@param1: 表名
*/
void BufferManager::DeleteAllRecord(string tableName)
{
	for (int i = 0; i < MaxBlock; i++)
	{
		if (blocks[i].fileName == tableName)		//如果找到了表对应的block就直接删除这个block中的所有内容即可
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