#if 0

#include "bplustree.h"

int main(int argc, char const *argv[])
{
	BPTree<int> tree;
	tree._insert(10, 2);
	tree._insert(9, 2);
	tree._insert(1, 2);
	tree._insert(2, 2);
	tree._insert(3, 2);
	tree._insert(6, 2);
	tree._insert(7, 2);
	tree._insert(8, 2);
	tree._insert(4, 2);
	tree._insert(5, 2);
	tree._insert(0, 2);
	tree._insert(11, 2);
	tree._insert(12, 2);
	tree._insert(13, 2);
	tree._insert(14, 2);
	tree._insert(15, 2);
	tree._insert(30, 2);
	tree._insert(25, 2);
	tree._insert(28, 2);
	tree._insert(20, 2);
	tree._insert(18, 2);
	tree._insert(17, 2);
	tree._insert(27, 2);
	tree._insert(26, 2);
	tree._delete(12);
	tree._delete(15);
	tree._delete(28);
	tree._delete(25);
	tree._delete(17);
	tree._delete(27);
	tree._delete(30);
	tree._delete(9);
	tree._delete(10);
	tree._delete(11);
	tree._insert(11, 2);
	tree._insert(10, 2);
	tree._insert(9, 2);
	tree._insert(30, 2);
	tree._insert(27, 2);
	tree._insert(17, 2);
	tree._insert(25, 2);
	tree._insert(28, 2);
	tree._insert(15, 2);
	tree._insert(12, 2);

	// BPTree<float> tree;
	// tree._insert(123.2, 2);
	// tree._insert(1.2, 2);
	// tree._insert(3.4, 2);
	// tree.print();

	// tree._delete(7);
	// tree._delete(6);
	// tree._delete(5);
	// tree._delete(9);
	// tree._delete(8);
	// tree._delete(12);
	// tree._delete(13);
	// tree._delete(0);
	// tree._delete(1);
	// tree._delete(10);
	// tree._delete(4);
	// tree._delete(14);
	// tree._delete(11);
	// tree._delete(15);
	// tree._delete(2);
	// tree._delete(3);
	tree.print();
	// auto node = tree._get_leftist_leaf();
	// while(node != nullptr)
	// {
	// 	for(auto it : node->keys)
	// 	{
	// 		std::cout << it << '\n';
	// 	}
	// 	node = node->next_sibling;
	// }

	return 0;
}


#endif