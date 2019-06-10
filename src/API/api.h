// 
//	api.h
//	

#ifndef _API_H_
#define _API_H_ 1

#include <iostream>
#include <cmath>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include "../base.h" 
#include "../CatalogManager/catalogmanager.h"
#include "../IndexManager/index_manager.h"
#include "../RecordManager/RecordManager.h"

class API
{
	private:
	public:
		Catalog CL;
		RecordManager RM;
		IndexManager IM;

		API(){};
		~API(){};

		Table Combine(Table &table1, std::string tattr, int optype, Data key);
		Table ReMove(Table &table1, std::string tattr, int optype, Data key);
		int CreateTable(std::string tablename, Attribute attr);
		int DropTable(std::string tablename);
		int CreateIndex(std::string tablename, std::string attr, std::string indexname);
		int DropIndex(std::string tablename, std::string indexname);
		int Insert(std::string tablename, std::vector<Data> tuple);
		int Delete(std::string tablename, SelectCondition scondition);
		Table Select(std::string tablename, std::vector<std::string> attr, SelectCondition scondition);
};

#endif