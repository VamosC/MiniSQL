#include "../BufferManager/BufferManager.h"
#include "../ERROR.h"
#include <vector>
#include <string>
#include <cassert>
#include <iostream>

typedef char Block;
typedef int BlockId;

// 交换两个值
template <typename T>
static void swap(T&& x, T&& y)
{
	//移动语义避免临时变量的拷贝
	auto tmp = std::move(x);
	x = y;
	y = std::move(tmp);
}

inline int round_2(int degree)
{
	return degree % 2 == 0 ? degree/2 : (degree/2) + 1;
}

// 转换成json方便调试
// 仅可调试int, float
template <typename T>
inline std::string to_string(std::vector<T> &arr)
{
	if(arr.empty())
		return "[]";
	std::string ret = "[";
	for(auto i = 0; i < arr.size(); i++)
	{
		if(i != 0)
			ret.append(", ");
		if constexpr(std::is_same_v<T, std::string>)
		{
			ret.append("\"" + arr[i] + "\"");	
		}
		else
		{
			ret.append("\""+std::to_string(arr[i])+"\"");
		}
	}
	ret.append("]");
	return ret;
}

template <typename T>
class NodeFound;

template <typename T>
class TreeNode
{
public:
	// 初始化结点
	// type来确定string
	TreeNode() : parent(-1), left_sibling(-1), next_sibling(-1), is_deleted(false), is_leaf(false) {}
	TreeNode(Block* block, int type, BlockId id) : is_deleted(false), is_leaf(false)
	{
		this->id = id;
		auto i = 0;
		// 区分叶子结点
		if(block[i] == 1)
		{
			is_leaf = true;
		}

		// 初始化结点相关属性
		i += sizeof(char);
		num = *(int*)(block+i);
		i += sizeof(int);
		parent = *(int*)(block+i);
		i += sizeof(int);
		next_sibling = *(int*)(block+i);
		i += sizeof(int);
		left_sibling = *(int*)(block+i);
		i += sizeof(int);

		// 初始化结点内容
		// 区分叶子和其他结点, 存放不同的内容
		if(is_leaf)
		{
			// 叶子结点存放key和block_id
			for(auto j = num; j > 0; j--)
			{
				if constexpr(std::is_same_v<T, std::string>)
				{
					char *tmp = new char[type+1];
					memset(tmp, 0, type+1);
					memcpy(tmp, block+i, type);
					keys.push_back(T(tmp));
					i += type;
					delete[] tmp;
					block_ids.push_back(*(int*)(block+i));
					i += sizeof(int);
				}
				else
				{
					keys.push_back(*(T*)(block+i));
					i += sizeof(T);
					block_ids.push_back(*(int*)(block+i));
					i += sizeof(int);
				}
			}
		}

		else
		{
			// 其他结点存放的是key和指向孩子结点的指针
			// 注意此时num指的是指针的数量
			children.push_back(*(int*)(block+i));
			i += sizeof(int);
			for(auto j = num-1; j > 0; j--)
			{
				if constexpr(std::is_same_v<T, std::string>)
				{
					char *tmp = new char[type+1];
					memset(tmp, 0, type+1);
					memcpy(tmp, block+i, type);
					keys.push_back(T(tmp));
					i += type;
					delete[] tmp;
					children.push_back(*(int*)(block+i));
					i += sizeof(int);
				}
				else
				{
					keys.push_back(*(T*)(block+i));
					i += sizeof(T);
					children.push_back(*(int*)(block+i));
					i += sizeof(int);
				}
			}
		}
	}
	~TreeNode() {}
	BlockId id;
	int num;// 当前结点的容量(叶子结点为key的容量, 内部结点为指针的容量)
	BlockId parent;// 指向父结点的指针
	BlockId next_sibling;// 指向右边兄弟的指针
	BlockId left_sibling;// 指向左边兄弟的指针
	std::vector<BlockId> children;// 指向孩子结点的指针
	std::vector<T> keys; // 存索引的key
	std::vector<int> block_ids; // 存放真正数据所在的块号
	bool is_deleted;
	
	// 判断是否为叶子
	bool isLeaf() const
	{
		return is_leaf;
	}

	// 设置为叶子
	void setLeaf()
	{
		is_leaf = true;
	}

	// 叶子结点插入key
	void add(const T& key, int block_id)
	{
		keys.push_back(key);
		block_ids.push_back(block_id);
		for(auto i = num - 1; i >= 0; i--)
		{
			if(keys[i] > keys[i+1])
			{
				swap(keys[i], keys[i+1]);
				swap(block_ids[i], block_ids[i+1]);
			}
		}
		num++;
	}

	// 内部结点插入key
	void add(const T& key, std::shared_ptr<TreeNode<T>> node)
	{
		keys.push_back(key);
		children.push_back(node->id);
		for(auto i = num - 2; i >= 0; i--)
		{
			if(keys[i] > keys[i+1])
			{
				swap(keys[i], keys[i+1]);
				swap(children[i+1], children[i+2]);
			}
			else
			{
				break;
			}
		}
		num++;
	}

private:
	bool is_leaf;// 是否为叶子结点
};

template <typename T>
class NodeFound
{
public:
	NodeFound() : pos(-1), node(-1){}
	int pos;
	BlockId node;
};

