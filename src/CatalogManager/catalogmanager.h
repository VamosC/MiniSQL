//
//	catalogmanager.h
//	目录管理

#ifndef _CATALOGMANAGER_H_
#define _CATALOGMANAGER_H_ 1

#include <iostream>
#include <iomanip>
#include <cmath>
#include <string>
#include <vector>
#include <sstream>
#include "../base.h" 
#include "../BufferManager/BufferManager.h"
#define TABLE_PATH "./database/catalog/CatalogFile"


class CatalogManager{
	private:
		BufferManager &buffer_manager;
		//把表格信息写入写出文件时需要用到转换的函数 
		int String2Num(const std::string &tmp);
		std::string Num2String(int tmp);
		
		//返回表在文件中的位置
		// int GetTablePlace(std::string tablename, int& suitable_block);
		//返回表名
		// std::string getTableName(std::string buffer, int start, int& end);
		
	public:
		CatalogManager(BufferManager &bm) : buffer_manager(bm){}
		//得到存放某表格信息的块数 
		int GetBlockAmount(const std::string &table_name);
		//关于表格的操作 
		
		//创建表格
		//输入：表格名称、表格属性、索引对象、主码 
		//输出: true-成功； false-失败,包含异常 
		// int CreateTable(std::string table_name, Attribute attr, Index indices, int primary_key);
		
		//删除表格
		//输入：表格名称
		//输出：1-成功； 0-失败,包含异常  
		// int DropTable(std::string tablename);
		 
		//通过表名查看表是否存在	
		//输入：表格名称
		//输出：正整数-索引序号； 0-不存在	
		// bool isTableExist(std::string tablename);
		
		//打印表格信息  ??待定，不知道查询结果的反馈方式 
		// void PrintTable(std::string tablename, Attribute tattr); 
	
	
		//关于属性和索引
		
		//某一属性是否存在
		//输入：表格名称、属性名称
		//输出：非负数-位置； -1-不存在		 
		// int isAttributeExist(std::string tablename, std::string tattr);
				
		//得到某表的全部属性,必须在表存在时才可以用 
		//输入：表格名称
		//输出：Attribute结构数据
		// Attribute GetTableAttribute(std::string tablename);
		
		//在指定属性上建立索引
		//输入：表格名称、属性名称、索引名称
		//输出：1-成功； 0-失败,包含异常
		// int CreateIndex(std::string table_name, std::string tattr, std::string index_name);
		
		//删除索引
		//输入：表格名称、索引名称
		//输出：1-成功； 0-失败,包含异常
		// int DropIndex(std::string tablename, std::string indexname);
		
		//索引是否存在
		//输入：表格名称、索引名称
		//输出：正整数 - index的序号，0-不存在		 
		// int isIndexExist(std::string tablename, std::string indexname);
		 
		//得到某表的全部索引,必须在表存在时才可以用 
		//输入：表格名称
		//输出：Index结构数据
		// Index GetTableIndex(std::string tablename);
};

#endif 
 
