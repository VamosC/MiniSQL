#include <vector>
#include <string>
#include <cassert>
#include <iostream>

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
		ret.append("\""+std::to_string(arr[i])+"\"");
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
	TreeNode() : num(0), parent(nullptr), next_sibling(nullptr), left_sibling(nullptr), is_leaf(false){}
	~TreeNode() {}
	int num;// 当前结点的容量(叶子结点为key的容量, 内部结点为指针的容量)
	std::shared_ptr<TreeNode<T>> parent;// 指向父结点的指针
	std::shared_ptr<TreeNode<T>> next_sibling;// 指向右边兄弟的指针
	std::shared_ptr<TreeNode<T>> left_sibling;// 指向左边兄弟的指针
	std::vector<std::shared_ptr<TreeNode<T>>> children;// 指向孩子结点的指针
	std::vector<T> keys; // 存索引的key
	std::vector<int> block_ids; // 存放真正数据所在的块号
	
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

	// for debug
	void print()
	{
		std::cout << "{\"node\":[";
		std::cout << to_string(keys);
		for(auto i = 0;i < children.size(); i++)
		{
			std::cout << ",";
			children[i]->print();
		}
		std::cout << "]}";
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
		children.push_back(node);
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
	NodeFound() : pos(-1), node(nullptr){}
	int pos;
	std::shared_ptr<TreeNode<T>> node;
};

template <typename T>
class BPTree
{
public:
	BPTree(int degree) : degree(degree){}
	bool _find(const T &key, NodeFound<T> &res);// 查询key的所在的节点
	bool _insert(const T &key, int block_id);// 插入索引结点
	bool _delete(const T &key);// 删除索引结点
	bool _find_range(const T &start, const T &end, int l_op, int r_op, std::vector<int> &res);// 范围查找
	bool _find_range_lt(const T &end, int r_op, std::vector<int> &res);
	bool _find_range_gt(const T &start, int l_op, std::vector<int> &res);
	std::shared_ptr<TreeNode<T>> _get_leftist_leaf();
	void print() // for debug
	{
		root->print();
	}
private:
	std::shared_ptr<TreeNode<T>> root;
	int degree;
	void insert_in_parent(std::shared_ptr<TreeNode<T>> &l_node, const T& key, std::shared_ptr<TreeNode<T>> &r_node);// split后插入父结点
	void adjust_in_parent(std::shared_ptr<TreeNode<T>> &node);// delete后调整父结点
	void left_rotate(std::shared_ptr<TreeNode<T>> &l_node, std::shared_ptr<TreeNode<T>> &r_node);// 辅助函数, delete向左旋转
	void right_rotate(std::shared_ptr<TreeNode<T>> &l_node, std::shared_ptr<TreeNode<T>> &r_node);// 辅助函数, delete向右旋转
};


template <typename T>
bool BPTree<T>::_find(const T &key, NodeFound<T> &res)
{
	// 根结点为空
	if(root == nullptr)
	{
		return false;
	}
	// 根是叶子
	if(root->isLeaf())
	{
		// 顺序查找
		for(auto i = 0; i < root->num; i++)
		{
			if(root->keys[i] == key)
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
		auto node = root;
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
			node = node->children[i];
		}
		while(!node->isLeaf());

		//找到叶结点后进行检查
		for(auto i = 0; i < node->num; i++)
		{
			if(node->keys[i] == key)
			{
				res.pos = i;
				res.node = node;
				return true;
			}
		}
		res.node = node;
		return false;
	}
}


template <typename T>
bool BPTree<T>::_find_range(const T& start, const T& end, int l_op, int r_op, std::vector<int> &res)
{
	if(root == nullptr)
	{
		return false;
	}
	else
	{
		NodeFound<T> found;
		// 不关心能否找到, 只关心在哪个叶结点中
		_find(start, found);
		auto node = found.node;
		auto over_flag = false;
		while(node != nullptr)
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
			node = node->next_sibling;
		}
		return true;
	}
}