template <typename T>
class BPTree
{
public:
	BPTree(BufferManager &bm, int degree, const std::string &file_path, int type, BlockId root = -1, BlockId leftist_leaf = -1) 
	: bm(bm), 
	  degree(degree), 
	  file_path(file_path),  
	  type(type),
	  root(root),
	  leftist_leaf(leftist_leaf)
	  {}
	bool _find(const T &key, NodeFound<T> &res);// 查询key的所在的节点
	bool _find(const T &key, std::vector<int> &res);
	bool _insert(const T &key, int block_id);// 插入索引结点
	bool _delete(const T &key);// 删除索引结点
	bool _find_range(const T &start, const T &end, int l_op, int r_op, std::vector<int> &res);// 范围查找
	bool _find_range_lt(const T &end, int r_op, std::vector<int> &res);
	bool _find_range_gt(const T &start, int l_op, std::vector<int> &res);
	BlockId _get_leftist_leaf();
	BlockId getEmptyBlock();
	void write_back(std::shared_ptr<TreeNode<T>> &node);
	void write_all_back()
	{
		auto block = bm.getPage(file_path, 0);
		*(int*)block = root;
		*(int*)(block+sizeof(int)) = _get_leftist_leaf();
		bm.modifyPage(bm.getPageId(file_path, 0));
	}
	void print() // for debug
	{
		auto block = bm.getPage(file_path, root);
		auto root_node = std::make_shared<TreeNode<T>>(block, type, root);
		print(root_node);
	}
	void print(std::shared_ptr<TreeNode<T>> &node)
	{
		std::cout << "{\"node\":[";
		std::cout << to_string(node->keys);
		for(auto i = 0;i < node->children.size(); i++)
		{
			std::cout << ",";
			auto block = bm.getPage(file_path, node->children[i]);
			auto child = std::make_shared<TreeNode<T>>(block, type, node->children[i]);
			print(child);
		}
		std::cout << "]}";
	}
private:
	BufferManager &bm;
	std::string file_path;
	int type;
	BlockId root;
	BlockId leftist_leaf;
	int degree;
	void insert_in_parent(std::shared_ptr<TreeNode<T>> &l_node, const T& key, std::shared_ptr<TreeNode<T>> &r_node);// split后插入父结点
	void adjust_in_parent(std::shared_ptr<TreeNode<T>> &node);// delete后调整父结点
	void left_rotate(std::shared_ptr<TreeNode<T>> &l_node, std::shared_ptr<TreeNode<T>> &r_node);// 辅助函数, delete向左旋转
	void right_rotate(std::shared_ptr<TreeNode<T>> &l_node, std::shared_ptr<TreeNode<T>> &r_node);// 辅助函数, delete向右旋转
};

template <typename T>
bool BPTree<T>::_find(const T &key, std::vector<int> &res)
{
	// 根结点为空
	if(root == -1)
	{
		return false;
	}
	auto block = bm.getPage(file_path, root);
	auto root_node = std::make_shared<TreeNode<T>>(block, type, root);
	// 根是叶子
	if(root_node->isLeaf())
	{
		// 顺序查找
		for(auto i = 0; i < root_node->num; i++)
		{
			if(root_node->keys[i] == key)
			{
				res.push_back(root_node->block_ids[i]);
				return true;
			}
		}
		// 可二分查找加快速度
		return false;
	}

	// 根不是叶子
	else
	{
		// 寻找叶子结点
		auto node = root_node;
		do
		{
			auto i = 0;
			for(i = 0; i < node->num - 1; i++)
			{
				if(node->keys[i] > key)
				{
					break;
				}
			}
			auto block = bm.getPage(file_path, node->children[i]);
			node = std::make_shared<TreeNode<T>>(block, type, node->children[i]);
		}
		while(!node->isLeaf());
		//找到叶结点后进行检查
		for(auto i = 0; i < node->num; i++)
		{
			if(node->keys[i] == key)
			{
				res.push_back(node->block_ids[i]);
				return true;
			}
		}
		return false;
	}
}

template <typename T>
bool BPTree<T>::_find(const T &key, NodeFound<T> &res)
{
	// 根结点为空
	if(root == -1)
	{
		return false;
	}
	auto block = bm.getPage(file_path, root);
	auto root_node = std::make_shared<TreeNode<T>>(block, type, root);
	// 根是叶子
	if(root_node->isLeaf())
	{
		// 顺序查找
		for(auto i = 0; i < root_node->num; i++)
		{
			if(root_node->keys[i] == key)
			{
				res.pos = i;
				res.node = root;
				return true;
			}
		}
		// 可二分查找加快速度

		res.node = root;
		return false;
	}

	// 根不是叶子
	else
	{
		// 寻找叶子结点
		auto node = root_node;
		do
		{
			auto i = 0;
			for(i = 0; i < node->num - 1; i++)
			{
				if(node->keys[i] > key)
				{
					break;
				}
			}
			auto block = bm.getPage(file_path, node->children[i]);
			node = std::make_shared<TreeNode<T>>(block, type, node->children[i]);
		}
		while(!node->isLeaf());
		//找到叶结点后进行检查
		for(auto i = 0; i < node->num; i++)
		{
			if(node->keys[i] == key)
			{
				res.pos = i;
				res.node = node->id;
				return true;
			}
		}
		res.node = node->id;
		return false;
	}
}


