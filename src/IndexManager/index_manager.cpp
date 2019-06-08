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
	auto file_name = "./"+table_name+"_"+index_name;
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

		// 假如bplustree中存在索引树 是编程逻辑bug
		if(is_BPTree_exist(file_name, type))
		{
			assert(false);
		}
		else
		{
			if(type == INT_TYPE)
			{
				int_index[file_name] = std::make_shared<BPTree<int>>(get_degree(type));
			}
			else if(type == FLOAT_TYPE)
			{
				float_index[file_name] = std::make_shared<BPTree<float>>(get_degree(type));
			}
			else if(type > 0)
			{
				string_index[file_name] = std::make_shared<BPTree<std::string>>(get_degree(type));
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
	auto file_name = "./" + table_name + "_" + index_name;
	
	// 首先查看内存中是否存在b+tree索引
	// 不存在就需要先从硬盘中加载
	if(!is_BPTree_exist(file_name, data.type))
	{
		//从硬盘中加载
		read_into(file_name, data.type);
	}
	if(data.type == INT_TYPE)
	{
		NodeFound<int> res;
		if(int_index[file_name]->_find(data.idata, res))
		{
			block_id.push_back((res.node->block_ids)[res.pos]);
			return true;
		}
		else
		{
			return false;
		}
	}
	else if(data.type == FLOAT_TYPE)
	{
		NodeFound<float> res;
		if(float_index[file_name]->_find(data.fdata, res))
		{
			block_id.push_back((res.node->block_ids)[res.pos]);
			return true;
		}
		else
		{
			return false;
		}
	}
	else if(data.type > 0)
	{
		NodeFound<std::string> res;
		if(string_index[file_name]->_find(data.sdata, res))
		{
			block_id.push_back((res.node->block_ids)[res.pos]);
			return true;
		}
		else
		{
			return false;
		}
	}
}


bool IndexManager::find_range_key(const std::string &table_name, const std::string &index_name, const condition &cond, std::vector<int> &block_ids)
{
	auto file_name = "./" + table_name + "_" + index_name;
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
		return true;
	}
	else if(type == FLOAT_TYPE)
	{
		return true;
	}
	else if(type > 0)
	{
		return true;
	}
	// 编程逻辑错误
	else
	{
		assert(false);
	}
}


bool IndexManager::insert_index(const std::string &table_name, const std::string &index_name, Data data, int block_id)
{
	auto file_name = "./" + table_name + "_" + index_name;
	
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
}


bool IndexManager::delete_index(const std::string &table_name, const std::string &index_name, Data data)
{
	auto file_name = "./" + table_name + "_" + index_name;
	
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
	auto file_name = "./" + table_name + "_" + index_name;

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

	// 删除硬盘上的文件
	if(is_file_exist(file_name))
	{
		if(remove(file_name.c_str()) != 0)
			throw std::string("Inner Error: remove index file error!");
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
		auto leaf = it.second->_get_leftist_leaf();
		auto &file_name = it.first;
		auto i = 0;
		char* p = bm.getPage(file_name, i);
		memset(p, 0, _PAGESIZE);
		auto k = sizeof(char) + sizeof(int);
		int count = 0;
		while(leaf != nullptr)
		{
			for(auto j = 0; j < leaf->keys.size(); j++)
			{
				if(k + 2*sizeof(int) > _PAGESIZE)
				{
					memcpy(p+sizeof(char), &count, sizeof(int));
					bm.modifyPage(bm.getPageId(file_name, i));
					count = 0;
					i++;
					p = bm.getPage(file_name, i);
					memset(p, 0, _PAGESIZE);
					k = sizeof(char) + sizeof(int);
				}
				memcpy(p+k, &(leaf->keys[j]), sizeof(int));
				k += sizeof(int);
				memcpy(p+k, &(leaf->block_ids[j]), sizeof(int));
				k += sizeof(int);
				count++;
			}
			leaf = leaf->next_sibling;
		}
		p[0] = -1;
		memcpy(p+sizeof(char), &count, sizeof(int));
		bm.modifyPage(bm.getPageId(file_name, i));
	}

	for(auto &it : float_index)
	{
		auto leaf = it.second->_get_leftist_leaf();
		auto &file_name = it.first;
		auto i = 0;
		char* p = bm.getPage(file_name, i);
		memset(p, 0, _PAGESIZE);
		auto k = sizeof(char) + sizeof(int);
		int count = 0;
		while(leaf != nullptr)
		{
			for(auto j = 0; j < leaf->keys.size(); j++)
			{
				if(k + sizeof(int) + sizeof(float) > _PAGESIZE)
				{
					memcpy(p+sizeof(char), &count, sizeof(int));
					bm.modifyPage(bm.getPageId(file_name, i));
					count = 0;
					i++;
					p = bm.getPage(file_name, i);
					memset(p, 0, _PAGESIZE);
					k = sizeof(char) + sizeof(int);
				}
				memcpy(p+k, &(leaf->keys[j]), sizeof(float));
				k += sizeof(float);
				memcpy(p+k, &(leaf->block_ids[j]), sizeof(int));
				k += sizeof(int);
				count++;
			}
			leaf = leaf->next_sibling;
		}
		p[0] = -1;
		memcpy(p+sizeof(char), &count, sizeof(int));
		bm.modifyPage(bm.getPageId(file_name, i));
	}

	for(auto &it : string_index)
	{
		auto leaf = it.second->_get_leftist_leaf();
		auto &file_name = it.first;
		auto i = 0;
		auto length = string_index_length[file_name];
		char* p = bm.getPage(file_name, i);
		memset(p, 0, _PAGESIZE);
		auto k = sizeof(char) + sizeof(int);
		int count = 0;
		while(leaf != nullptr)
		{
			for(auto j = 0; j < leaf->keys.size(); j++)
			{
				if(k + length + sizeof(int) > _PAGESIZE)
				{
					memcpy(p+sizeof(char), &count, sizeof(int));
					bm.modifyPage(bm.getPageId(file_name, i));
					count = 0;
					i++;
					p = bm.getPage(file_name, i);
					memset(p, 0, _PAGESIZE);
					k = sizeof(char) + sizeof(int);
				}
				memcpy(p+k, leaf->keys[j].c_str(), length);
				k += length;
				memcpy(p+k, &(leaf->block_ids[j]), sizeof(int));
				k += sizeof(int);
				count++;
			}
			leaf = leaf->next_sibling;
		}
		p[0] = -1;
		memcpy(p+sizeof(char), &count, sizeof(int));
		bm.modifyPage(bm.getPageId(file_name, i));
	}
}

