//main
//程序流程控制
//检查命令语法正确性
//接受并届时用户输入的命令


#include <iostream>
#include <string>
#include <sstream>
#include <fstream> 
#include "/Interpreter/interpreter.h"
#include "base.h"

int main()
{
	std::cout << "---------------   Welcome to our MiniSQL   ---------------" << endl;
	//initialization初始化？

	std::string input;
	getline(std::cin, input);
	int result = 1;
	//cout << input; 

	while (result != 2)
	{
		//cout << "1" << endl;
		Interpreter instru(input);
		instru.JudgeAndExec();

		getline(std::cin, input);
		//cout << input; 	
	}

	std::cout << "---------------     See you next time!     ---------------";
}
