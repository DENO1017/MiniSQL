//
//  BPlusTree.h
//  Summer_Pro_Minisql
//  Description: The basic b+ tree implement of template.
//
//  Author:Su Man.
//  Date:18/6/6.
//

#ifndef __BPlusTree__
#define __BPlusTree__

#include <iostream>
#include <vector>
#include <set>
#include<string>
#include "BufferManager.h"
#include "Minisql.h"
using namespace std;

extern BufferManager bm;
typedef int offsetNumber;

template <typename KeyType>
class TreeNode
{
private:
public:
	int count;
	TreeNode* parent;
	TreeNode* next;
	vector <TreeNode*> child;
	vector <KeyType> value;
	vector <offsetNumber> blockId;
	bool isLeaf;
	TreeNode();
	TreeNode(int degree, bool i);
	bool search(KeyType key, int &index);
	int add(KeyType key, offsetNumber b);
	int delet(int index);
	TreeNode<KeyType>* splite(KeyType &key);
};

template <typename KeyType>
class BPlusTree
{
private:
	int degree;

	struct SearchResult
	{
		TreeNode<KeyType> *pNode;
		int index;
		bool isFound;
	};

public:
	TreeNode<KeyType>* HeadLeaf;
	TreeNode<KeyType>* root;
	BPlusTree(int d);
	~BPlusTree() { cout << "~BPlusTree()" << endl; }
	void dropTree(TreeNode<KeyType>* t);
	void init();
	bool insertKey(KeyType key, offsetNumber b);
	bool AdjustAfterInsert(TreeNode<KeyType> *t);
	bool deleteKey(KeyType key);
	bool adjustAfterDelete(TreeNode<KeyType> *t);
	set<offsetNumber> searchKey(KeyType key, int op);
	void searchLeaf(TreeNode<KeyType> *t, KeyType key, SearchResult &rst);

	//void readFromDiskAll(String fileName);
	//void writtenbackToDiskAll(String fileName);
	//void readFromDisk(Block* btmp);
};

//TreeNode

template <typename KeyType>
TreeNode<KeyType>::TreeNode()
{
	count = 0;
	parent = NULL;
	next = NULL;
	for (int i = 0; i < 20 + 1; i++)
	{
		child.push_back(NULL);
		value.push_back(int());
		blockId.push_back(offsetNumber());
	}
	isLeaf = true;
	child.push_back(NULL);
}

template <typename KeyType>
TreeNode<KeyType>::TreeNode(int degree, bool i)
{
	count = 0;
	parent = NULL;
	next = NULL;
	isLeaf = i;
	for (int i = 0; i < degree + 1; i++)
	{
		child.push_back(NULL);
		value.push_back(KeyType());
		blockId.push_back(offsetNumber());
	}
	child.push_back(NULL);
}

template <typename KeyType>
bool TreeNode<KeyType>::search(KeyType key, int &index)
{
	int i;
	if (this->count == 0)
	{
		index = 0;
		return false;
	}
	if (value[count - 1] < key)
	{
		index = count;
		return false;
	}
	if (value[0]>key)
	{
		index = 0;
		return false;
	}
	for (i = 0; i < count; i++)
	{
		if (value[i] == key)
		{
			index = i;
			return true;
		}
		else if (value[i] > key)
		{
			index = i;
			return false;
		}
	}
	return false;
}

template <typename KeyType>
int TreeNode<KeyType>::add(KeyType key, offsetNumber b)
{
	if (count == 0)
	{
		value[0] = key;
		count++;
		return 0;
	}
	else
	{
		int i = 0;
		if (search(key, i))
		{
			return -1;
		}
		else
		{
			for (int j = count; j > i; j--)
			{
				value[j] = value[j - 1];
				if (this->isLeaf)
					blockId[j] = blockId[j - 1];
			}
			value[i] = key;
			if (this->isLeaf)
				blockId[i] = b;
			count++;
			return i;
		}
	}
}

template <typename KeyType>
int TreeNode<KeyType>::delet(int index)
{
	for (int j = count - 1; j > index; j--)
	{
		value[j - 1] = value[j];
		if (this->isLeaf)
			blockId[j - 1] = blockId[j];
	}
	value[count - 1] = KeyType();
	if (this->isLeaf)
		blockId[count - 1] = offsetNumber();
	count--;
	return 1;
}

