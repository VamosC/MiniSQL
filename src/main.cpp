//main
//�������̿���
//��������﷨��ȷ��
//���ܲ���ʱ�û����������


#include <iostream>
#include <string>
#include <sstream>
#include <fstream> 
#include "BufferManager/BufferManager.h"
#include "IndexManager/index_manager.h"
#include "CatalogManager/catalogmanager.h"
#include "RecordManager/RecordManager.h"
#include "Interpreter/interpreter.h"

int main()
{
	std::cout << "---------------   Welcome to our MiniSQL   ---------------" << '\n';
	
	//initialization
	BufferManager buffer_manager;
	IndexManager index_manager(buffer_manager);
	CatalogManager catalog_manager(buffer_manager);
	RecordManager record_manager(buffer_manager, catalog_manager, index_manager);
	API api(catalog_manager, record_manager, index_manager);
	Interpreter interpreter(api, catalog_manager);


	std::string input;
	bool quit_flag = false;
	
	while (!quit_flag)
	{
		do
		{
			getline(std::cin, input);
		}
		while(input == "");
		interpreter.GetInput(input);
		quit_flag = interpreter.JudgeAndExec();
	}

	std::cout << "---------------     See you next time!     ---------------" << '\n';
}