template <typename T>
bool BPTree<T>::_find_range(const T& start, const T& end, int l_op, int r_op, std::vector<int> &res)
{
	if(root == -1)
	{
		return false;
	}
	else
	{
		NodeFound<T> found;
		// 不关心能否找到, 只关心在哪个叶结点中
		_find(start, found);
		auto block = bm.getPage(file_path, found.node);
		auto node = std::make_shared<TreeNode<T>>(block, type, found.node);
		auto over_flag = false;
		while(true)
		{
			for(auto i = 0; i < node->keys.size(); i++)
			{
				if(l_op == 2)
				{
					if(r_op == 2)
					{
						if(node->keys[i] > start && node->keys[i] < end)
						{
							res.push_back(node->block_ids[i]);
						}
						// 超过范围
						if(node->keys[i] >= end)
						{
							over_flag = true;
							break;
						}
					}
					else if(r_op == 4)
					{
						if(node->keys[i] > start && node->keys[i] <= end)
						{
							res.push_back(node->block_ids[i]);
						}
						// 超过范围
						if(node->keys[i] > end)
						{
							over_flag = true;
							break;
						}
					}
					// 运算符错误
					else
					{
						assert(false);
					}
				}
				else if(l_op == 4)
				{
					if(r_op == 2)
					{
						if(node->keys[i] >= start && node->keys[i] < end)
						{
							res.push_back(node->block_ids[i]);
						}
						// 超过范围
						if(node->keys[i] >= end)
						{
							over_flag = true;
							break;
						}
					}
					else if(r_op == 4)
					{
						if(node->keys[i] >= start && node->keys[i] <= end)
						{
							res.push_back(node->block_ids[i]);
						}
						// 超过范围
						if(node->keys[i] > end)
						{
							over_flag = true;
							break;
						}
					}
					// 运算符错误
					else
					{
						assert(false);
					}
				}
				// 运算符错误
				else
				{
					assert(false);
				}
			}
			if(over_flag)
			{
				break;
			}
			// 检查该结点是否为最后一个结点
			if(node->next_sibling == -1)
			{
				break;
			}
			else
			{
				auto block = bm.getPage(file_path, node->next_sibling);
				node = std::make_shared<TreeNode<T>>(block, type, node->next_sibling);
			}
		}
		return true;
	}
}

template <typename T>
bool BPTree<T>::_find_range_lt(const T &end, int r_op, std::vector<int> &res)
{
	if(root == -1)
	{
		return false;
	}
	else
	{
		NodeFound<T> found;
		_find(end, found);
		auto block = bm.getPage(file_path, found.node);
		auto node = std::make_shared<TreeNode<T>>(block, type, found.node);
		while(true)
		{
			for(auto i = 0; i < node->keys.size(); i++)
			{
				if(r_op == 2)
				{
					if(node->keys[i] < end)
					{
						res.push_back(node->block_ids[i]);
					}
					else
					{
						break;
					}
				}
				else if(r_op == 4)
				{
					if(node->keys[i] <= end)
					{
						res.push_back(node->block_ids[i]);
					}
					else
					{
						break;
					}
				}
				// 错误的运算符
				else
				{
					assert(false);
				}
			}
			// 检查左边兄弟结点是否存在
			if(node->left_sibling == -1)
			{
				break;
			}
			else
			{
				auto block = bm.getPage(file_path, node->left_sibling);
				node = std::make_shared<TreeNode<T>>(block, type, node->left_sibling);
			}
		}
		return res.size() != 0;
	}
}

template <typename T>
bool BPTree<T>::_find_range_gt(const T &start, int l_op, std::vector<int> &res)
{
	if(root == -1)
	{
		return false;
	}
	else
	{
		NodeFound<T> found;
		_find(start, found);
		auto block = bm.getPage(file_path, found.node);
		auto node = std::make_shared<TreeNode<T>>(block, type, found.node);
		while(true)
		{
			for(auto i = 0; i < node->keys.size(); i++)
			{
				if(l_op == 2)
				{
					if(node->keys[i] > start)
					{
						res.push_back(node->block_ids[i]);
					}
				}
				else if(l_op == 4)
				{
					if(node->keys[i] >= start)
					{
						res.push_back(node->block_ids[i]);
					}
				}
				// 错误的运算符
				else
				{
					assert(false);
				}
			}
			// 检查右边兄弟是否存在
			if(node->next_sibling == -1)
			{
				break;
			}
			else
			{
				auto block = bm.getPage(file_path, node->next_sibling);
				node = std::make_shared<TreeNode<T>>(block, type, node->next_sibling);
			}
		}
		return res.size() != 0;
	}
}

template <typename T>
BlockId BPTree<T>::getEmptyBlock()
{
	auto i = 0;
	char* block;
	do
	{
		i++;
		block = bm.getPage(file_path, i);
	}
	while(block[0] != '\0');
	return i;
}

template <typename T>
void BPTree<T>::write_back(std::shared_ptr<TreeNode<T>> &node)
{
	auto block = bm.getPage(file_path, node->id);
	auto i = 0;
	if(node->is_deleted)
	{
		block[0] = '\0';
	}
	else
	{
		block[i] = node->isLeaf() ? 1 : -1;
		i += sizeof(char);
		*(int*)(block+i) = node->num;
		i += sizeof(int);
		*(int*)(block+i) = node->parent;
		i += sizeof(int);
		*(int*)(block+i) = node->next_sibling;
		i += sizeof(int);
		*(int*)(block+i) = node->left_sibling;
		i += sizeof(int);

		// 区分叶子和其他结点
		if(node->isLeaf())
		{
			for(auto j = 0; j < node->num; j++)
			{
			    // type > 0 string
				if constexpr(std::is_same_v<T, std::string>)
				{
					char* tmp = new char[type];
					memset(tmp, 0, type);
					memcpy(tmp, node->keys[j].c_str(), node->keys[j].size());
					memcpy(block+i, tmp, type);
					i += type;
					*(int*)(block+i) = node->block_ids[j];
					i += sizeof(int);
					delete[] tmp;
				}
				else
				{
					*(T*)(block+i) = node->keys[j];
					i += sizeof(T);
					*(int*)(block+i) = node->block_ids[j];
					i += sizeof(int);
				}
			}
		}
		else
		{
			auto j = 0;
			for(j = 0; j < node->num-1; j++)
			{
			    // type > 0 string
				if constexpr(std::is_same_v<T, std::string>)
				{
					*(int*)(block+i) = node->children[j];
					i += sizeof(int);
					memcpy(block+i, node->keys[j].c_str(), node->keys[j].size());
					i += type;
				}
				else
				{
					*(int*)(block+i) = node->children[j];
					i += sizeof(int);
					*(T*)(block+i) = node->keys[j];
					i += sizeof(T);
				}
			}
			*(int*)(block+i) = node->children[j];
		}
	}
	auto page_id = bm.getPageId(file_path, node->id);
	bm.modifyPage(page_id);
}

