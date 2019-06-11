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
		CatalogManager &CL;
		RecordManager &RM;
		IndexManager &IM;
	public:
		API(CatalogManager &catalog_manager, RecordManager &record_manager, IndexManager &index_manager) : CL(catalog_manager), RM(record_manager), IM(index_manager){}
		~API(){}

		Table Combine(Table &table1, std::string tattr, int optype, Data key);
		Table ReMove(Table &table1, std::string tattr, int optype, Data key);
		int CreateTable(const std::string &table_name, Attribute attr);
		int DropTable(const std::string &tablename);
		int CreateIndex(const std::string &table_name, const std::string &attr, const std::string &index_name);
		int DropIndex(const std::string &table_name, const std::string &index_name);
		int Insert(const std::string &table_name, std::vector<Data> tuple);
		int Delete(const std::string &table_name, SelectCondition scondition);
		Table Select(const std::string &table_name, std::vector<std::string> attr, SelectCondition scondition);
};

#endif
