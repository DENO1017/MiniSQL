#include<iostream>
#include<string>
#include<stdio.h>
#include<stdlib.h>
#include<vector>
#include"API.h"
#include"CatalogManager.h"
#include"RecordManager.h"
#include"IndexManager.h"
#pragma warning(disable:4996)

using namespace std;


typedef struct attribute *PtrToAttribute;
struct attribute
{
	string name;
	int type;
	int ifPrimary = 0;
	int ifUnique = 0;
};


string getWord(string str, int* index);
int Interpter(API ap, string str);
vector<attribute> attri;
BufferManager bm;
CatalogManager cm;
RecordManager rm;
IndexManager im;


int main()
{
	API ap;
	ap.cm = &cm;
	ap.rm = &rm;
	ap.InitProgram();


	string str1 = "create table Book (BookNo int unique, BookName char(100), Price float, primary key (BookNo))";
	Interpter(ap, str1);
	vector<Attribute> tmpattri;
	tmpattri = cm.tableList[0].attri;
	string str2 = "insert into Book values( 1, 'DS', 11.5)";
	string str3 = "insert into Book values( 2, 'ADS', 11.5)";
	string str4 = "insert into Book values( 3, 'math', 100.23)";
	string str5 = "select * from Book;";
	string str6 = "drop table Book;";
	string str7 = "delete from Book;";
	string str8 = "delete from Book where BookNo = 2;";
	string str9 = "insert	into	Book	values	( 4	, 'a',	11.5)";
	string str10 = "insert	into	Book	values	( 	5	, 'a',12.5	)";
	string str11 = "insert	into	Book	values(6 , 'a',11.1 )";
	string str12 = "insert	into	Book	values(7, 'a' ,11.2 )";
	string str14 = "insert	into	Book	values(8, 'a', 11.3 )";
	string str15 = "insert	into	Book	values(9, 'f', 10.4)";
	string str16 = "insert	into	Book	values(10,'g', 10.5 )";
	string str17 = "insert	into	Book	values(11, 'h', 10.6)";
	string str18 = "insert	into	Book	values(12, 'i',10.7 )";
	string str19 = "insert	into	Book	values(13, 'j',10.8 )";
	string str20 = "insert	into	Book	values(14,'k', 10.9)";
	string str21 = "insert	into	Book	values(15,'l', 10.2)";
	string str22 = "insert	into	Book	values(16, 'm', 10.3)";
	string str23 = "insert	into	Book	values(17, 'n', 10.1)";
	string str24 = "create index BookNoIndex on Book (BookNo);";
	string str25 = "create index BookNameIndex on Book (BookName);";
	string str26 = "drop index BookNoIndex;";
	string str28 = "drop index BookNameIndex;";
	string str29 = "delete from Book where BookName = 'a';";
	string str30 = "drop table Book;";
	string str27 = "quit;";
	string str31 = "select * from Book where BookNo = 2;";
	Interpter(ap, str2);//block0
	Interpter(ap, str3);//block1
	Interpter(ap, str4);//block2
	Interpter(ap, str9);//writedisk no writebuffer
	Interpter(ap, str10);
	Interpter(ap, str11);
	Interpter(ap, str12);

	Interpter(ap, str25);
	Interpter(ap, str24);

	Interpter(ap, str14);
	Interpter(ap, str15);

	Interpter(ap, str16);
	Interpter(ap, str17);
	////Interpter(ap, str18);
	////Interpter(ap, str19);
	////Interpter(ap, str20);
	////Interpter(ap, str21);
	////Interpter(ap, str22);
	cout << "--------------" << endl;
	Interpter(ap, str5);
	Interpter(ap, str31);

	Interpter(ap, str8);//////////////////////////
	cout << "-----------------------" << endl;
	Interpter(ap, str29);
	Interpter(ap, str5);
	Interpter(ap, str26);
	Interpter(ap, str28);

	////Interpter(ap, str7);
	//Interpter(ap, str5);
	//Interpter(ap, str7);
	//Interpter(ap, str6);
	Interpter(ap, str31);
	Interpter(ap, str5);
	Interpter(ap, str30);
	//Interpter(ap, str27);


	system("pause");
}

