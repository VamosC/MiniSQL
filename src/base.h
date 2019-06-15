// 
//	base.h
//	the structure of table

#ifndef _BASE_H_
#define _BASE_H_ 1

#include <iostream>
#include <vector>
#include <string>
#include <cassert>
#include <iomanip>

const int _PAGESIZE = 4096;
const int _MAXFRAMESIZE = 100;

#define INT -1
#define FLOAT 0
//数据 
//一个数据的类型以及数据的存放
//sdata的长度为 1~255 
class Data{
public:
	int type;				//-1 - int; 0 - float; 正数 - string（varchar长度） 
	int idata;
	float fdata;
	std::string sdata;
	Data(){}
	~Data(){}
	bool operator==(const Data &data)
	{
		if(data.type != type)
		{
			return false;
		}
		if(type == INT)
		{
			return idata == data.idata;
		}
		else if(type == FLOAT)
		{
			return fdata == data.fdata;
		}
		else
		{
			return sdata == data.sdata;
		}
	}
	bool operator!=(const Data &data)
	{
		return !operator==(data);
	}
};
//用于where的判断
typedef enum{
    LESS,
    LESS_OR_EQUAL,
    EQUAL,
    GREATER_OR_EQUAL,
    GREATER,
    NOT_EQUAL
} WHERE;
//Where存放一组判断语句
struct Where {
	Data data; //数据
	WHERE relation_character;   //关系
};
// 单个属性的查找条件
// start <(<=) x <(<=) end
struct condition
{
	// -1:不存在 4:<= 2:<
	int l_op;
	Data start;
	// -1:不存在 4:<= 2:<
	int r_op;
	Data end;
};

//查找条件
//最多30个条件 
//operation here: 0- =	1- <>	2- <	3- >	4- <=	5- >=
struct SelectCondition{
	int amount;
	std::string attr[30];
	int operationtype[30];
	Data key[30];
};


//属性 
//数量、属性名、类型、是否唯一、是否存在索引、主码
//最多存放32个属性 
struct Attribute{
	int amount;
	std::string attr_name[32];
	int attr_type[32];		//1 - int; 0 - float; 2 - string
	bool is_unique[32];
	int primary_key;		//主码的序号 
};


//元组
class Tuple{
private:
	std::vector<Data> data;
	bool isDeleted_;
public:
	Tuple() : isDeleted_(false){};
	
	//拷贝函数 
	Tuple(const Tuple &copytuple)
	{
		data = copytuple.data;
	}

	Tuple(const std::vector<Data> &tuples)
	{
		for(auto it : tuples)
		{
			addData(it);
		}
	}
	bool operator==(const Tuple &t)
	{
		if(t.data.size() != data.size())
		{
			return false;
		}
		for(auto i = 0; i < data.size(); i++)
		{
			if(data[i] != t.data[i])
				return false;
		}
		return true;
	}
	
	//输入元组
	//初步设想是在insert操作时按顺序一个一个把data导进来 
	void addData(const Data &d)
	{
		this->data.push_back(d);
	}
	
	//打印一个元祖的数据
	//按顺序输出，中间的间隔还需调整 
	void Printdata(const std::vector<int>& width)
	{
		int i = 0;
		for(auto it = data.begin(); it != data.end(); it++, i++)
		{
			std::cout << "|";
			if( (*it).type == INT)
				std::cout << std::left << std::setw(width[i]) << (*it).idata;
			else if( (*it).type == FLOAT)
				std::cout << std::left << std::setw(width[i]) << (*it).fdata;
			else
				std::cout << std::left << std::setw(width[i]) << (*it).sdata;
		}
		std::cout << "|\n";
	}
	int getWidth(int i)
	{
		assert(i >= 0 && i < data.size());
		if(data[i].type == INT)
		{
			auto item = std::to_string(data[i].idata);
			return item.size();
		}
		else if(data[i].type == FLOAT)
		{
			auto item = std::to_string(data[i].fdata);
			return item.size();
		}
		else
		{
			assert(false);
		}
	}

	//返回数据
	std::vector<Data> getData() const
	{
		return this->data;
	}
	bool isDeleted() {
		return isDeleted_;
	}

	void setDeleted() {
		isDeleted_ = true;
	}
}; 


//索引
//一张表最多有10个索引 
struct Index{
	int amount;
	std::string name[10]; 
	int whose[10];			//索引及对应的属性 
}; 

//表格
class Table{
private:
	std::vector<Tuple> tuples;
	std::string table_name;
	struct Attribute attr;
public:
	Table(){};
	Table(std::string name, Attribute tmpa ): table_name(name), attr(tmpa){};
	void PrintTable()
	{
		std::vector<int> width;
		for(auto i = 0; i < attr.amount; i++)
		{
			if(attr.attr_type[i] == INT)
			{
				auto max = 0;
				for(auto j = 0; j < tuples.size(); j++)
				{
					max = tuples[j].getWidth(i) > max ? tuples[j].getWidth(i) : max;
				}
				max = max > attr.attr_name[i].size() ? max : attr.attr_name[i].size();
				width.push_back(max);
			}
			else if(attr.attr_type[i] == FLOAT)
			{
				auto max = 0;
				for(auto j = 0; j < tuples.size(); j++)
				{
					max = tuples[j].getWidth(i) > max ? tuples[j].getWidth(i) : max;
				}
				max = max > attr.attr_name[i].size() ? max : attr.attr_name[i].size();
				width.push_back(max);
			}
			else
			{
				auto max = attr.attr_name[i].size() > attr.attr_type[i] ? attr.attr_name[i].size() : attr.attr_type[i]; 
				width.push_back(max);
			}
		}

		// 打印表头
		for(auto i = 0; i < width.size(); i++)
		{
			std::cout << "+";
			for(auto j = 0; j < width[i]; j++)
			{
				std::cout << "-";
			}
		}
		std::cout << "+\n";
		for(auto i = 0; i < width.size(); i++)
		{
			std::cout << "|";
			std::cout << std::left << std::setw(width[i]) << attr.attr_name[i];
		}
		std::cout << "|\n";
		for(auto i = 0; i < width.size(); i++)
		{
			std::cout << "+";
			for(auto j = 0; j < width[i]; j++)
			{
				std::cout << "-";
			}
		}
		std::cout << "+\n";

		// 打印数据
		for(auto it : tuples)
		{
			it.Printdata(width);
		}

		// 打印结尾
		for(auto i = 0; i < width.size(); i++)
		{
			std::cout << "+";
			for(auto j = 0; j < width[i]; j++)
			{
				std::cout << "-";
			}
		}
		std::cout << "+\n";
	}
	
	std::string GetTableName()
	{
		return table_name;
	}
	Attribute GetAttr()
	{
		return attr;
	}
	std::vector<Tuple>& GetTuples()
	{
		return tuples;
	} 
	int size()
	{
		return tuples.size();
	}
};

#endif