template <typename KeyType>
TreeNode<KeyType>* TreeNode<KeyType>::splite(KeyType &key)
{
	int degree = this->count;
	int minnum = degree / 2;
	TreeNode<KeyType>* nNode;
	nNode = new TreeNode<KeyType>(this->count, this->isLeaf);
	key = this->value[degree / 2];
	if (!this->isLeaf)
	{
		for (int i = 0; i < (degree - 1) / 2; i++)
		{
			nNode->value[i] = this->value[i + degree / 2 + 1];
			nNode->child[i] = this->child[i + degree / 2 + 1];
			nNode->child[i]->parent = nNode;
			this->value[i + degree / 2 + 1] = KeyType();
			this->child[i + degree / 2 + 1] = NULL;
		}
		nNode->child[(degree - 1) / 2] = this->child[degree];
		nNode->child[(degree - 1) / 2]->parent = nNode;
		nNode->count = (degree - 1) / 2;
		this->count = degree / 2;
	}
	else
	{
		for (int i = 0; i < degree-minnum; i++)
		{
			nNode->value[i] = this->value[i + minnum];
			nNode->blockId[i] = this->blockId[i + minnum];
			this->value[i + minnum] = KeyType();
			this->blockId[i + minnum] = offsetNumber();
		}
		nNode->count = degree - minnum;
		this->count = minnum;
	}

	nNode->parent = this->parent;
	nNode->next = this->next;
	this->next = nNode;
	return nNode;
}

//BPlusTree

template <typename KeyType>
BPlusTree<KeyType>::BPlusTree(int d)
{
	degree = d;
	root = new TreeNode < KeyType >(degree, true);
	HeadLeaf = root;
}

template <class KeyType>
void BPlusTree<KeyType>::dropTree(TreeNode<KeyType>* t)
{
	if (t==NULL) return;
	if (!t->isLeaf) //if it has child
	{
		for (size_t i = 0; i <= t->count; i++)
		{
			//cout << "t->value[0]" << t->value[0] << endl;
			dropTree(t->child[i]);
			t->child[i] = NULL;
		}
	}
	delete t;
	return;
}

template <class KeyType>
void BPlusTree<KeyType>::init()
{
	dropTree(root);
	root = new TreeNode<KeyType>(degree, true);
	HeadLeaf = root;
}

template <typename KeyType>
bool BPlusTree<KeyType>::insertKey(KeyType key, offsetNumber b)
{
	SearchResult rst;
	searchLeaf(root, key, rst);
	if (rst.isFound)
	{
		return false;
	}
	else
	{
		rst.pNode->add(key, b);
		if (rst.pNode->count == degree)
		{
			AdjustAfterInsert(rst.pNode);
		}
	}
	return true;
}

template <typename KeyType>
bool BPlusTree<KeyType>::AdjustAfterInsert(TreeNode<KeyType> *t)
{
	KeyType key;
	TreeNode<KeyType>* nNode = t->splite(key);
	if (nNode->parent == NULL)
	{
		TreeNode<KeyType>* r = new TreeNode<KeyType>(degree, false);
		this->root = r;
		nNode->parent = root;
		t->parent = root;
	}
	int ind = nNode->parent->add(key,0);
	nNode->parent->child[ind] = t;
	nNode->parent->child[ind + 1] = nNode;

	if (nNode->parent->count == degree)
	{
		AdjustAfterInsert(nNode->parent);
	}

	return true;
}

template <class KeyType>
bool BPlusTree<KeyType>::deleteKey(KeyType key)
{
	SearchResult rst;
	if (!root)
	{
		return false;
	}
	else
	{
		searchLeaf(root, key, rst);
		if (!rst.isFound)
		{
			return false;
		}
		else
		{
			if (rst.pNode->parent == NULL)
			{
				rst.pNode->delet(rst.index);
			}
			else
			{
				if (rst.index == 0 && HeadLeaf != rst.pNode)
				{
					int index = 0;

					TreeNode<KeyType>* now_parent = rst.pNode->parent;
					bool if_found_inBranch = now_parent->search(key, index);
					while (!if_found_inBranch)
					{
						if (now_parent->parent)
							now_parent = now_parent->parent;
						else
						{
							break;
						}
						if_found_inBranch = now_parent->search(key, index);
					}

					now_parent->value[index] = rst.pNode->value[1];

					rst.pNode->delet(rst.index);

				}
				else
				{
					rst.pNode->delet(rst.index);
				}
			}
			return adjustAfterDelete(rst.pNode);
		}
	}
}

