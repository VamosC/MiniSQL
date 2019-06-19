#pragma once
#include "../BufferManager/BufferManager.h"
#include "../base.h"
#include "bplustree.h"
#include "../util.h"
#include "../ERROR.h"

#include <cstdio>
#include <iostream>
#include <fstream>
#include <map>

class IndexManager
{
public:
	// 构造需要传入BufferManager的引用	
	IndexManager(BufferManager &bm);
	// 析构时自动将b+tree持久化存储
	~IndexManager();

	// 新建索引
	// 如何使用:
	// index_manager.create_index("student", "name", -1);
	// 表示新建表student的name索引, 类型是整形
	// 最后一个参数是type
	void create_index(const std::string &table_name, const std::string &index_name, int type);

	// 查找key对应的block_id
	// 如何使用:
	// std::vector<int> arr;
	// index_manager.find_index("student", "name", data, arr);
	// 返回值是是否找到
	bool find_key(const std::string &table_name, const std::string &index_name, Data data, std::vector<int> &block_id);

	// 范围查找
	// 如何使用:
	// std::vector<int> arr;
	// auto cond = condition{.l_op = 2, .start = xxx, .r_op = 2, .end = xxx};
	// index_manager.find_range_key("student", "name", cond, arr);
	bool find_range_key(const std::string &table_name, const std::string &index_name, const condition &cond, std::vector<int> &block_ids);

	// 添加key
	// 如何使用:
	// index_manager.insert_index("student", "name", data);
	bool insert_index(const std::string &table_name, const std::string &index_name, Data data, int block_id);
	
	// 删除key
	// 如何使用:
	// index_manager.delete_index("student", "name", data);
	bool delete_index(const std::string &table_name, const std::string &index_name, Data data);
	
	// 删除索引
	// 如何使用:
	// index_manager.drop_index("student", "name", -1);
	// 表示删除表student的name索引, 类型是整形
	// 最后一个参数是type
	void drop_index(const std::string &table_name, const std::string &index_name, int type);

	// 避免拷贝构造
	IndexManager(const IndexManager& im) = delete;
private:
	static const int INT_TYPE = -1;
	static const int FLOAT_TYPE = 0;
	BufferManager &bm;
	std::map<std::string, std::shared_ptr<BPTree<int>>> int_index;
	std::map<std::string, std::shared_ptr<BPTree<float>>> float_index;
	std::map<std::string, std::shared_ptr<BPTree<std::string>>> string_index;
	std::map<std::string, int> string_index_length;

	// 判断b+tree是否存在内存
	bool is_BPTree_exist(const std::string &index, int type);

	// 新建索引文件
	void new_file(const std::string &file_name);

	void write_back();

	void read_into(const std::string &file_name, int type);

	int get_degree(int type);

};