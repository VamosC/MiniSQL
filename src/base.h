// 
//	base.h
//	the structure of table

#ifndef _BASE_H_
#define _BASE_H_ 1

#include <iostream>
#include <vector>
#include <string>

const int _PAGESIZE = 4096;
const int _MAXFRAMESIZE = 100;

#define INT -1
#define FLOAT 0
//数据 
//一个数据的类型以及数据的存放
//sdata的长度为 1~255 
struct Data{
	int type;				//-1 - int; 0 - float; 正数 - string（varchar长度） 
	int idata;
	float fdata;
	std::string sdata;
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
	Tuple(){};
	
	//拷贝函数 
	Tuple(const Tuple &copytuple)
	{
		data = copytuple.data;
	}
	
	
	//输入元组
	//初步设想是在insert操作时按顺序一个一个把data导进来 
	void Inputdata( Data &td )
	{
		struct Data tmp = td;
		data.push_back(tmp);
	}
	void addData(Data d)
	{
		this->data.push_back(d);
	}
	
	//打印一个元祖的数据
	//按顺序输出，中间的间隔还需调整 
	void Printdata()
	{
		std::vector<Data>::iterator it;
		for( it = data.begin(); it != data.end(); it++ )
		{
			if( (*it).type == INT)
				std::cout << (*it).idata << "\t\t\t";
			else if( (*it).type == FLOAT)
				std::cout << (*it).fdata << "\t\t\t";
			else
				std::cout << (*it).sdata << "\t\t\t";
		}
		std::cout << '\n';
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
public:
	std::string table_name;
	struct Attribute attr;
	struct Index indices; 
	std::vector<Tuple> tuples;
	
	Table(){};
	Table( std::string name, Attribute tmpa ): table_name(name), attr(tmpa){};
	
	void PrintTable();
	void PrintTable( int* limitattr );
	
	int InsertTuple( Tuple &tmpt );
	int DeleteTuple( Tuple &tmpt );
	
	int SetIndex( int numofattr, std::string name );
	int DropIndex( std::string name );
	
	std::string GetTablename()
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
	Index GetIndex()
	{
		return indices;
	} 
};

#endif