int Interpter(API ap, string str)
{
	int posi = 0;
	string word = "";
	word = getWord(str, &posi);
	if (word == "create")//create 语句
	{
		word = getWord(str, &posi);
		//关于建表仅仅写了关于表的attribute和primarykey的建立，不能识别其他语句
		if (word == "table")//create table语句
		{
			int primary_Posi;
			word = getWord(str, &posi);//获得表名
			string primary_Key = "";
			string table_Name = "";
			if (word.empty() == 0)
			{
				table_Name = word;
			}
			else
			{
				cout << "Syntax Error for no table name" << endl;
				return 0;
			}
			word = getWord(str, &posi);
			if (word != "(")
			{
				cout << "Syntax Error for no attributes" << endl;
				return 0;
			}
			else//开始读取表内attribute名
			{
				//vector<PtrToAttribute>Attribute;
				string primaryKey = "";
				word = getWord(str, &posi);
				while (word != "primary"&&word != ")"&&word != "")
				{
					struct attribute tempAttribute;
					
					if (!word.empty())
					{
						tempAttribute.name = word;
					}
					else
					{
						cout << "Syntax Error for no getting the attribute name" << endl;
						return 0;
					}
					word = getWord(str, &posi);
					if (word == "int")
					{
						tempAttribute.type = INT;
					}
					else if (word == "float")
					{
						tempAttribute.type = FLOAT;
					}
					else if (word == "char")
					{
						word = getWord(str, &posi);
						if (word == "(")
						{
							word = getWord(str, &posi);
							if (!word.empty())
							{
								int charsize = stoi(word);
								tempAttribute.type = charsize;
								word = getWord(str, &posi);
								if (word != ")")
								{
									cout << "Syntax Error for wrong format" << endl;
									return 0;
								}
							}
							else
							{
								cout << "Syntax Error for failed to get char size" << endl;
								return 0;
							}
						}
						else
						{
							cout << "Syntax Error for wrong start format" << endl;
							return 0;
						}
					}
					else
					{
						cout << "Syntax Error for wrong type" << endl;
						return 0;
					}
					
					word = getWord(str, &posi);
					if (word == "," || word == ")")
					{
						//cout << tempAttribute.name << endl;
						attri.push_back(tempAttribute);
						if (word == ")")
						{
							break;
						}
						else
						{
							word = getWord(str, &posi);
							continue;
						}
					}
					else
					{
						if (word == "unique")
						{
							tempAttribute.ifUnique = 1;
							word = getWord(str, &posi);
							if (word == "," || word == ")")
							{
								attri.push_back(tempAttribute);
								if (word == ")")
								{
									break;
								}
								else
								{
									word = getWord(str, &posi);
									continue;
								}
							}
						}
						else
						{
							cout << "Syntax Error for not in correct format" << endl;
							return 0;
						}

					}
					
				}
				//attribute读取完毕，可能遇到了primary key或直接结束
				if (word == "primary")
				{
					word = getWord(str, &posi);
					if (word == "key")
					{
						word = getWord(str, &posi);
						if (word == "(")
						{
							word = getWord(str, &posi);
							if (!word.empty())
							{
								primaryKey = word;
								int i;

								for (i = 0; i < (int)attri.size(); i++)
								{
									if (attri[i].name == primaryKey)
									{
										attri[i].ifPrimary = 1;
										primary_Posi = i;
										break;
									}
								}
								if (i == attri.size())
								{
									cout << "Syntax Error for wrong primary key name" << endl;
									return 0;
								}
								word = getWord(str, &posi);
								if (word != ")")
								{
									cout << "Syntax Error for wrong format" << endl;
									return 0;
								}
							}
							else
							{
								cout << "Syntax Error for accepting word" << endl;
								return 0;
							}
						}
						else
						{
							cout << "Syntax Error for ')' missing" << endl;
							return 0;
						}
					}
					else
					{
						cout << "Syntax Error for didn't get right key word" << endl;
						return 0;
					}
				}
			}

			vector<Attribute> a;
			//CatalogManager catalog;

			for (int i = 0; i < attri.size(); i++)
			{
				if (attri[i].ifUnique == 1)
				{
					Attribute newAttri(attri[i].name, attri[i].type, true);
					a.push_back(newAttri);
				}
				else
				{
					Attribute newAttri(attri[i].name, attri[i].type, false);
					a.push_back(newAttri);
				}
			}
			Table newTable(table_Name, a, primary_Posi);
			ap.CreateTable(newTable);
			//catalog.CreateTable(newTable);

			//调用建表函数建立表格，可以传入表名、表的attribute向量，主键名称，可根据需要对以上内容进行修改
			return 1;
		}
		else if (word == "index")
		{
			char*  indexName = new char[100];
			char*  tableName = new char[100];
			char*  attriName = new char[100];
			word = getWord(str, &posi);
			if (word != "")
			{
				strcpy(indexName, word.c_str());
				word = getWord(str, &posi);
				if (word == "on")
				{
					word = getWord(str, &posi);
					if (word != "")
					{
						strcpy(tableName, word.c_str());
						word = getWord(str, &posi);
						if (word == "(")
						{
							word = getWord(str, &posi);
							if (word != "")
							{
								strcpy(attriName, word.c_str());
								word = getWord(str, &posi);
								if (word == ")")
								{
									word = getWord(str, &posi);
									if (word == ";")
									{
										ap.CreateIndex(indexName, tableName, attriName);
									}
									else
									{
										cout << "Syntax Error: wrong format when create index" << endl;
										return 0;
									}
								}
								else
								{
									cout << "Syntax Error: wrong format when create index" << endl;
									return 0;
								}
							}
							else
							{
								cout << "Syntax Error: failed to get attri name" << endl;
								return 0;
							}
						}
						else
						{
							cout << "Syntax Error: wrong format when create table" << endl;
							return 0;
						}
					}
					else
					{
						cout << "Syntax Error: failed to get table name when create index" << endl;
						return 0;
					}
				}
				else
				{
					cout << "Syntax Error: wrong format losing on when create index" << endl;
					return 0;
				}
			}
			else
			{
				cout << "Syntax Error: Failed to get index name" << endl;
				return 0;
			}
		}
		else
		{
			cout << "Syntax Error for wrong create order";
			return 0;
		}
	}
	//insert into Book values('001', 'DS', '15');
	else if (word == "insert")
	{
		string tableName = "";
		vector<string>recordContent;
		word = getWord(str, &posi);
		if (word == "into")
		{
			word = getWord(str, &posi);
			if (word != "")
			{
				tableName = word;
				word = getWord(str, &posi);
				if (word == "values")
				{
					word = getWord(str, &posi);
					if (word == "(")
					{
						word = getWord(str, &posi);
						while (word != "" && word != ")")
						{
							recordContent.push_back(word);
							word = getWord(str, &posi);
							if (word == ",")
							{
								word = getWord(str, &posi);
								continue;
							}
							else if (word == ")")
							{
								continue;
							}
							else
							{
								cout << "Syntax Error: insert sentence wrong format when getting values" << endl;
								return 0;
							}
						}
					}
					else
					{
						cout << "Syntax Error: insert sentence wrong format when getting values" << endl;
						return 0;
					}
				}
				else
				{
					cout << "Syntax Error: insert sentence wrong format after table name" << endl;
					return 0;
				}
			}
			else
			{
				cout << "Syntax Error: fail to get table name when inserting" << endl;
				return 0;
			}
		}
		else
		{
			cout << "Syntax Error: insert sentence wrnog format" << endl;
			return 0;
		}
		ap.InsertRecord(tableName, recordContent);
		return 1;
	}
	//select * from Book
	//select * from Book where BookNo = 'J0001'
	//select BookName from Book where BookNo = 'J0001'
	else if (word == "select")//选择操作
	{
		vector<string>selectAttribute;
		vector<Condition> conditionVec;
		string selectTableName = "";
		word = getWord(str, &posi);
		if (word == "*")
		{
			word = getWord(str, &posi);
			if (word == "from")
			{
				word = getWord(str, &posi);
				if (!word.empty())
				{
					selectTableName = word;
					word = getWord(str, &posi);
					if (word == "where")
					{
						string attriName = "";
						string value = "";
						int operation;
						while (word != ";")
						{
							word = getWord(str, &posi);		//列名
							attriName = word;
							word = getWord(str, &posi);		//conditoin
							if (word == "=")
							{
								operation = Condition::OPERATOR_EQUAL;
							}
							else if (word == "<>")
							{
								operation = Condition::OPERATOR_NOT_EQUAL;
							}
							else if (word == ">")
							{
								operation = Condition::OPERATOR_MORE;
							}
							else if (word == "<")
							{
								operation = Condition::OPERATOR_LESS;
							}
							else if (word == ">=")
							{
								operation = Condition::OPERATOR_MORE_EQUAL;
							}
							else if (word == "<=")
							{
								operation = Condition::OPERATOR_LESS_EQUAL;
							}
							else
							{
								cout << "Syntax Error: wrong condition input" << endl;
								return 0;
							}
							word = getWord(str, &posi);
							if (word != "")
							{
								value = word;
							}
							else
							{
								cout << "Syntax Error: wrong condition input" << endl;
								return 0;
							}
							Condition tmpCondition(attriName, value, operation);
							conditionVec.push_back(tmpCondition);
							word = getWord(str, &posi);
							if (word != "and" &&word != ";")
							{
								cout << "Syntax Error: wrong select condition format" << endl;
								return 0;
							}

						}
						ap.RecordAllShow(selectTableName, conditionVec);
					}
					else
					{
						ap.RecordAllShow(selectTableName, conditionVec);
					}
				}
			}
			else
			{
				cout << "Sytnax Error for no attribute name" << endl;
				return 0;
			}
		}
		else
		{
			cout << "Sytnax Error: wrong select format" << endl;
			return 0;
		}
		return 1;
	}
	else if (word == "drop")
	{
		string dropTableName = "";
		word = getWord(str, &posi);
		if (word == "table")
		{
			word = getWord(str, &posi);
			if (word != "")
			{
				dropTableName = word;
				ap.tableDrop(dropTableName);
				word = getWord(str, &posi);
				if (word == "")
				{
					cout << "Syntax Error: wrong drop table format" << endl;
					return 0;
				}
				return 1;
			}
			else
			{
				cout << "Syntax Error: drop table forget name" << endl;
				return 0;
			}
		}
		else if (word == "index")
		{
			string dropIndexName = "";
			word = getWord(str, &posi);
			if (word != "")
			{
				dropIndexName = word;
				ap.IndexDrop(dropIndexName);
				word = getWord(str, &posi);
				if (word == "")
				{
					cout << "Syntax Error: wrong drop index format" << endl;
					return 0;
				}
				return 1;
			}
			else
			{
				cout << "Syntax Error: failed to get index name when drop index" << endl;
				return 0;
			}
		}
		else
		{
			cout << "Syntax Error: drop sentence wrong format" << endl;
			return 0;
		}
	}
	else if (word == "delete")
	{
		string deleteTableName = "";
		string attriName = "";
		int condition = 0;
		string value = "";
		
		word = getWord(str, &posi);
		if (word == "from")
		{
			word = getWord(str, &posi);
			if (word != "")
			{
				deleteTableName = word;
				word = getWord(str, &posi);
				if (word != ";")
				{
					if (word == "where")
					{
						word = getWord(str, &posi);			//写入attribute名
						if (word != "")
						{
							attriName = word;
							word = getWord(str, &posi);		//写入condition
							if (word != "")
							{
								if (word == "=")
								{
									condition = Condition::OPERATOR_EQUAL;
								}
								else if (word == "<>")
								{
									condition = Condition::OPERATOR_NOT_EQUAL;
								}
								else if (word == ">")
								{
									condition = Condition::OPERATOR_MORE;
								}
								else if (word == "<")
								{
									condition = Condition::OPERATOR_LESS;
								}
								else if (word == ">=")
								{
									condition = Condition::OPERATOR_MORE_EQUAL;
								}
								else if (word == "<=")
								{
									condition = Condition::OPERATOR_LESS_EQUAL;
								}
								else
								{
									cout<< "Syntax Error: delete sentence condition" << endl;
									return 0;
								}
								word = getWord(str, &posi);
								if (word != "")
								{
									value = word;
									Condition conditionValue(attriName, value, condition);
									ap.DeleteConditionRecord(deleteTableName, conditionValue);
								}
								else
								{
									cout << "Syntax Error: delete sentence condition lost value" << endl;
									return 0;
								}
							}
							else
							{
								cout << "Syntax Error: delete sentence condition lost operation" << endl;
								return 0;
							}
						}
						else
						{
							cout << "Syntax Error: delete sentence condition lost attribute name" << endl;
							return 0;
						}
					}
					else
					{
						cout << "Syntax Error: delete sentence wrong format of condition" << endl;
						return 0;
					}
				}
				else
				{
					ap.deleteAllRecord(deleteTableName);
				}
			}
			else
			{
				cout << "Syntax Error: delete sentence wrong format of table name" << endl;
				return 0;
			}
		}
		else
		{
			cout << "Syntax Error: delete sentence wrong format" << endl;
			return 0;
		}
	}
	else if (word == "quit")
	{
		ap.prepareToQuit();
		return 0;
	}
}