template <typename T>
bool BPTree<T>::_find_range_lt(const T &end, int r_op, std::vector<int> &res)
{
	if(root == nullptr)
	{
		return false;
	}
	else
	{
		NodeFound<T> found;
		_find(end, found);
		auto node = found.node;
		while(node != nullptr)
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
			node = node->left_sibling;
		}
		return res.size() != 0;
	}
}

template <typename T>
bool BPTree<T>::_find_range_gt(const T &start, int l_op, std::vector<int> &res)
{
	if(root == nullptr)
	{
		return false;
	}
	else
	{
		NodeFound<T> found;
		_find(start, found);
		auto node = found.node;
		while(node != nullptr)
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
			node = node->next_sibling;
		}
		return res.size() != 0;
	}
}


template <typename T>
bool BPTree<T>::_insert(const T &key, int block_id)
{
	// 根结点为空
	if(root == nullptr)
	{
		root = std::make_shared<TreeNode<T>>();
		root->setLeaf();
		root->num = 1;
		root->keys.push_back(key);
		root->block_ids.push_back(block_id);
		return true;
	}

	// 根结点非空
	else
	{
		NodeFound<T> res;

		// key不存在则插入
		if(!_find(key, res))
		{
			auto node = res.node;

			// 叶子结点满
			if(node->num + 1 == degree)
			{
				auto new_node = std::make_shared<TreeNode<T>>();
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
				node->next_sibling = new_node;
				new_node->left_sibling = node;

				// 右兄弟非空
				if(new_node->next_sibling != nullptr)
				{
					new_node->next_sibling->left_sibling = new_node;
				}

				new_node->num = degree - n_2;
				node->num = n_2;

				new_node->parent = node->parent;

				// 继续调整父结点
				insert_in_parent(node, new_node->keys[0], new_node);
				return true;
			}

			// 叶子结点没满
			else
			{
				node->add(key, block_id);
				return true;
			}
		}

		// key已经存在
		else
		{
			throw std::string("index key exists!");
		}
	}
}


template <typename T>
void BPTree<T>::insert_in_parent(std::shared_ptr<TreeNode<T>> &l_node, const T &key, std::shared_ptr<TreeNode<T>> &r_node)
{
	// 根结点
	if(l_node->parent == nullptr)
	{
		auto node = std::make_shared<TreeNode<T>>();
		node->keys.push_back(key);
		node->children.push_back(l_node);
		node->children.push_back(r_node);
		node->num = 2;
		// 更新各个结点的父结点
		l_node->parent = node;
		r_node->parent = node;
		root = node;
	}

	// 内部结点
	else
	{
		auto parent_node = l_node->parent;
		//父结点满
		if(parent_node->num + 1 > degree)
		{
			//split
			auto new_node = std::make_shared<TreeNode<T>>();
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
				new_node->children[i]->parent = new_node;

			// 改变左右兄弟指针关系
			new_node->next_sibling = parent_node->next_sibling;
			parent_node->next_sibling = new_node;
			new_node->left_sibling = parent_node;
			if(new_node->next_sibling != nullptr)
			{
				new_node->next_sibling->left_sibling = new_node;
			}
			insert_in_parent(parent_node, ret_key, new_node);
		}

		// 父结点未满
		else
		{
			parent_node->add(key, r_node);
		}
	}
}