template <class KeyType>
bool BPlusTree<KeyType>::adjustAfterDelete(TreeNode<KeyType> *pNode)
{
	int minmumKey = (degree - 1) / 2;
	if (((pNode->isLeaf) && (pNode->count >= minmumKey)) || ((degree != 3) && (!pNode->isLeaf) && (pNode->count >= minmumKey - 1)) || ((degree == 3) && (!pNode->isLeaf) && (pNode->count < 0))) // do not need to adjust
	{
		return  true;
	}
	if (pNode->parent == NULL)
	{
		if (pNode->count > 0) //do not need to adjust
		{
			return true;
		}
		else
		{
			if (root->isLeaf) //the true will be an empty tree
			{
				return true;
			}
			else // root will be the leafhead
			{
				root = pNode->child[0];
				root->parent = NULL;
				delete pNode;
			}
		}
	}// end root
	else
	{
		TreeNode<KeyType>* parent = pNode->parent;
		TreeNode<KeyType>* brother = NULL;
		if (pNode->isLeaf)
		{
			int index = 0;
			parent->search(pNode->value[0], index);

			if ((parent->child[0] != pNode) && (index + 1 == parent->count)) //choose the left brother to merge or replace
			{
				brother = parent->child[index];
				if (brother->count > minmumKey) // choose the most right key of brother to add to the left hand of the pnode
				{
					for (int i = pNode->count; i > 0; i--)
					{
						pNode->value[i] = pNode->value[i - 1];
						pNode->blockId[i] = pNode->blockId[i - 1];
					}
					pNode->value[0] = brother->value[brother->count - 1];
					pNode->blockId[0] = brother->blockId[brother->count - 1];
					brother->delet(brother->count - 1);

					pNode->count++;
					parent->value[index] = pNode->value[0];
					return true;

				} // end add
				else // merge the node with its brother
				{
					parent->delet(index);

					for (int i = 0; i < pNode->count; i++)
					{
						brother->value[i + brother->count] = pNode->value[i];
						brother->blockId[i + brother->count] = pNode->blockId[i];
					}
					brother->count += pNode->count;
					brother->next = pNode->next;

					delete pNode;

					return adjustAfterDelete(parent);
				}// end merge

			}// end of the left brother
			else // choose the right brother
			{
				if (parent->child[0] == pNode)
					brother = parent->child[1];
				else
					brother = parent->child[index + 2];
				if (brother->count > minmumKey)//// choose the most left key of brother to add to the right hand of the node
				{
					pNode->value[pNode->count] = brother->value[0];
					pNode->blockId[pNode->count] = brother->blockId[0];
					pNode->count++;
					brother->delet(0);
					if (parent->child[0] == pNode)
						parent->value[0] = brother->value[0];
					else
						parent->value[index + 1] = brother->value[0];
					return true;

				}// end add
				else // merge the node with its brother
				{
					for (int i = 0; i < brother->count; i++)
					{
						pNode->value[pNode->count + i] = brother->value[i];
						pNode->blockId[pNode->count + i] = brother->blockId[i];
					}
					if (pNode == parent->child[0])
						parent->delet(0);
					else
						parent->delet(index + 1);
					pNode->count += brother->count;
					pNode->next = brother->next;
					delete brother;

					return adjustAfterDelete(parent);
				}//end merge
			}// end of the right brother

		}// end leaf
		else // branch node
		{
			int index = 0;
			parent->search(pNode->child[0]->value[0], index);
			if ((parent->child[0] != pNode) && (index + 1 == parent->count)) // choose the left brother to merge or replace
			{
				brother = parent->child[index];
				if (brother->count > minmumKey - 1) // choose the most right key and child to add to the left hand of the pnode
				{
					//modify the pnode
					pNode->child[pNode->count + 1] = pNode->child[pNode->count];
					for (size_t i = pNode->count; i > 0; i--)
					{
						pNode->child[i] = pNode->child[i - 1];
						pNode->value[i] = pNode->value[i - 1];
					}
					pNode->child[0] = brother->child[brother->count];
					pNode->value[0] = parent->value[index];
					pNode->count++;
					//modify the father
					parent->value[index] = brother->value[brother->count - 1];
					//modify the brother and child
					if (brother->child[brother->count])
					{
						brother->child[brother->count]->parent = pNode;
					}
					brother->delet(brother->count - 1);

					return true;

				}// end add
				else // merge the node with its brother
				{
					//modify the brother and child
					brother->value[brother->count] = parent->value[index];
					parent->delet(index);
					brother->count++;

					for (int i = 0; i < pNode->count; i++)
					{
						brother->child[brother->count + i] = pNode->child[i];
						brother->value[brother->count + i] = pNode->value[i];
						brother->child[brother->count + i]->parent = brother;
					}
					brother->child[brother->count + pNode->count] = pNode->child[pNode->count];
					brother->child[brother->count + pNode->count]->parent = brother;

					brother->count += pNode->count;


					delete pNode;

					return adjustAfterDelete(parent);
				}

			}// end of the left brother
			else // choose the right brother
			{
				if (parent->child[0] == pNode)
					brother = parent->child[1];
				else
					brother = parent->child[index + 2];
				if (brother->count > minmumKey - 1)// choose the most left key and child to add to the right hand of the pnode
				{
					//modifty the pnode and child
					pNode->child[pNode->count + 1] = brother->child[0];
					pNode->value[pNode->count] = brother->value[0];
					pNode->child[pNode->count + 1]->parent = pNode;
					pNode->count++;
					//modify the fater
					if (pNode == parent->child[0])
						parent->value[0] = brother->value[0];
					else
						parent->value[index + 1] = brother->value[0];
					//modify the brother
					brother->child[0] = brother->child[1];
					brother->delet(0);

					return true;
				}
				else // merge the node with its brother
				{
					//modify the pnode and child
					pNode->value[pNode->count] = parent->value[index];

					if (pNode == parent->child[0])
						parent->delet(0);
					else
						parent->delet(index + 1);

					pNode->count++;

					for (int i = 0; i < brother->count; i++)
					{
						pNode->child[pNode->count + i] = brother->child[i];
						pNode->value[pNode->count + i] = brother->value[i];
						pNode->child[pNode->count + i]->parent = pNode;
					}
					pNode->child[pNode->count + brother->count] = brother->child[brother->count];
					pNode->child[pNode->count + brother->count]->parent = pNode;

					pNode->count += brother->count;


					delete brother;

					return adjustAfterDelete(parent);

				}

			}

		}

	}
	return false;
}