string getWord(string str, int * index)
{
	int posi1, posi2;
	string word;
	while ((str[*index] == ' ' || str[*index] == '\n' || str[*index] == '\t')&&str[*index]!='\0')
	{
		(*index)++;
	}
	posi1 = *index;
	//处理括号、逗号、比较符号等有意义的符号内容
	if (str[posi1] == '(' || str[posi1] == ')' || str[posi1] == ',' || str[posi1] == '=' || str[posi1] == '<' || str[posi1] == '>'|| str[posi1] == ';')
	{
		(*index)++;
		if (str[*index] == '=' || str[*index] == '>')
		{
			(*index)++;
		}
		posi2 = *index;
		word = str.substr(posi1, posi2 - posi1);
		return word;
	}
	else if (str[posi1] == '\'')		//遇到单引号返回单引号内部内容
	{
		(*index)++;
		while (str[*index] == ' ' || str[*index] == '\n' || str[*index] == '\t')//不接受单引号内部的空格等无用符号
		{
			(*index)++;
		}
		posi1 = *index;
		while (str[*index] != '\'')
		{
			(*index)++;
		}
		posi2 = *index;
		(*index)++;
		word = str.substr(posi1, posi2 - posi1);
		return word;
	}
	else
	{
		while (str[*index] != ' '&&str[*index] != '\n'&&str[*index] != '\t'&& str[*index] != '>=' && str[*index] != '<=' && str[*index] != '<>' &&str[*index] != '<'&& str[*index] != '>' &&str[*index] != '='&&str[*index] != 39 && str[*index] != '('&&str[*index] != ')'&&str[*index] != ','&&str[*index] != ';')
		{
			(*index)++;
		}
		posi2 = *index;
		if (posi1 != posi2)
		{
			word = str.substr(posi1, posi2 - posi1);
		}
		else
		{
			word = "";
		}
		return word;
	}
}