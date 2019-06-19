#include "index_manager.h"

// 构造需要传入BufferManager的引用	
IndexManager::IndexManager(BufferManager &bm) : bm(bm){}

// 析构时自动将b+tree持久化存储
IndexManager::~IndexManager()
{
	write_back();
}


void IndexManager::create_index(const std::string &table_name, const std::string &index_name, int type)
{
	auto file_name = "./database/index/"+table_name+"_"+index_name;
	if(is_file_exist(file_name))
	{
		// 不应该出现这个情况
		// 如果出现这个情况 可能是上一次删索引的时候没删除文件
		// 这是系统内部的逻辑错误
		assert(false);
	}
	else
	{
		// 新建索引文件
		new_file(file_name);

		// 新建bplustree
		// 记录bplustree的属性
		auto block = bm.getPage(file_name, 0);
		// root 位置
		*((int*)block) = -1;
		*((int*)(block+sizeof(int))) = -1;
		bm.modifyPage(bm.getPageId(file_name, 0));
		// 假如bplustree中存在索引树 是编程逻辑bug
		if(is_BPTree_exist(file_name, type))
		{
			assert(false);
		}
		else
		{
			if(type == INT_TYPE)
			{
				int_index[file_name] = std::make_shared<BPTree<int>>(bm, get_degree(type), file_name, type);
			}
			else if(type == FLOAT_TYPE)
			{
				float_index[file_name] = std::make_shared<BPTree<float>>(bm, get_degree(type), file_name, type);
			}
			else if(type > 0)
			{
				string_index[file_name] = std::make_shared<BPTree<std::string>>(bm, get_degree(type), file_name, type);
				if(string_index_length.count(file_name) == 0)
				{
					string_index_length[file_name] = type;
				}
			}
		}
	}
}