template <typename KeyType>
set<offsetNumber> BPlusTree<KeyType>::searchKey(KeyType key, int op)
{
	set<offsetNumber> result;

	SearchResult rst;
	searchLeaf(root, key, rst);

	if (!rst.isFound)
	{
		TreeNode<KeyType>* temp = root;
		{
			while (temp != NULL)
			{
				temp = temp->child[0];
			}
		}
		return result;
	}
	else
	{
		TreeNode<KeyType> *temp;
		switch (op)
		{
		case 0:///////=
		{
			result.insert(rst.pNode->blockId[rst.index]);
			break;
		}
		case 1://////////!=
		{
			temp = HeadLeaf;
			while (temp != rst.pNode)
			{
				for (int i = 0; i < temp->count; i++)
				{
					result.insert(temp->blockId[i]);
				}
				temp = temp->next;
			}
			//temp = temp->next;
			while (temp != NULL)
			{
				for (int i = 0; i < temp->count; i++)
				{
					result.insert(temp->blockId[i]);
				}
				temp = temp->next;
			}
			break;
		}
		case 2:///////////////>
		{
			temp = rst.pNode;
			while (temp != NULL)
			{
				for (int i = 0; i < temp->count; i++)
				{
					result.insert(temp->blockId[i]);
				}
				temp = temp->next;
			}
			break;
		}
		case 3://///////////////<
		{
			temp = HeadLeaf;
			while (temp != rst.pNode)
			{
				for (int i = 0; i < temp->count; i++)
				{
					result.insert(temp->blockId[i]);
				}
				temp = temp->next;
			}
			break;
		}
		case 4:////////////>=
		{
			temp = rst.pNode;
			while (temp != NULL)
			{
				for (int i = 0; i < temp->count; i++)
				{
					result.insert(temp->blockId[i]);
				}
				temp = temp->next;
			}
			break;
		}
		case 5://////////////<=
		{
			temp = HeadLeaf;
			while (temp != rst.pNode->next)
			{
				for (int i = 0; i < temp->count; i++)
				{
					result.insert(temp->blockId[i]);
				}
				temp = temp->next;
			}
			break;
		}
		default:
			break;
		}
		return result;
	}
}

template <typename KeyType>
void BPlusTree<KeyType>::searchLeaf(TreeNode<KeyType> *t, KeyType key, SearchResult &rst)
{
	int index = 0;
	if (t->search(key, index))
	{
		if (t->isLeaf)
		{
			rst.pNode = t;
			rst.index = index;
			rst.isFound = true;
		}
		else
		{
			t = t->child[index + 1];
			while (!t->isLeaf)
			{
				t = t->child[0];
			}
			rst.pNode = t;
			rst.index = 0;
			rst.isFound = true;
		}
	}
	else
	{
		if (t->isLeaf)
		{
			rst.pNode = t;
			rst.index = index;
			rst.isFound = false;
		}
		else
		{
			searchLeaf(t->child[index], key, rst);
		}
	}
}


#endif