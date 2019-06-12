#ifndef ERROR_H
#define ERROR_H

#include <exception>
#include <iostream>
class minisql_exception
{
public:
	minisql_exception(const std::string &msg) : s(msg){}
	void add_msg(const std::string addition) {
		s.insert(0, ":");
		s.insert(0, addition);
	}
	void print() {
		std::cout << s << '\n';
	}
private:
	std::string s;
};

//表已存在异常
class TABLE_EXISTED : public std::exception {

};

//表不存在异常
class TABLE_NOT_EXISTED : public std::exception {

};
//目标属性不存在异常
class ATTR_NOT_EXIST : public std::exception {

};
//where条件中的两个数据的类型不匹配异常
class WHERE_TYPE_NOT_MATCH : public std::exception {

};
//Tuple属性不匹配异常
class TUPLE_ATTR_NOT_MATCH : public std::exception {

};
//主键冲突异常
class PRIM_KEY_CONFLICT : public std::exception {

};
//数据独立性异常
class UNIQUE_CONFLICT :public std::exception {

};

#endif 