template <typename T>
bool BPTree<T>::_insert(const T &key, int block_id)
{
	// 根结点为空
	if(root == -1)
	{
		auto root_node = std::make_shared<TreeNode<T>>();
		root_node->setLeaf();
		root_node->num = 1;
		root_node->keys.push_back(key);
		root_node->block_ids.push_back(block_id);
		root = getEmptyBlock();
		root_node->id = root;
		write_back(root_node);
		return true;
	}

	// 根结点非空
	else
	{
		NodeFound<T> res;

		// key不存在则插入
		if(!_find(key, res))
		{
			auto block = bm.getPage(file_path, res.node);
			auto node = std::make_shared<TreeNode<T>>(block, type, res.node);
			// 叶子结点满
			if(node->num + 1 == degree)
			{
				auto new_node = std::make_shared<TreeNode<T>>();
				new_node->id = getEmptyBlock();
				new_node->setLeaf();
				auto n_2 = round_2(degree-1);
				node->add(key, block_id);
				// split
				for(auto i = n_2; i < node->num; i++)
				{
					new_node->keys.push_back(node->keys[i]);
					new_node->block_ids.push_back(node->block_ids[i]);
				}

				// 抹去右边n/2
				node->keys.erase(node->keys.begin() + n_2, node->keys.end());
				node->block_ids.erase(node->block_ids.begin() + n_2, node->block_ids.end());

				// 改变左右指针关系
				new_node->next_sibling = node->next_sibling;
				node->next_sibling = new_node->id;
				new_node->left_sibling = node->id;

				// 右兄弟非空
				if(new_node->next_sibling != -1)
				{
					auto block = bm.getPage(file_path, new_node->next_sibling);
					auto r_bro = std::make_shared<TreeNode<T>>(block, type, new_node->next_sibling);
					r_bro->left_sibling = new_node->id;
					write_back(r_bro);
				}

				new_node->num = degree - n_2;
				node->num = n_2;

				new_node->parent = node->parent;
				write_back(node);
				write_back(new_node);
				// 继续调整父结点
				insert_in_parent(node, new_node->keys[0], new_node);
				return true;
			}

			// 叶子结点没满
			else
			{
				node->add(key, block_id);
				write_back(node);
				return true;
			}
		}

		// key已经存在
		else
		{
			throw minisql_exception("index key exists!");
		}
	}
}


template <typename T>
void BPTree<T>::insert_in_parent(std::shared_ptr<TreeNode<T>> &l_node, const T &key, std::shared_ptr<TreeNode<T>> &r_node)
{
	// 根结点
	if(l_node->parent == -1)
	{
		auto node = std::make_shared<TreeNode<T>>();
		node->keys.push_back(key);
		node->children.push_back(l_node->id);
		node->children.push_back(r_node->id);
		node->num = 2;
		node->id = getEmptyBlock();
		// 更新各个结点的父结点
		l_node->parent = node->id;
		r_node->parent = node->id;
		root = node->id;
		write_back(l_node);
		write_back(r_node);
		write_back(node);
	}

	// 内部结点
	else
	{
		auto block = bm.getPage(file_path, l_node->parent);
		auto parent_node = std::make_shared<TreeNode<T>>(block, type, l_node->parent);
		//父结点满
		if(parent_node->num + 1 > degree)
		{
			//split
			auto new_node = std::make_shared<TreeNode<T>>();
			new_node->id = getEmptyBlock();
			auto n_2 = round_2(degree);
			new_node->parent = parent_node->parent;
			parent_node->add(key, r_node);
			auto ret_key = parent_node->keys[n_2-1];
			for(auto i = n_2; i < parent_node->num-1;i++)
			{
				new_node->keys.push_back(parent_node->keys[i]);
				new_node->children.push_back(parent_node->children[i]);
			}
			new_node->children.push_back(parent_node->children[parent_node->children.size()-1]);
			//抹掉右半部分
			parent_node->keys.erase(parent_node->keys.begin()+n_2-1, parent_node->keys.end());
			parent_node->children.erase(parent_node->children.begin()+n_2, parent_node->children.end());
			parent_node->num = n_2;
			new_node->num = degree+1 - n_2;

			for(auto i = 0; i < new_node->children.size(); i++)
			{
				auto block = bm.getPage(file_path, new_node->children[i]);
				auto tmp_node = std::make_shared<TreeNode<T>>(block, type, new_node->children[i]);
				tmp_node->parent = new_node->id;
				write_back(tmp_node);
			}

			// 改变左右兄弟指针关系
			new_node->next_sibling = parent_node->next_sibling;
			parent_node->next_sibling = new_node->id;
			new_node->left_sibling = parent_node->id;
			if(new_node->next_sibling != -1)
			{
				auto block = bm.getPage(file_path, new_node->next_sibling);
				auto r_bro = std::make_shared<TreeNode<T>>(block, type, new_node->next_sibling);
				r_bro->left_sibling = new_node->id;
				write_back(r_bro);
			}
			write_back(parent_node);
			write_back(new_node);
			insert_in_parent(parent_node, ret_key, new_node);
		}

		// 父结点未满
		else
		{
			parent_node->add(key, r_node);
			write_back(parent_node);
		}
	}
}


