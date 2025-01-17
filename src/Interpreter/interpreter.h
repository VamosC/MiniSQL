//
//interpreter.h
// 

#ifndef _INTERPRETER_H_
#define _INTERPRETER_H_ 1

#include <iostream>
#include <string>
#include <sstream>
#include <fstream> 
#include "../base.h"
#include "../CatalogManager/catalogmanager.h"
#include "../API/api.h"


class Interpreter
{
public:
	//构造函数,得到操作指令字符串并把它存储到当前指令中去 
	Interpreter(API &api, CatalogManager &cm);
	//析构函数 
	~Interpreter() {};
	
	

	//判断是哪种命令语句 共9种,之后调用相应的函数？？ 
	//输入：表格名称、表格属性、索引对象、主码 
	//输出: 1-成功； 0-失败,包含异常 
	bool JudgeAndExec(std::ifstream *file = nullptr);
	
	//具体操作函数 
	//判断语句是否正确，给出错误原因/拆解调用执行函数 
	void ExecCreateTable(std::ifstream *file);
	void ExecDropTable();
	void ExecCreateIndex();
	void ExecDropIndex();
	void ExecSelect();
	void ExecInsert();
	void ExecDelete();
	bool ExecFile(); 
	
	//对于流的操作
	bool GetInstruction(std::ifstream *file); 
	std::string GetWord();
	void GetInput(std::string &input);
private:
	API &api;
	CatalogManager &catalog_manager;
	std::istringstream instruction; 
};


#endif
