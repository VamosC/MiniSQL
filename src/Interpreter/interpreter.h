//
//interpreter.h
// 

#ifndef _INTERPRETER_H_
#define _INTERPRETER_H_ 1
 
#include <iostream>
#include <string>
#include <sstream>
#include <fstream> 
#include <Windows.h>
#include "stdafx.h"
#include "../base.h"
#include "../CatalogManager/catalogmanager.h"
//引用其他部分 


class Interpreter
{
public:
	//构造函数,得到操作指令字符串并把它存储到当前指令中去 
	Interpreter(string &input);
	//析构函数 
	~Interpreter() {};
	
	

	//判断是哪种命令语句 共9种,之后调用相应的函数？？ 
	//输入：表格名称、表格属性、索引对象、主码 
	//输出: 1-成功； 0-失败,包含异常 
	int JudgeAndExec();
	
	//具体操作函数 
	//判断语句是否正确，给出错误原因/拆解调用执行函数 
	int ExecCreateTable();
	int ExecDropTable();
	int ExecCreateIndex();
	int ExecDropIndex();
	int ExecSelect();
	int ExecInsert();
	int ExecDelete();
	void ExecFile(); 
	
	//对于流的操作
	void GetInstruction(string &input); 
	string GetWord();

private:
	istringstream instruction;

};