int IndexManager::get_block_num(const std::string &file_name)
{
	char* p;
	auto i = 0;
	while(1)
	{
		p = bm.getPage(file_name, i);
		if(p[0] != -1)
		{
			i++;
		}
		else
		{
			return i+1;
		}
	}
}


void IndexManager::read_into(const std::string file_name, int type)
{
	if(type == INT_TYPE)
	{
		auto bptree = std::make_shared<BPTree<int>>(get_degree(type));
		auto i = 0;
		char* p = bm.getPage(file_name, i);
		while(p[0] != -1)
		{
			auto k = sizeof(char) + sizeof(int);
			while(k + 2*sizeof(int) <= _PAGESIZE)
			{
				bptree->_insert(*(int*)(p+k), *(int*)(p+k+sizeof(int)));
				k += 2*sizeof(int);
			}
			i++;
			p = bm.getPage(file_name, i);
		}
		auto count = *(int*)(p+sizeof(char));
		auto k = sizeof(char) + sizeof(int);
		while(count > 0)
		{
			bptree->_insert(*(int*)(p+k), *(int*)(p+k+sizeof(int)));
			k += 2*sizeof(int);
			count--;
		}
		int_index[file_name] = bptree;
	}
	else if(type == FLOAT_TYPE)
	{
		auto bptree = std::make_shared<BPTree<float>>(get_degree(type));
		auto i = 0;
		char* p = bm.getPage(file_name, i);
		while(p[0] != -1)
		{
			auto k = sizeof(char) + sizeof(int);
			while(k + sizeof(float) + sizeof(int) <= _PAGESIZE)
			{
				bptree->_insert(*(float*)(p+k), *(int*)(p+k+sizeof(float)));
				k += (sizeof(int) + sizeof(float));
			}
			i++;
			p = bm.getPage(file_name, i);
		}
		auto count = *(int*)(p+sizeof(char));
		auto k = sizeof(char) + sizeof(int);
		while(count > 0)
		{
			bptree->_insert(*(float*)(p+k), *(int*)(p+k+sizeof(float)));
			k += (sizeof(int) + sizeof(float));
			count--;
		}
		float_index[file_name] = bptree;
	}
	else if(type > 0)
	{
		auto bptree = std::make_shared<BPTree<std::string>>(get_degree(type));
		auto i = 0;
		char* p = bm.getPage(file_name, i);
		while(p[0] != -1)
		{
			auto k = sizeof(char) + sizeof(int);
			while(k + sizeof(int) + type <= _PAGESIZE)
			{
				char* tmp = new char[type+1];
				memset(tmp, 0, type+1);
				memcpy(tmp, p+k, type);
				bptree->_insert(std::string(tmp), *(int*)(p+k+sizeof(type)));
				delete[] tmp;
				k += (sizeof(int) + type);
			}
			i++;
			p = bm.getPage(file_name, i);
		}
		auto count = *(int*)(p+sizeof(char));
		auto k = sizeof(char) + sizeof(int);
		while(count > 0)
		{
			char* tmp = new char[type+1];
			memset(tmp, 0, type+1);
			memcpy(tmp, p+k, type);
			bptree->_insert(std::string(tmp), *(int*)(p+k+sizeof(type)));
			delete[] tmp;
			k += (sizeof(int) + type);
			count--;
		}
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
		return (_PAGESIZE - sizeof(char) - sizeof(int))/(2*sizeof(int));
	}
	else if(type == FLOAT_TYPE)
	{
		return (_PAGESIZE - sizeof(char) - sizeof(int))/(sizeof(float) + sizeof(int));
	}
	else if(type > 0)
	{
		return (_PAGESIZE - sizeof(char) - sizeof(int))/(sizeof(int) + type);
	}
	else
	{
		assert(false);
	}
}

// int main(int argc, char const *argv[])
// {
// 	int x;
// 	BufferManager bm;
// 	// std::string a("132");
// 	// std::string b("ads");
// 	// std::string c("02da");
// 	auto data = Data{.type = -1, .idata = 3};
// 	auto data1 = Data{.type = -1, .idata = 100};
// 	auto data2 = Data{.type = -1, .idata = 4};
// 	IndexManager im(bm);
// 	std::vector<int> block_id;
// 	im.create_index("student", "grade", -1);
// 	im.insert_index("student", "grade", data, 0);
// 	im.insert_index("student", "grade", data1, 100);
// 	im.insert_index("student", "grade", data2, 3);
// 	// im.create_index("student", "age", 4);
// 	// im.insert_index("student", "age", data, 9);
// 	// im.insert_index("student", "age", data1, 0);
// 	// im.insert_index("student", "age", data2, 3);
// 	im.find_key("student", "grade", data1, block_id);
// 	im.drop_index("student", "age", 4);
// 	std::cout << block_id[0] << '\n';
// 	std::cin >> x;
// 	return 0;
// }