template <typename T>
bool BPTree<T>::_delete(const T &key)
{
	// 根结点为空
	if(root == nullptr)
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
			auto node = res.node;

			// 先删
			node->keys.erase(node->keys.begin()+res.pos);
			node->block_ids.erase(node->block_ids.begin()+res.pos);
			node->num--;

			// 删除key后仍然half-full 直接删除
			if(node->num >= round_2(degree-1))
			{
				return true;
			}

			// 删除结点后key过少
			else
			{
				// 首先查看是否为根结点, special case
				if(node->parent == nullptr)
				{
					return true;
				}

				// 若不是根结点
				// 其次查看左右两边兄弟是否能借到
				// 左兄弟
				if(node->left_sibling != nullptr)
				{
					// 可借
					if(node->left_sibling->num - 1 >= round_2(degree-1))
					{
						// 借左兄弟最后一个key
						auto l_bro = node->left_sibling;
						auto key_tmp = l_bro->keys.back();
						node->keys.insert(node->keys.begin(), l_bro->keys.back());
						node->block_ids.insert(node->block_ids.begin(), l_bro->block_ids.back());
						node->num++;

						// 抹掉左兄弟的key
						l_bro->num--;
						l_bro->keys.pop_back();
						l_bro->block_ids.pop_back();

						// 找到左边有key的内部结点
						auto parent_node = node->parent;
						while(parent_node->children[0] == node)
						{
							node = parent_node;
							parent_node = parent_node->parent;
						}
						auto i = 0;
						for(i = 0; i < parent_node->children.size(); i++)
						{
							if(parent_node->children[i] == node)
							{
								break;
							}
						}
						// 修改内部结点的索引值
						parent_node->keys[i-1] = key_tmp;
						return true;
					}
					// 不可借问右兄弟
					else
					{
						if(node->next_sibling != nullptr)
						{
							// 借得到key
							if(node->next_sibling->num - 1 >= round_2(degree-1))
							{
								// 借右边兄弟的第一个key
								auto r_bro = node->next_sibling;
								node->keys.push_back(r_bro->keys.front());
								node->block_ids.push_back(r_bro->block_ids.front());
								node->num++;

								// 抹掉右兄弟的第一个结点
								r_bro->keys.erase(r_bro->keys.begin());
								r_bro->block_ids.erase(r_bro->block_ids.begin());
								r_bro->num--;

								auto key_tmp = r_bro->keys.front();

								// 更新父结点
								auto parent_node = r_bro->parent;
								node = r_bro;
								while(parent_node->children[0] == node)
								{
									node = parent_node;
									parent_node = parent_node->parent;
								}

								auto i = 0;
								for(i = 0; i < parent_node->children.size(); i++)
								{
									if(parent_node->children[i] == node)
									{
										break;
									}
								}
								parent_node->keys[i-1] = key_tmp;
								return true;
							}
							// 右兄弟借不到, 与相同父母的兄弟merge
							else
							{
								// 和左结点父母相同
								if(node->parent == node->left_sibling->parent)
								{	
									// 复制
									auto l_bro = node->left_sibling;
									for(auto i = 0; i < node->keys.size(); i++)
									{
										l_bro->keys.push_back(node->keys[i]);
										l_bro->block_ids.push_back(node->block_ids[i]);
										l_bro->num++;
									}

									// 调整指针关系
									l_bro->next_sibling = node->next_sibling;
									node->next_sibling->left_sibling = l_bro;

									// 调整父结点
									adjust_in_parent(node);

									return true;
								}

								// 和右结点父母相同
								else if(node->parent == node->next_sibling->parent)
								{
									auto r_bro = node->next_sibling;
									for(auto i = 0;i < r_bro->keys.size(); i++)
									{
										node->keys.push_back(r_bro->keys[i]);
										node->block_ids.push_back(r_bro->block_ids[i]);
										node->num++;
									}

									// 调整指针关系
									node->next_sibling = r_bro->next_sibling;
									if(r_bro->next_sibling != nullptr)
									{
										r_bro->next_sibling->left_sibling = node;
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
							assert(node->parent == node->left_sibling->parent);
							auto l_bro = node->left_sibling;
							for(auto i = 0; i < node->keys.size(); i++)
							{
								l_bro->keys.push_back(node->keys[i]);
								l_bro->block_ids.push_back(node->block_ids[i]);
								l_bro->num++;
							}

							// 调整指针关系
							// 右兄弟不存在
							l_bro->next_sibling = node->next_sibling;

							// 调整父结点
							adjust_in_parent(node);
							return true;
						}

					}
				}

				// 左边不存在询问右兄弟
				else if(node->next_sibling != nullptr)
				{
					// 借得到key
					if(node->next_sibling->num - 1 >= round_2(degree-1))
					{
						// 借右边兄弟的第一个key
						auto r_bro = node->next_sibling;
						node->keys.push_back(r_bro->keys.front());
						node->block_ids.push_back(r_bro->block_ids.front());
						node->num++;

						// 抹掉右兄弟的第一个结点
						r_bro->keys.erase(r_bro->keys.begin());
						r_bro->block_ids.erase(r_bro->block_ids.begin());
						r_bro->num--;

						auto key_tmp = r_bro->keys.front();
						// 更新父结点
						auto parent_node = r_bro->parent;
						node = r_bro;
						while(parent_node->children[0] == node)
						{
							node = parent_node;
							parent_node = parent_node->parent;
						}

						auto i = 0;
						for(i = 0; i < parent_node->children.size(); i++)
						{
							if(parent_node->children[i] == node)
							{
								break;
							}
						}
						parent_node->keys[i-1] = key_tmp;

						return true;
					}
					// 借不到key, 和右兄弟进行合并
					else
					{
						assert(node->parent == node->next_sibling->parent);
						auto r_bro = node->next_sibling;
						for(auto i = 0;i < r_bro->keys.size(); i++)
						{
							node->keys.push_back(r_bro->keys[i]);
							node->block_ids.push_back(r_bro->block_ids[i]);
							node->num++;
						}

						// 调整指针关系
						node->next_sibling = r_bro->next_sibling;
						if(r_bro->next_sibling != nullptr)
						{
							r_bro->next_sibling->left_sibling = node;
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

	auto parent_node = node->parent;
	assert(parent_node != nullptr);

	// 找到要删的结点
	auto i = 0;
	for(i = 0; i < parent_node->children.size(); i++)
	{
		if(parent_node->children[i] == node)
			break;
	}
	assert(i >= 0 && i < parent_node->children.size());
	// 先删该结点中的内容
	parent_node->children.erase(parent_node->children.begin()+i);
	parent_node->keys.erase(parent_node->keys.begin()+i-1);
	parent_node->num--;

	// 父结点是根结点 special case
	if(parent_node->parent == nullptr)
	{
		// 根结点没有key 应当删除该根结点
		if(parent_node->num == 1)
		{
			root = parent_node->children[0];
			root->parent = nullptr;
		}
		return;
	}

	// 检查当前结点是否half-full
	// 已经half-full则不需要调整
	// 否则需要进行调整
	if(parent_node->num < round_2(degree))
	{
		// 首先检查能否借
		if(parent_node->left_sibling != nullptr)
		{
			auto l_bro = parent_node->left_sibling;
			// 左兄弟可借
			if(l_bro->num-1 >= round_2(degree))
			{
				right_rotate(l_bro, parent_node);
			}

			// 左兄弟不可借
			else
			{
				// 向右兄弟借
				if(parent_node->next_sibling != nullptr)
				{
					auto r_bro = parent_node->next_sibling;
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
							auto p = parent_node->parent;
							auto i = 0;

							// 查找对应的索引key
							for(i = 0; i < p->children.size(); i++)
							{
								if(p->children[i] == parent_node)
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
								r_bro->children[i]->parent = parent_node;
							}
							parent_node->children.push_back(r_bro->children[r_bro->children.size()-1]);
							parent_node->num++;
							r_bro->children[r_bro->children.size()-1]->parent = parent_node;
							
							// 调整指针关系
							parent_node->next_sibling = r_bro->next_sibling;
							if(r_bro->next_sibling != nullptr)
							{
								r_bro->next_sibling->left_sibling = parent_node;
							}
							adjust_in_parent(r_bro);
						}

						// 与左兄弟是一个父亲
						else if(parent_node->parent == l_bro->parent)
						{
							auto p = parent_node->parent;
							auto i = 0;
							for(i = 0; i < p->children.size(); i++)
							{
								if(p->children[i] == parent_node)
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
								parent_node->children[i]->parent = l_bro;
							}
							l_bro->children.push_back(parent_node->children[parent_node->children.size()-1]);
							l_bro->num++;
							l_bro->children[l_bro->children.size()-1]->parent = l_bro;

							// 调整指针关系
							l_bro->next_sibling = parent_node->next_sibling;
							if(parent_node->next_sibling)
							{
								parent_node->next_sibling->left_sibling = l_bro;
							}

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
					auto p = parent_node->parent;
					auto i = 0;
					for(i = 0; i < p->children.size(); i++)
					{
						if(p->children[i] == parent_node)
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
						parent_node->children[i]->parent = l_bro;
					}
					l_bro->children.push_back(parent_node->children[parent_node->children.size()-1]);
					l_bro->num++;
					l_bro->children[l_bro->children.size()-1]->parent = l_bro;

					// 调整指针关系
					l_bro->next_sibling = parent_node->next_sibling;
					if(parent_node->next_sibling)
					{
						parent_node->next_sibling->left_sibling = l_bro;
					}

					adjust_in_parent(parent_node);
				}
			}
		}
		else if(parent_node->next_sibling != nullptr)
		{
			auto r_bro = parent_node->next_sibling;

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
				auto p = parent_node->parent;
				auto i = 0;

				// 查找对应的索引key
				for(i = 0; i < p->children.size(); i++)
				{
					if(p->children[i] == parent_node)
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
					r_bro->children[i]->parent = parent_node;
				}
				parent_node->children.push_back(r_bro->children[r_bro->children.size()-1]);
				parent_node->num++;
				r_bro->children[r_bro->children.size()-1]->parent = parent_node;
							
				// 调整指针关系
				parent_node->next_sibling = r_bro->next_sibling;
				if(r_bro->next_sibling != nullptr)
				{
					r_bro->next_sibling->left_sibling = parent_node;
				}
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
	auto common_parent = p->parent;
	while(common_parent->children[0] == p)
	{
		p = common_parent;
		common_parent = common_parent->parent;
	}

	auto i = 0;
	for(i = 0; i < common_parent->children.size(); i++)
	{
		if(common_parent->children[i] == p)
		{
			break;
		}
	}

	auto key_tmp = common_parent->keys[i-1];

	// 将借到的兄弟插入
	l_node->children.push_back(r_node->children.front());
	l_node->keys.push_back(key_tmp);

	// 调整父指针
	l_node->children[l_node->children.size()-1]->parent = l_node;

	// 调整索引
	common_parent->keys[i-1] = r_node->keys.front();

	// 抹掉右兄弟的索引和指针
	r_node->children.erase(r_node->children.begin());
	r_node->keys.erase(r_node->keys.begin());

	// 调整结点大小
	r_node->num--;
	l_node->num++;
}

template <typename T>
void BPTree<T>::right_rotate(std::shared_ptr<TreeNode<T>> &l_node, std::shared_ptr<TreeNode<T>> &r_node)
{
	auto p = r_node;
	auto common_parent = p->parent;

	// 寻找共同的父亲
	while(common_parent->children[0] == p)
	{
		p = common_parent;
		common_parent = common_parent->parent;
	}	

	auto i = 0;
	for(i = 0; i < common_parent->children.size(); i++)
	{
		if(common_parent->children[i] == p)
		{
			break;
		}
	}

	auto key_tmp = common_parent->keys[i-1];

	// 将借到的兄弟插入
	r_node->children.insert(r_node->children.begin(), l_node->children.back());
	r_node->keys.insert(r_node->keys.begin(), key_tmp);

	// 调整父指针
	r_node->children[0]->parent = r_node;

	//调整索引
	common_parent->keys[i-1] = l_node->keys.back();

	// 抹掉左兄弟转移的索引和指针
	l_node->children.pop_back();
	l_node->keys.pop_back();

	// 调整结点大小
	r_node->num++;
	l_node->num--;
}


template <typename T>
std::shared_ptr<TreeNode<T>> BPTree<T>::_get_leftist_leaf()
{
	if(root == nullptr)
	{
		return nullptr;
	}
	else
	{
		auto node = root;
		while(!node->isLeaf())
		{
			node = node->children[0];
		}
		return node;
	}
}