template <typename T>
bool BPTree<T>::_delete(const T &key)
{
	// 根结点为空
	if(root == -1)
	{
		return false;
	}
	else
	{
		NodeFound<T> res;

		//没有找到相应的key
		if(!_find(key, res))
		{
			return false;
		}

		else
		{
			auto block = bm.getPage(file_path, res.node);
			auto node = std::make_shared<TreeNode<T>>(block, type, res.node);

			// 先删
			node->keys.erase(node->keys.begin()+res.pos);
			node->block_ids.erase(node->block_ids.begin()+res.pos);
			node->num--;

			// 删除key后仍然half-full 直接删除
			if(node->num >= round_2(degree-1))
			{
				write_back(node);
				return true;
			}

			// 删除结点后key过少
			else
			{
				// 首先查看是否为根结点, special case
				if(node->parent == -1)
				{
					write_back(node);
					return true;
				}

				// 若不是根结点
				// 其次查看左右两边兄弟是否能借到
				// 左兄弟
				if(node->left_sibling != -1)
				{
					auto block = bm.getPage(file_path, node->left_sibling);
					auto l_bro = std::make_shared<TreeNode<T>>(block, type, node->left_sibling);
					// 可借
					if(l_bro->num - 1 >= round_2(degree-1))
					{
						// 借左兄弟最后一个key
						auto key_tmp = l_bro->keys.back();
						node->keys.insert(node->keys.begin(), l_bro->keys.back());
						node->block_ids.insert(node->block_ids.begin(), l_bro->block_ids.back());
						node->num++;

						// 抹掉左兄弟的key
						l_bro->num--;
						l_bro->keys.pop_back();
						l_bro->block_ids.pop_back();
						write_back(node);
						write_back(l_bro);
						// 找到左边有key的内部结点
						auto block = bm.getPage(file_path, node->parent);
						auto parent_node = std::make_shared<TreeNode<T>>(block, type, node->parent);
						while(parent_node->children[0] == node->id)
						{
							node = parent_node;
							auto block = bm.getPage(file_path, node->parent);
							parent_node = std::make_shared<TreeNode<T>>(block, type, node->parent);
						}
						auto i = 0;
						for(i = 0; i < parent_node->children.size(); i++)
						{
							if(parent_node->children[i] == node->id)
							{
								break;
							}
						}
						// 修改内部结点的索引值
						parent_node->keys[i-1] = key_tmp;
						write_back(parent_node);
						return true;
					}
					// 不可借问右兄弟
					else
					{
						if(node->next_sibling != -1)
						{
							// 借得到key
							auto block = bm.getPage(file_path, node->next_sibling);
							auto r_bro = std::make_shared<TreeNode<T>>(block, type, node->next_sibling);
							if(r_bro->num - 1 >= round_2(degree-1))
							{
								// 借右边兄弟的第一个key
								node->keys.push_back(r_bro->keys.front());
								node->block_ids.push_back(r_bro->block_ids.front());
								node->num++;

								// 抹掉右兄弟的第一个结点
								r_bro->keys.erase(r_bro->keys.begin());
								r_bro->block_ids.erase(r_bro->block_ids.begin());
								r_bro->num--;
								write_back(r_bro);
								write_back(node);
								auto key_tmp = r_bro->keys.front();

								// 更新父结点
								auto block = bm.getPage(file_path, r_bro->parent);
								auto parent_node = std::make_shared<TreeNode<T>>(block, type, r_bro->parent);
								node = r_bro;
								while(parent_node->children[0] == node->id)
								{
									node = parent_node;
									auto block = bm.getPage(file_path, node->parent);
									parent_node = std::make_shared<TreeNode<T>>(block, type, node->parent);
								}

								auto i = 0;
								for(i = 0; i < parent_node->children.size(); i++)
								{
									if(parent_node->children[i] == node->id)
									{
										break;
									}
								}
								parent_node->keys[i-1] = key_tmp;
								write_back(parent_node);
								return true;
							}
							// 右兄弟借不到, 与相同父母的兄弟merge
							else
							{
								// 和左结点父母相同
								if(node->parent == l_bro->parent)
								{	
									// 复制
									for(auto i = 0; i < node->keys.size(); i++)
									{
										l_bro->keys.push_back(node->keys[i]);
										l_bro->block_ids.push_back(node->block_ids[i]);
										l_bro->num++;
									}

									// 调整指针关系
									l_bro->next_sibling = node->next_sibling;
									node->is_deleted = true;
									auto block = bm.getPage(file_path, node->next_sibling);
									auto neighbor = std::make_shared<TreeNode<T>>(block, type, node->next_sibling);
									neighbor->left_sibling = l_bro->id;
									write_back(neighbor);
									write_back(node);
									write_back(l_bro);
									// 调整父结点
									adjust_in_parent(node);

									return true;
								}

								// 和右结点父母相同
								else if(node->parent == r_bro->parent)
								{
									for(auto i = 0;i < r_bro->keys.size(); i++)
									{
										node->keys.push_back(r_bro->keys[i]);
										node->block_ids.push_back(r_bro->block_ids[i]);
										node->num++;
									}

									// 调整指针关系
									node->next_sibling = r_bro->next_sibling;
									r_bro->is_deleted = true;
									write_back(node);
									write_back(r_bro);
									if(r_bro->next_sibling != -1)
									{
										auto block = bm.getPage(file_path, r_bro->next_sibling);
										auto neighbor = std::make_shared<TreeNode<T>>(block, type, r_bro->next_sibling);
										neighbor->left_sibling = node->id;
										write_back(neighbor);
									}
									// 调整父结点
									adjust_in_parent(r_bro);

									return true;
								}

								// 不可能存在单个结点没有相同父母的兄弟
								else
								{
									assert(false);
								}
							}
						}

						// 右兄弟不存在, 那么和左兄弟进行合并操作
						else
						{
							assert(node->parent == l_bro->parent);
							for(auto i = 0; i < node->keys.size(); i++)
							{
								l_bro->keys.push_back(node->keys[i]);
								l_bro->block_ids.push_back(node->block_ids[i]);
								l_bro->num++;
							}

							// 调整指针关系
							// 右兄弟不存在
							l_bro->next_sibling = node->next_sibling;
							node->is_deleted = true;
							write_back(l_bro);
							write_back(node);
							// 调整父结点
							adjust_in_parent(node);
							return true;
						}

					}
				}

				// 左边不存在询问右兄弟
				else if(node->next_sibling != -1)
				{
					auto block = bm.getPage(file_path, node->next_sibling);
					auto r_bro = std::make_shared<TreeNode<T>>(block, type, node->next_sibling);
					// 借得到key
					if(r_bro->num - 1 >= round_2(degree-1))
					{
						// 借右边兄弟的第一个key
						node->keys.push_back(r_bro->keys.front());
						node->block_ids.push_back(r_bro->block_ids.front());
						node->num++;

						// 抹掉右兄弟的第一个结点
						r_bro->keys.erase(r_bro->keys.begin());
						r_bro->block_ids.erase(r_bro->block_ids.begin());
						r_bro->num--;

						write_back(node);
						write_back(r_bro);
						auto key_tmp = r_bro->keys.front();
						// 更新父结点
						auto block = bm.getPage(file_path, r_bro->parent);
						auto parent_node = std::make_shared<TreeNode<T>>(block, type, r_bro->parent);
						node = r_bro;
						while(parent_node->children[0] == node->id)
						{
							node = parent_node;
							auto block = bm.getPage(file_path, node->parent);
							parent_node = std::make_shared<TreeNode<T>>(block, type, node->parent);
						}

						auto i = 0;
						for(i = 0; i < parent_node->children.size(); i++)
						{
							if(parent_node->children[i] == node->id)
							{
								break;
							}
						}
						parent_node->keys[i-1] = key_tmp;
						write_back(parent_node);

						return true;
					}
					// 借不到key, 和右兄弟进行合并
					else
					{
						assert(node->parent == r_bro->parent);
						for(auto i = 0;i < r_bro->keys.size(); i++)
						{
							node->keys.push_back(r_bro->keys[i]);
							node->block_ids.push_back(r_bro->block_ids[i]);
							node->num++;
						}

						// 调整指针关系
						node->next_sibling = r_bro->next_sibling;
						r_bro->is_deleted = true;
						write_back(node);
						write_back(r_bro);
						if(r_bro->next_sibling != -1)
						{
							auto block = bm.getPage(file_path, r_bro->next_sibling);
							auto neighbor = std::make_shared<TreeNode<T>>(block, type, r_bro->next_sibling);
							neighbor->left_sibling = node->id;
							write_back(neighbor);
						}

						// 调整父结点
						adjust_in_parent(r_bro);

						return true;
					}
				}
				// 不可能既没有左兄弟又没有右兄弟
				// 根结点的情况已经检查过
				else
				{
					assert(false);
				}
			}
			return true;
		}
	}
}