bool IndexManager::find_key(const std::string &table_name, const std::string &index_name, Data data, std::vector<int> &block_id)
{
	auto file_name = "./database/index/" + table_name + "_" + index_name;
	
	// 首先查看内存中是否存在b+tree索引
	// 不存在就需要先从硬盘中加载
	if(!is_BPTree_exist(file_name, data.type))
	{
		//从硬盘中加载
		read_into(file_name, data.type);
	}
	if(data.type == INT_TYPE)
	{
		if(int_index[file_name]->_find(data.idata, block_id))
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else if(data.type == FLOAT_TYPE)
	{
		if(float_index[file_name]->_find(data.fdata, block_id))
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else if(data.type > 0)
	{
		if(string_index[file_name]->_find(data.sdata, block_id))
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		assert(false);
	}
}


bool IndexManager::find_range_key(const std::string &table_name, const std::string &index_name, const condition &cond, std::vector<int> &block_ids)
{
	auto file_name = "./database/index/" + table_name + "_" + index_name;
	int type;
	if(cond.l_op != -1)
	{
		type = cond.start.type;
	}
	else if(cond.r_op != -1)
	{
		type = cond.end.type;
	}
	// 编程逻辑错误
	else
	{
		assert(false);
	}
	// 首先查看内存中是否存在b+tree索引
	// 不存在就需要先从磁盘中加载
	if(!is_BPTree_exist(file_name, type))
	{
		// 从硬盘中加载
		read_into(file_name, type);
	}
	if(type == INT_TYPE)
	{
		if(cond.l_op != -1 && cond.r_op != -1)
		{
			return int_index[file_name]->_find_range(cond.start.idata, cond.end.idata, cond.l_op, cond.r_op, block_ids);
		}

		if(cond.r_op != -1)
		{
			return int_index[file_name]->_find_range_lt(cond.end.idata, cond.r_op, block_ids);
		}

		if(cond.l_op != -1)
		{
			return int_index[file_name]->_find_range_gt(cond.start.idata, cond.l_op, block_ids);
		}
		// 运算符错误
		assert(false);
	}
	else if(type == FLOAT_TYPE)
	{
		if(cond.l_op != -1 && cond.r_op != -1)
		{
			return float_index[file_name]->_find_range(cond.start.fdata, cond.end.fdata, cond.l_op, cond.r_op, block_ids);
		}

		if(cond.r_op != -1)
		{
			return float_index[file_name]->_find_range_lt(cond.end.fdata, cond.r_op, block_ids);
		}

		if(cond.l_op != -1)
		{
			return float_index[file_name]->_find_range_gt(cond.start.fdata, cond.l_op, block_ids);
		}
		// 运算符错误
		assert(false);
	}
	else if(type > 0)
	{
		if(cond.l_op != -1 && cond.r_op != -1)
		{
			return string_index[file_name]->_find_range(cond.start.sdata, cond.end.sdata, cond.l_op, cond.r_op, block_ids);
		}

		if(cond.r_op != -1)
		{
			return string_index[file_name]->_find_range_lt(cond.end.sdata, cond.r_op, block_ids);
		}

		if(cond.l_op != -1)
		{
			return string_index[file_name]->_find_range_gt(cond.start.sdata, cond.l_op, block_ids);
		}
		// 运算符错误
		assert(false);
	}
	// 编程逻辑错误
	else
	{
		assert(false);
	}
}


bool IndexManager::insert_index(const std::string &table_name, const std::string &index_name, Data data, int block_id)
{
	auto file_name = "./database/index/" + table_name + "_" + index_name;
	
	// 首先查看内存中是否存在b+tree索引
	// 不存在就需要先从硬盘中加载
	if(!is_BPTree_exist(file_name, data.type))
	{
		//从硬盘中加载
		read_into(file_name, data.type);
	}
	if(data.type == INT_TYPE)
	{
		return int_index[file_name]->_insert(data.idata, block_id);
	}
	else if(data.type == FLOAT_TYPE)
	{
		return float_index[file_name]->_insert(data.fdata, block_id);
	}
	else if(data.type > 0)
	{
		return string_index[file_name]->_insert(data.sdata, block_id);
	}
	// 错误
	else
	{
		assert(false);
	}
}


bool IndexManager::delete_index(const std::string &table_name, const std::string &index_name, Data data)
{
	auto file_name = "./database/index/" + table_name + "_" + index_name;
	// 首先查看内存中是否存在b+tree索引
	// 不存在就需要先从硬盘中加载
	if(!is_BPTree_exist(file_name, data.type))
	{
		//从硬盘中加载
		read_into(file_name, data.type);
	}
	if(data.type == INT_TYPE)
	{
		return int_index[file_name]->_delete(data.idata);
	}
	else if(data.type == FLOAT_TYPE)
	{
		return float_index[file_name]->_delete(data.fdata);
	}
	else if(data.type > 0)
	{
		return string_index[file_name]->_delete(data.sdata);
	}
	return true;
}

// 删除索引
void IndexManager::drop_index(const std::string &table_name, const std::string &index_name, int type)
{
	auto file_name = "./database/index/" + table_name + "_" + index_name;

	// 删除内存中的b+tree
	if(is_BPTree_exist(file_name, type))
	{
		if(type == INT_TYPE)
		{
			int_index.erase(file_name);
		}
		else if(type == FLOAT_TYPE)
		{
			float_index.erase(file_name);
		}
		// 字符串
		else if(type > 0)
		{
			string_index.erase(file_name);
			string_index_length.erase(file_name);
		}
	}

	// 删除内存中的内容


	// 删除硬盘上的文件
	if(is_file_exist(file_name))
	{
		if(remove(file_name.c_str()) != 0)
			throw minisql_exception("Inner Error: remove index file error!");
	}

	// 不可能出现, 如果出现则是出现了编程逻辑bug
	// 可能是创建索引的时候没有create
	else
	{
		assert(false);
	}
}


// 新建一个索引文件
void IndexManager::new_file(const std::string &file_name)
{
	std::fstream file;
	file.open(file_name, std::ios::out);
}

// 判断b+tree是否存在内存
bool IndexManager::is_BPTree_exist(const std::string &index, int type)
{
	if(type == INT_TYPE)
	{
		if(int_index.count(index) != 0)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else if(type == FLOAT_TYPE)
	{
		if(float_index.count(index) != 0)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else if(type > 0)
	{
		if(string_index.count(index) != 0)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	// 出现未知类型说明编程逻辑错误
	else 
	{
		assert(false);
	}
}


void IndexManager::write_back()
{
	for(auto &it : int_index)
	{
		it.second->write_all_back();
	}

	for(auto &it : float_index)
	{
		it.second->write_all_back();
	}

	for(auto &it : string_index)
	{
		it.second->write_all_back();
	}
}


void IndexManager::read_into(const std::string &file_name, int type)
{
	auto block = bm.getPage(file_name, 0);
	auto root = *((int*)(block));
	auto lefist_leaf = *((int*)(block+sizeof(int)));
	if(type == INT_TYPE)
	{
		auto bptree = std::make_shared<BPTree<int>>(bm, get_degree(type), file_name, type, root, lefist_leaf);
		int_index[file_name] = bptree;
	}
	else if(type == FLOAT_TYPE)
	{
		auto bptree = std::make_shared<BPTree<float>>(bm, get_degree(type), file_name, type, root, lefist_leaf);
		float_index[file_name] = bptree;
	}
	else if(type > 0)
	{
		auto bptree = std::make_shared<BPTree<std::string>>(bm, get_degree(type), file_name, type, root, lefist_leaf);
		string_index[file_name] = bptree;
		if(string_index_length.count(file_name) == 0)
		{
			string_index_length[file_name] = type;
		}
	}
}

int IndexManager::get_degree(int type)
{
	if(type == INT_TYPE)
	{
		return (_PAGESIZE - sizeof(char) - 5*sizeof(int))/(2*sizeof(int))+1;
	}
	else if(type == FLOAT_TYPE)
	{
		return (_PAGESIZE - sizeof(char) - 5*sizeof(int))/(sizeof(float) + sizeof(int))+1;
	}
	else if(type > 0)
	{
		return (_PAGESIZE - sizeof(char) - 5*sizeof(int))/(sizeof(int) + type)+1;
	}
	else
	{
		assert(false);
	}
}