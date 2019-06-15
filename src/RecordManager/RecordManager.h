#ifndef _RECORDMANAGER_H_
#define _RECORDMANAGER_H_ 1
#define INF 1e6
#define IS_DELETED '1'

#include <cstdio> 
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <cassert>
#include <algorithm>
#include "../base.h"
#include "../CatalogManager/catalogmanager.h"
#include "../BufferManager/BufferManager.h"
#include "../IndexManager/index_manager.h"
#include "../ERROR.h"

class RecordManager 
{
private:
	BufferManager &buffer_manager;
	CatalogManager &catalog_manager;
	IndexManager &index_manager;
	const WHERE op_table[6] = {EQUAL, NOT_EQUAL, LESS, GREATER, LESS_OR_EQUAL, GREATER_OR_EQUAL};
	void removeDuplicate(std::vector<int> &block_ids);
	void combine(std::vector<int> &tmp, std::vector<int> &block_ids);
public:
	RecordManager(BufferManager &bm, CatalogManager &cm, IndexManager &im) : buffer_manager(bm), catalog_manager(cm), index_manager(im) {}
	//返回整张表
	//输入：表名
	//输出：Table类型对象
	Table selectRecord(const std::string &table_name, std::string result_table_name = "DefaultName");
	//返回包含所有目标属性满足Where条件的记录的表
	//输入：表名，目标属性，一个Where类型的对象
	//输出：Table类型对象
	Table selectRecord(const std::string &table_name, const std::string &to_attr, Where where, std::string result_table_name = "DefaultName");
	//建立表文件
	//输入：表名
	//输出：void
	void createTableFile(const std::string &table_name);
	//功能：删除表
	//输入：表名
	//输出：void
	void dropTableFile(const std::string &table_name);
	//向对应表中插入一条记录
	//输入：表名，一个元组
	//输出：void
	void insertRecord(const std::string &table_name, Tuple& tuple);

	//删除对应表中记录（不删除表文件）
	//输入：表名
	//输出：删除的记录数
	int deleteRecord(const std::string &table_name);


	//删除对应表中所有目标属性值满足Where条件的记录
	//输入：表名，目标属性，一个Where类型的对象
	//输出：删除的记录数
	int deleteRecord(const std::string &table_name, SelectCondition scondition);
	//对表中存在的记录建立索引
	//输入：表名，目标属性名
	//输出：void
	void createIndex(const std::string &table_name, const std::string &index_name, const std::string &attr);

	//获取文件大小
	int getBlockNum(const std::string &table_name);

	//insertRecord的辅助函数
	void DoInsertOnRecord(char* p, int offset, int len, const std::vector<Data>& v);

	//deleteRecord的辅助函数
	char* SetDeleteOnRecord(char* p);

	//从内存中读取一个tuple
	Tuple readTuple(const char* p, Attribute attr);

	//获取一个tuple的长度
	int getTupleLength(char* p);

	//判断插入的记录是否和其他记录冲突
	bool isConflict(std::vector<Tuple>& tuples, std::vector<Data>& v, int index);

	//带索引查找
	void searchWithIndex(const std::string &table_name, const std::string &index_name, const Where &where, std::vector<int>& block_ids);

	//在块中进行条件删除
	int queryDeleteInBlock(const std::string &table_name, int block_id, const Attribute &attr, std::vector<int> &index, const SelectCondition &cond);

	//在块中进行条件查询
	void querySelectInBlock(const std::string &table_name, int block_id, const Attribute &attr, int index, const Where &where, std::vector<Tuple>& v);

};


template <typename T>
int getDataLength(T data) {
	std::stringstream stream;
	stream << data;
	return stream.str().length();
}

template <typename T>
bool QueryJudge(T a, T b, WHERE relation) {

	if (relation == LESS)
	{
		if (a < b)
			return true;
		else
			return false;
	}
	else if (relation == LESS_OR_EQUAL)
	{
		if (a <= b)
			return true;
		else
			return false;
	}
	else if (relation == EQUAL)
	{
		if (a == b)
			return true;
		else
			return false;
	}
	else if (relation == GREATER_OR_EQUAL)
	{
		if (a >= b)
			return true;
		else
			return false;
	}
	else if (relation == GREATER)
	{
		if (a > b)
			return true;
		else
			return false;
	}
	else if (relation == NOT_EQUAL)
	{
		if (a != b)
			return true;
		else
			return false;
	}
	// 出现这种情况是编程出错
	else
		assert(false);
}

template <typename T>
void CopyFunc(char* p, int& offset, T data) {
	std::stringstream stream;
	stream << data;
	std::string s1 = stream.str();
	for (int i = 0; i < s1.length(); i++, offset++)
		p[offset] = s1[i];
}


#endif