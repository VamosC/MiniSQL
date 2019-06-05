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
	BPTree() : degree(4){}
	bool _find(const T& key, NodeFound<T> &res);// 查询key的所在的节点
	bool _insert(const T& key, int block_id);// 插入索引结点
	bool _delete(const T& key);
	void print()
	{
		root->print();
	}
private:
	std::shared_ptr<TreeNode<T>> root;
	int degree;
	void insert_in_parent(std::shared_ptr<TreeNode<T>> &l_node, const T& key, std::shared_ptr<TreeNode<T>> &r_node);
};

template <typename T>
bool BPTree<T>::_find(const T& key, NodeFound<T> &res)
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
bool BPTree<T>::_insert(const T& key, int block_id)
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
					new_node->next_sibling->left_sibling = new_node;

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
void BPTree<T>::insert_in_parent(std::shared_ptr<TreeNode<T>> &l_node, const T& key, std::shared_ptr<TreeNode<T>> &r_node)
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
				new_node->next_sibling->left_sibling = new_node;
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
bool BPTree<T>::_delete(const T& key)
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
			// 删除key后仍然half-full 直接删除
			if(node->num - 1 >= round_2(degree-1))
			{
				node->keys.erase(node->keys.begin()+res.pos);
				node->block_ids.erase(node->block_ids.begin()+res.pos);
				node->num--;
				return true;
			}

			// 删除结点后key过少
			else
			{
				// 首先查看是否为根结点, special case
				if(node->parent == nullptr)
				{
					node->keys.erase(node->keys.begin()+res.pos);
					node->block_ids.erase(node->block_ids.begin()+res.pos);
					node->num--;
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
						// 先删
						node->keys.erase(node->keys.begin()+res.pos);
						node->block_ids.erase(node->block_ids.begin()+res.pos);

						// 借左兄弟最后一个key
						auto l_bro = node->left_sibling;
						auto key_tmp = l_bro->keys.back();
						node->keys.insert(node->keys.begin(), l_bro->keys.back());
						node->block_ids.insert(node->block_ids.begin(), l_bro->keys.back());

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
								break;
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
								// 先删
								node->keys.erase(node->keys.begin()+res.pos);
								node->block_ids.erase(node->block_ids.begin()+res.pos);

								// 借右边兄弟的第一个key
								auto r_bro = node->next_sibling;
								node->keys.push_back(r_bro->keys.front());
								node->block_ids.push_back(r_bro->block_ids.front());

								// 抹掉右兄弟的第一个结点
								r_bro->keys.erase(r_bro->keys.begin());
								r_bro->block_ids.erase(r_bro->block_ids.begin());
								r_bro->num--;
								// 无需更新父结点
								return true;
							}
						}
						// 右兄弟不存在或者借不到, 那么和左兄弟进行合并操作
						return true;
					}
				}

				// 左边不存在询问右兄弟
				else if(node->next_sibling != nullptr)
				{
					// 借得到key
					if(node->next_sibling->num - 1 >= round_2(degree-1))
					{
						// 先删
						node->keys.erase(node->keys.begin()+res.pos);
						node->block_ids.erase(node->block_ids.begin()+res.pos);

						// 借右边兄弟的第一个key
						auto r_bro = node->next_sibling;
						node->keys.push_back(r_bro->keys.front());
						node->block_ids.push_back(r_bro->block_ids.front());

						// 抹掉右兄弟的第一个结点
						r_bro->keys.erase(r_bro->keys.begin());
						r_bro->block_ids.erase(r_bro->block_ids.begin());
						r_bro->num--;
						// 无需更新父结点
						return true;
					}
					// 借不到key, 和右兄弟进行合并
					else
					{
						return true;
					}
				}
				// 不可能既没有左兄弟又没有右兄弟
				// 根结点的情况已经检查过
				else
					assert(false);
			}
		}
	}
}