template <typename T>
void BPTree<T>::adjust_in_parent(std::shared_ptr<TreeNode<T>> &node)
{
	// node不可能是根结点
	// 根结点的情况已经处理过
	// 说明parent一定是存在的
	assert(node->parent != -1);
	auto block = bm.getPage(file_path, node->parent);
	auto parent_node = std::make_shared<TreeNode<T>>(block, type, node->parent);

	// 找到要删的结点
	auto i = 0;
	for(i = 0; i < parent_node->children.size(); i++)
	{
		if(parent_node->children[i] == node->id)
			break;
	}
	assert(i >= 0 && i < parent_node->children.size());
	// 先删该结点中的内容
	parent_node->children.erase(parent_node->children.begin()+i);
	parent_node->keys.erase(parent_node->keys.begin()+i-1);
	parent_node->num--;

	// 父结点是根结点 special case
	if(parent_node->parent == -1)
	{
		// 根结点没有key 应当删除该根结点
		if(parent_node->num == 1)
		{
			root = parent_node->children[0];
			parent_node->is_deleted = true;
			auto block = bm.getPage(file_path, root);
			auto root_node = std::make_shared<TreeNode<T>>(block, type, root);
			root_node->parent = -1;
			write_back(root_node);
			write_back(parent_node);
		}
		return;
	}

	// 检查当前结点是否half-full
	// 已经half-full则不需要调整
	// 否则需要进行调整
	if(parent_node->num < round_2(degree))
	{
		// 首先检查能否借
		if(parent_node->left_sibling != -1)
		{
			auto block = bm.getPage(file_path, parent_node->left_sibling);
			auto l_bro = std::make_shared<TreeNode<T>>(block, type, parent_node->left_sibling);
			// 左兄弟可借
			if(l_bro->num-1 >= round_2(degree))
			{
				right_rotate(l_bro, parent_node);
			}

			// 左兄弟不可借
			else
			{
				// 向右兄弟借
				if(parent_node->next_sibling != -1)
				{
					auto block = bm.getPage(file_path, parent_node->next_sibling);
					auto r_bro = std::make_shared<TreeNode<T>>(block, type, parent_node->next_sibling);
					// 右兄弟可借
					if(r_bro->num-1 >= round_2(degree))
					{
						left_rotate(parent_node, r_bro);
					}
				    // 右兄弟不可借
					else
					{
						// 与右兄弟是一个父亲
						if(parent_node->parent == r_bro->parent)
						{
							auto block = bm.getPage(file_path, parent_node->parent);
							auto p = std::make_shared<TreeNode<T>>(block, type, parent_node->parent);
							auto i = 0;

							// 查找对应的索引key
							for(i = 0; i < p->children.size(); i++)
							{
								if(p->children[i] == parent_node->id)
								{
									break;
								}
							}
							// 将右兄弟内容合并到结点中
							parent_node->keys.push_back(p->keys[i]);

							for(auto i = 0; i < r_bro->keys.size(); i++)
							{
								parent_node->keys.push_back(r_bro->keys[i]);
								parent_node->children.push_back(r_bro->children[i]);
								parent_node->num++;

								// 调整父指针
								auto block = bm.getPage(file_path, r_bro->children[i]);
								auto child = std::make_shared<TreeNode<T>>(block, type, r_bro->children[i]);
								child->parent = parent_node->id;
								write_back(child);
							}
							parent_node->children.push_back(r_bro->children[r_bro->children.size()-1]);
							parent_node->num++;
							block = bm.getPage(file_path, r_bro->children[r_bro->children.size()-1]);
							auto child = std::make_shared<TreeNode<T>>(block, type, r_bro->children[r_bro->children.size()-1]);
							child->parent = parent_node->id;
							write_back(child);
							
							// 调整指针关系
							parent_node->next_sibling = r_bro->next_sibling;
							if(r_bro->next_sibling != -1)
							{
								auto block = bm.getPage(file_path, r_bro->next_sibling);
								auto neighbor = std::make_shared<TreeNode<T>>(block, type, r_bro->next_sibling);
								neighbor->left_sibling = parent_node->id;
								write_back(neighbor);
							}
							write_back(parent_node);
							r_bro->is_deleted = true;
							write_back(r_bro);
							adjust_in_parent(r_bro);
						}

						// 与左兄弟是一个父亲
						else if(parent_node->parent == l_bro->parent)
						{
							auto block = bm.getPage(file_path, parent_node->parent);
							auto p = std::make_shared<TreeNode<T>>(block, type, parent_node->parent);
							auto i = 0;
							for(i = 0; i < p->children.size(); i++)
							{
								if(p->children[i] == parent_node->id)
								{
									break;
								}
							}
							l_bro->keys.push_back(p->keys[i-1]);

							// 合并右兄弟到左兄弟中
							for(auto i = 0; i < parent_node->keys.size(); i++)
							{
								l_bro->keys.push_back(parent_node->keys[i]);
								l_bro->children.push_back(parent_node->children[i]);

								// 调整结点大小
								l_bro->num++;

								// 调整父指针
								auto block = bm.getPage(file_path, parent_node->children[i]);
								auto child = std::make_shared<TreeNode<T>>(block, type, parent_node->children[i]);
								child->parent = l_bro->id;
								write_back(child);
							}
							l_bro->children.push_back(parent_node->children[parent_node->children.size()-1]);
							l_bro->num++;
							block = bm.getPage(file_path, l_bro->children[l_bro->children.size()-1]);
							auto child = std::make_shared<TreeNode<T>>(block, type, l_bro->children[l_bro->children.size()-1]);
							child->parent = l_bro->id;
							write_back(child);

							// 调整指针关系
							l_bro->next_sibling = parent_node->next_sibling;
							if(parent_node->next_sibling != -1)
							{
								auto block = bm.getPage(file_path, parent_node->next_sibling);
								auto neighbor = std::make_shared<TreeNode<T>>(block, type, parent_node->next_sibling);
								child->left_sibling = l_bro->id;
								write_back(child);
							}
							parent_node->is_deleted = true;
							write_back(l_bro);
							write_back(parent_node);
							adjust_in_parent(parent_node);
						}
						else
						{
							assert(false);
						}
					}
				}
				// 右兄弟不存在
				// 与左兄弟合并
				else
				{
					assert(l_bro->parent == parent_node->parent);
					auto block = bm.getPage(file_path, parent_node->parent);
					auto p = std::make_shared<TreeNode<T>>(block, type, parent_node->parent);
					auto i = 0;
					for(i = 0; i < p->children.size(); i++)
					{
						if(p->children[i] == parent_node->id)
						{
							break;
						}
					}
					l_bro->keys.push_back(p->keys[i-1]);

					// 合并右兄弟到左兄弟中
					for(auto i = 0; i < parent_node->keys.size(); i++)
					{
						l_bro->keys.push_back(parent_node->keys[i]);
						l_bro->children.push_back(parent_node->children[i]);

						// 调整结点大小
						l_bro->num++;

						// 调整父指针
						auto block = bm.getPage(file_path, parent_node->children[i]);
						auto child = std::make_shared<TreeNode<T>>(block, type, parent_node->children[i]);
						child->parent = l_bro->id;
						write_back(child);
					}
					l_bro->children.push_back(parent_node->children[parent_node->children.size()-1]);
					l_bro->num++;
					block = bm.getPage(file_path, l_bro->children[l_bro->children.size()-1]);
					auto child = std::make_shared<TreeNode<T>>(block, type, l_bro->children[l_bro->children.size()-1]);
					child->parent = l_bro->id;
					write_back(child);

					// 调整指针关系
					l_bro->next_sibling = parent_node->next_sibling;
					if(parent_node->next_sibling != -1)
					{
						auto block = bm.getPage(file_path, parent_node->next_sibling);
						auto neighbor = std::make_shared<TreeNode<T>>(block, type, parent_node->next_sibling);
						neighbor->left_sibling = l_bro->id;
						write_back(neighbor);
					}
					parent_node->is_deleted = true;
					write_back(parent_node);
					write_back(l_bro);
					adjust_in_parent(parent_node);
				}
			}
		}
		else if(parent_node->next_sibling != -1)
		{
			auto block = bm.getPage(file_path, parent_node->next_sibling);
			auto r_bro = std::make_shared<TreeNode<T>>(block, type, parent_node->next_sibling);

			// 右兄弟可借
			if(r_bro->num-1 >= round_2(degree))
			{
				left_rotate(parent_node, r_bro);
			}

			// 右兄弟不可借
			// 与右兄弟合并
			// 与右兄弟一定是相同父亲
			else
			{
				auto block = bm.getPage(file_path, parent_node->parent);
				auto p = std::make_shared<TreeNode<T>>(block, type, parent_node->parent);
				auto i = 0;

				// 查找对应的索引key
				for(i = 0; i < p->children.size(); i++)
				{
					if(p->children[i] == parent_node->id)
					{
						break;
					}
				}
				// 将右兄弟内容合并到结点中
				parent_node->keys.push_back(p->keys[i]);

				for(auto i = 0; i < r_bro->keys.size(); i++)
				{
					parent_node->keys.push_back(r_bro->keys[i]);
					parent_node->children.push_back(r_bro->children[i]);
					parent_node->num++;
					// 调整父指针
					auto block = bm.getPage(file_path, r_bro->children[i]);
					auto child = std::make_shared<TreeNode<T>>(block, type, r_bro->children[i]);
					child->parent = parent_node->id;
					write_back(child);
				}
				parent_node->children.push_back(r_bro->children[r_bro->children.size()-1]);
				parent_node->num++;
				block = bm.getPage(file_path, r_bro->children[r_bro->children.size()-1]);
				auto child = std::make_shared<TreeNode<T>>(block, type, r_bro->children[r_bro->children.size()-1]);
				child->parent = parent_node->id;
				write_back(child);
							
				// 调整指针关系
				parent_node->next_sibling = r_bro->next_sibling;
				if(r_bro->next_sibling != -1)
				{
					auto block = bm.getPage(file_path, r_bro->next_sibling);
					auto neighbor = std::make_shared<TreeNode<T>>(block, type, r_bro->next_sibling);
					neighbor->left_sibling = parent_node->id;
					write_back(neighbor);
				}
				r_bro->is_deleted = true;
				write_back(parent_node);
				write_back(r_bro);
				adjust_in_parent(r_bro);
			}
		}
		else
			assert(false);
	}
}


