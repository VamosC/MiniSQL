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

#endif 