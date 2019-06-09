//
//	catalogmanager.h
//	目录管理

#ifndef _CATALOGMANAGER_H_
#define _CATALOGMANAGER_H_ 1

#include <iostream>
#include <math>
#include <string>
#include <vector>
#include <sstream>
#include "../base.h" 
#include "../BufferManager/BufferManager.h"
#define TABLE_PATH "./database/catalog/CatalogFile"
using namespace std;


extern BufferManager buffer_manager;


class Catalog{
	private:
		//把表格信息写入写出文件时需要用到转换的函数 
		int String2Num(string tmp);
		string Num2String(int tmp);
		
		//得到存放某表格信息的块数 
		int GetBlockAmount(string tablename);
		//返回表在文件中的位置
		int GetTablePlace(string tablename, int& suitable_block);
		//返回表名
		string getTableName(string buffer, int start, int& end);
		
	public:
		//关于表格的操作 
		
		//创建表格
		//输入：表格名称、表格属性、索引对象、主码 
		//输出: 1-成功； 0-失败,包含异常 
		int CreateTable(string tablename, Attribute attr, Index indices, int primary_key);
		
		//删除表格
		//输入：表格名称
		//输出：1-成功； 0-失败,包含异常  
		int DropTable(string tablename);
		 
		//通过表名查看表是否存在	
		//输入：表格名称
		//输出：正整数-索引序号； 0-不存在	
		int isTableExist(string tablename);
		
		//打印表格信息  ??待定，不知道查询结果的反馈方式 
		void PrintTable(string tablename, Attribute tattr); 
	
	
		//关于属性和索引
		
		//某一属性是否存在
		//输入：表格名称、属性名称
		//输出：true-存在； false-不存在		 
		bool isAttributeExist(string tablename, string tattr);
				
		//得到某表的全部属性,必须在表存在时才可以用 
		//输入：表格名称
		//输出：Attribute结构数据
		Attribute GetTableAttribute(string tablename);
		
		//在指定属性上建立索引
		//输入：表格名称、属性名称、索引名称
		//输出：1-成功； 0-失败,包含异常
		int CreateIndex(string tablename, string tattr, string indexname);
		
		//删除索引
		//输入：表格名称、索引名称
		//输出：1-成功； 0-失败,包含异常
		int DropIndex(string tablename, string indexname);
		
		//索引是否存在
		//输入：表格名称、索引名称
		//输出：正整数 - index的序号，0-不存在		 
		int isIndexExist(string tablename, string indexname);
		 
		//得到某表的全部索引,必须在表存在时才可以用 
		//输入：表格名称
		//输出：Index结构数据
		Index GetTableIndex(string tablename);
};

#endif 
 
