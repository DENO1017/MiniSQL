#include"Condition.h"

Condition::Condition(string _attriName, string _value, int _condition)
{
	attriName = _attriName;
	value = _value;
	condition = _condition;
}

bool Condition::checkInt(int content)
{
	stringstream ss;
	ss << value;
	int thscontent;
	ss >> thscontent;

	if (this->condition == OPERATOR_EQUAL)
	{
		return content == thscontent;
	}
	else if (this->condition == OPERATOR_NOT_EQUAL)
	{
		return content != thscontent;
	}
	else if (this->condition == OPERATOR_MORE)
	{
		return content > thscontent;
	}
	else if (this->condition == OPERATOR_LESS)
	{
		return content < thscontent;
	}
	else if (this->condition == OPERATOR_MORE_EQUAL)
	{
		return content >= thscontent;
	}
	else if (this->condition == OPERATOR_LESS_EQUAL)
	{
		return content <= thscontent;
	}
}

bool Condition::checkFloat(float content)
{
	stringstream ss;
	ss << value;
	float thscontent;
	ss >> thscontent;


	if (this->condition == OPERATOR_EQUAL)
	{
		return content == thscontent;
	}
	else if (this->condition == OPERATOR_NOT_EQUAL)
	{
		return content != thscontent;
	}
	else if (this->condition == OPERATOR_MORE)
	{
		return content > thscontent;
	}
	else if (this->condition == OPERATOR_LESS)
	{
		return content < thscontent;
	}
	else if (this->condition == OPERATOR_MORE_EQUAL)
	{
		return content >= thscontent;
	}
	else if (this->condition == OPERATOR_LESS_EQUAL)
	{
		return content <= thscontent;
	}
}

bool Condition::checkString(string content)
{
	stringstream ss;
	ss << value;
	string thscontent;
	ss >> thscontent;

	if (this->condition == OPERATOR_EQUAL)
	{
		return content == value;
	}
	else if (this->condition == OPERATOR_NOT_EQUAL)
	{
		return content != value;
	}
	else if (this->condition == OPERATOR_MORE)
	{
		return content > value;
	}
	else if (this->condition == OPERATOR_LESS)
	{
		return content < value;
	}
	else if (this->condition == OPERATOR_MORE_EQUAL)
	{
		return content >= value;
	}
	else if (this->condition == OPERATOR_LESS_EQUAL)
	{
		return content <= value;
	}
}