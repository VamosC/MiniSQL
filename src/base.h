// 
//	base.h
//	the structure of table

#ifndef _BASE_H_
#define _BASE_H_ 1

#include <iostream>
#include <vector>
#include <string>
using namespace std;


//数据 
//一个数据的类型以及数据的存放
//sdata的长度为 1~255 
struct Data{
	int type;				//1 - int; 0 - float; 2 - string
	int idata;
	float fdata;
	string sdata;
};


//属性 
//数量、属性名、类型、是否唯一、是否存在索引、主码
//最多存放32个属性 
struct Attribute{
	int amount;
	string attr_name[32];
	int attr_type[32];		//1 - int; 0 - float; 2 - string
	bool is_unique[32];
	int primary_key;		//主码的序号 
};


//元组
class Tuple{
private:
	vector<Data> data;
public:
	Tuple(){};
	
	//拷贝函数 
	Tuple(Tuple &copytuple)
	{
		std::vector<Data>::iterator it;
		for( it = copytuple.begin(); it != data.end(); it++ )
		{
			struct Data tmp = *it;
			data.push_back(tmp);
		}
	}
	
	
	//输入元组
	//初步设想是在insert操作时按顺序一个一个把data导进来 
	void Inputdata( Data &td )
	{
		struct Data tmp = td;
		data.push_back(tmp);
	}
	
	
	//打印一个元祖的数据
	//按顺序输出，中间的间隔还需调整 
	void Printdata()
	{
		std::vector<Data>::iterator it;
		for( it = data.begin(); it != data.end(); it++ )
		{
			if( (*it).type == -1 )
				cout << (*it).idata << "\t\t\t";
			else if( (*it).type == 0 )
				cout << (*it).fdata << "\t\t\t";
			else
				cout << (*it).sdata << "\t\t\t";
		}
		cout <<	endl;
	}
}; 


//索引
//一张表最多有10个索引 
struct Index{
	int amount;
	string name[10]; 
	int whose[10];			//索引及对应的属性 
	//file where[10];
}; 

//表格
class Table{
private:
public:
	string table_name;
	struct Attribute attr;
	struct Index indices; 
	vector<Tuple> tuples;
	
	Table(){};
	Table( string name, Attribute tmpa ): table_name(name), attr(tmpa){};
	
	void PrintTable();
	void PrintTable( int* limitattr );
	
	int InsertTuple( Tuple &tmpt );
	int DeleteTuple( Tuple &tmpt );
	
	int SetIndex( int numofattr, string name );
	int DropIndex( string name );
	
	string GetTablename()
	{
		return table_name;
	}
	Attribute GetAttr()
	{
		return attr;
	}
	vector<Tuple>& GetTuples()
	{
		return tuples;
	} 
	Index GetIndex()
	{
		return indices;
	} 
};

#endif
