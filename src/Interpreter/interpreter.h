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
//������������ 


class Interpreter
{
public:
	//���캯��,�õ�����ָ���ַ����������洢����ǰָ����ȥ 
	Interpreter(string &input);
	//�������� 
	~Interpreter() {};
	
	

	//�ж�������������� ��9��,֮�������Ӧ�ĺ������� 
	//���룺������ơ�������ԡ������������� 
	//���: 1-�ɹ��� 0-ʧ��,�����쳣 
	int JudgeAndExec();
	
	//����������� 
	//�ж�����Ƿ���ȷ����������ԭ��/������ִ�к��� 
	int ExecCreateTable();
	int ExecDropTable();
	int ExecCreateIndex();
	int ExecDropIndex();
	int ExecSelect();
	int ExecInsert();
	int ExecDelete();
	void ExecFile(); 
	
	//�������Ĳ���
	void GetInstruction(string &input); 
	string GetWord();

private:
	istringstream instruction;

};