template <typename T>
void BPTree<T>::left_rotate(std::shared_ptr<TreeNode<T>> &l_node, std::shared_ptr<TreeNode<T>> &r_node)
{
	// 查找相同的父母
	auto p = r_node;
	auto block = bm.getPage(file_path, p->parent);
	auto common_parent = std::make_shared<TreeNode<T>>(block, type, p->parent);
	while(common_parent->children[0] == p->id)
	{
		p = common_parent;
		auto block = bm.getPage(file_path, p->parent);
		common_parent = std::make_shared<TreeNode<T>>(block, type, p->parent);
	}

	auto i = 0;
	for(i = 0; i < common_parent->children.size(); i++)
	{
		if(common_parent->children[i] == p->id)
		{
			break;
		}
	}

	auto key_tmp = common_parent->keys[i-1];

	// 将借到的兄弟插入
	l_node->children.push_back(r_node->children.front());
	l_node->keys.push_back(key_tmp);

	// 调整父指针
	block = bm.getPage(file_path, l_node->children[l_node->children.size()-1]);
	auto child = std::make_shared<TreeNode<T>>(block, type, l_node->children[l_node->children.size()-1]);
	child->parent = l_node->id;
	write_back(child);

	// 调整索引
	common_parent->keys[i-1] = r_node->keys.front();

	// 抹掉右兄弟的索引和指针
	r_node->children.erase(r_node->children.begin());
	r_node->keys.erase(r_node->keys.begin());

	// 调整结点大小
	r_node->num--;
	l_node->num++;
	write_back(l_node);
	write_back(r_node);
	write_back(common_parent);
}

