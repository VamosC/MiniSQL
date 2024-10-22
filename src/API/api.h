// 
//	api.h
//	

#ifndef _API_H_
#define _API_H_ 1

#include <iostream>
#include <ctime>
#include <cmath>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include "../ERROR.h"
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
		const WHERE op_table[6] = {EQUAL, NOT_EQUAL, LESS, GREATER, LESS_OR_EQUAL, GREATER_OR_EQUAL};
	public:
		API(CatalogManager &catalog_manager, RecordManager &record_manager, IndexManager &index_manager) : CL(catalog_manager), RM(record_manager), IM(index_manager){}
		~API(){}

		Table Combine(Table &table1, const std::string &table_name, const std::string &tattr, int optype, Data key);
		// Table ReMove(Table &table1, std::string tattr, int optype, Data key);
		void CreateTable(const std::string &table_name, const Attribute &attr);
		void DropTable(const std::string &tablename);
		void CreateIndex(const std::string &table_name, const std::string &attr, const std::string &index_name);
		void DropIndex(const std::string &table_name, const std::string &index_name);
		void Insert(const std::string &table_name, const std::vector<Data> &tuple);
		void Delete(const std::string &table_name, SelectCondition scondition);
		void Select(const std::string &table_name, std::vector<std::string> attr, SelectCondition scondition);
};

#endif
