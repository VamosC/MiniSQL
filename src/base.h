// 
//	base.h
//	the structure of table

#ifndef _BASE_H_
#define _BASE_H_ 1

#include <iostream>
#include <vector>
#include <string>
using namespace std;


//���� 
//һ�����ݵ������Լ����ݵĴ��
//sdata�ĳ���Ϊ 1~255 
struct Data{
	int type;				//-1 - int; 0 - float; 1 - string
	int idata;
	float fdata;
	string sdata;
};


//���� 
//�����������������͡��Ƿ�Ψһ���Ƿ��������������
//�����30������ 
struct Attribute{
	int amount;
	string attr_name[30];
	int attr_type[30];		//-1 - int; 0 - float; 1 - string
	bool is_unique[30];
	bool is_hasindex[30];
	int primary_key;		//�������� 
};


//Ԫ��
class Tuple{
private:
	vector<Data> data;
public:
	Tuple(){};
	
	//�������� 
	Tuple(Tuple &copytuple)
	{
		std::vector<Data>::iterator it;
		for( it = copytuple.begin(); it != data.end(); it++ )
		{
			struct Data tmp = *it;
			data.push_back(tmp);
		}
	}
	
	
	//����Ԫ��
	//������������insert����ʱ��˳��һ��һ����data������ 
	void Inputdata( Data &td )
	{
		struct Data tmp = td;
		data.push_back(tmp);
	}
	
	
	//��ӡһ��Ԫ�������
	//��˳��������м�ļ��������� 
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


//����
//һ�ű������10������ 
struct Index{
	int amount;
	string name[10]; 
	int whose[10];			//��������Ӧ������ 
	//file where[10];
}; 

//���
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