template <typename T>
void BPTree<T>::right_rotate(std::shared_ptr<TreeNode<T>> &l_node, std::shared_ptr<TreeNode<T>> &r_node)
{
	auto p = r_node;
	auto block = bm.getPage(file_path, p->parent);
	auto common_parent = std::make_shared<TreeNode<T>>(block, type, p->parent);

	// 寻找共同的父亲
	while(common_parent->children[0] == p->id)
	{
		p = common_parent;
		auto block = bm.getPage(file_path, p->parent);
		common_parent = std::make_shared<TreeNode<T>>(block, type, p->parent);
	}	

	auto i = 0;
	for(i = 0; i < common_parent->children.size(); i++)
	{
		if(common_parent->children[i] == p->id)
		{
			break;
		}
	}

	auto key_tmp = common_parent->keys[i-1];

	// 将借到的兄弟插入
	r_node->children.insert(r_node->children.begin(), l_node->children.back());
	r_node->keys.insert(r_node->keys.begin(), key_tmp);

	// 调整父指针
	block = bm.getPage(file_path, r_node->children[0]);
	auto child = std::make_shared<TreeNode<T>>(block, type, r_node->children[0]);
	child->parent = r_node->id;
	write_back(child);
	//调整索引
	common_parent->keys[i-1] = l_node->keys.back();

	// 抹掉左兄弟转移的索引和指针
	l_node->children.pop_back();
	l_node->keys.pop_back();

	// 调整结点大小
	r_node->num++;
	l_node->num--;
	write_back(common_parent);
	write_back(r_node);
	write_back(l_node);
}


template <typename T>
BlockId BPTree<T>::_get_leftist_leaf()
{
	if(root == -1)
	{
		return -1;
	}
	else
	{
		auto block = bm.getPage(file_path, root);
		auto node = std::make_shared<TreeNode<T>>(block, type, root);
		while(!node->isLeaf())
		{
			auto block = bm.getPage(file_path, node->children[0]);
			node = std::make_shared<TreeNode<T>>(block, type, node->children[0]);
		}
		return node->id;
	}
}