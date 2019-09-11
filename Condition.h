#ifndef __Condition_H__
#define __Condition_H__

#include <iostream>
#include <string>
#include<sstream>

using namespace std;

class Condition
{
public:
	const static int OPERATOR_EQUAL = 0;	// "="
	const static int OPERATOR_NOT_EQUAL = 1;	//"!="
	const static int OPERATOR_MORE = 2;		// ">"
	const static int OPERATOR_LESS = 3;		// "<"
	const static int OPERATOR_MORE_EQUAL = 4;	// ">="
	const static int OPERATOR_LESS_EQUAL = 5;	// "<="

	string attriName;
	string value;
	int condition;

	Condition(string _attriName, string _value, int _condition);

	bool checkInt(int content);
	bool checkFloat(float content);
	bool checkString(string content);
};


#endif // !__Condition_H__

