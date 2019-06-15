// 
//	api.cpp
//	

#include "api.h"
static bool Datacompare(const Tuple &tuple1, const Tuple &tuple2)
{
	std::vector<Data> data1 = tuple1.getData();
	std::vector<Data> data2 = tuple2.getData();

	if (data1[0].type == INT)
		return data1[0].idata < data2[0].idata;
	else if(data1[0].type == FLOAT)
		return data1[0].fdata < data2[0].fdata;
	else
		return data1[0].sdata < data2[0].sdata;
}

void API::CreateTable(const std::string &table_name, const Attribute &attr)
{
	Index tmp_index;
	tmp_index.amount = 0;

	// 检查属性是否重复
	for(auto i = 0; i < attr.amount; i++)
	{
		for(auto j = i+1; j < attr.amount; j++)
			if(attr.attr_name[i] == attr.attr_name[j])
			{
				std::cout << "Create table " << table_name << " error!" <<":Duplicated attribute names are not allowed!" << '\n';
				return;
			}
	}
	// 如果存在主键 则检查
	// 检查primary key是否unique
	if(attr.primary_key != -1)
	if(!attr.is_unique[attr.primary_key])
	{
		std::cout << "Create table " << table_name << " error!" << ":Primary key must be unique" << '\n';
		return;
	}
	try
	{
		auto start = clock();
		CL.CreateTable(table_name, attr, tmp_index, attr.primary_key);
		RM.createTableFile(table_name);
		auto end = clock();
		std::cout << "Query OK, 0 rows affected";
		std::cout << " (" << (double)(end - start)/CLOCKS_PER_SEC << " sec)" << '\n';
	}
	catch(minisql_exception &e)
	{
		e.add_msg("Create table failed!");
		e.print();
	}
}

void API::DropTable(const std::string &table_name)
{
	if(!CL.isTableExist(table_name))
	{
		std::cout << ("Delete table " + table_name + " error!:");
		std::cout << ("Table " + table_name + " not exists!") << '\n';
	}
	else
	{
		try
		{
			auto start = clock();
			Index tmp_index= CL.GetTableIndex(table_name);
			Attribute tmp_attr = CL.GetTableAttribute(table_name);
			for (int i = 0; i < tmp_index.amount; i++)
				IM.drop_index(table_name, tmp_index.name[i], tmp_attr.attr_type[tmp_index.whose[i]]);
			CL.DropTable(table_name);
			RM.dropTableFile(table_name);
			auto end = clock();
			std::cout << "Query OK, 0 rows affected";
			std::cout << " (" << (double)(end - start)/CLOCKS_PER_SEC << " sec)" << '\n';
		}
		catch(minisql_exception &e)
		{
			e.add_msg("Delete table " + table_name + " error!");
			e.print();
		}
	}
}


void API::CreateIndex(const std::string &table_name, const std::string &attr, const std::string &index_name)
{
	if(!CL.isTableExist(table_name))
	{
		std::cout << "Create index " + index_name + " on " + table_name + " error!:";
		std::cout << ("Table " + table_name + " not exists!") << '\n';
	}
	else
	{
		try
		{
			auto i = CL.isAttributeExist(table_name, attr);
			auto j = CL.isIndexExist(table_name, index_name);
			Attribute curattr = CL.GetTableAttribute(table_name);
			Index curindex = CL.GetTableIndex(table_name);
			if(i == -1)
			{
				std::cout << "Create index " + index_name + " on " + table_name + " error!" << (":Attribute " + attr + " on " + table_name + " not exists!") << '\n';
				return;
			}

			if(!curattr.is_unique[i])
			{
				std::cout << "Create index " + index_name + " on " + table_name + " error!" << ":Attribute as index must be unique!" << '\n';
				return;
			}

			if (j != -1)
			{
				std::cout << "Create index " + index_name + " on " + table_name + " error!" << ("Index " + index_name + " on " + table_name + " exists!") << '\n';
				return;
			}
			auto start = clock();
			CL.CreateIndex(table_name, attr, index_name);
			IM.create_index(table_name, index_name, curattr.attr_type[i]);
			RM.createIndex(table_name, index_name, attr);
			auto end = clock();
			std::cout << "Query OK, 0 rows affected";
			std::cout << " (" << (double)(end - start)/CLOCKS_PER_SEC << " sec)" << '\n';
		}
		catch(minisql_exception &e)
		{
			e.add_msg("Create index " + index_name + " on " + table_name + " error!");
			e.print();
		}
	}
}

void API::DropIndex(const std::string &table_name, const std::string &index_name)
{
	if(!CL.isTableExist(table_name))
	{
		std::cout << ("Drop index " + index_name + " on table " + table_name + " error!:");
		std::cout << ("Table " + table_name + " not exists!") << '\n';
	}
	else
	{
		auto index_pos = CL.isIndexExist(table_name, index_name);
		if (index_pos == -1)
		{
			std::cout << ("Drop index " + index_name + " on table " + table_name + " error!:");
			std::cout << ("Index " + index_name + " on " + table_name + " not exists!") << '\n';
			return;
		}
		try
		{
			auto cur_index = CL.GetTableIndex(table_name);
			auto cur_attr = CL.GetTableAttribute(table_name);
			auto start = clock();
			IM.drop_index(table_name, index_name, cur_attr.attr_type[cur_index.whose[index_pos]]);
			CL.DropIndex(table_name, index_name);
			auto end = clock();
			std::cout << "Query OK, 0 rows affected";
			std::cout << " (" << (double)(end - start)/CLOCKS_PER_SEC << " sec)" << '\n';
		}
		catch(minisql_exception &e)
		{
			e.add_msg("Drop index " + index_name + " on table " + table_name + " error!");
			e.print();
		}
	}
}

void API::Insert(const std::string &table_name, const std::vector<Data> &tuple)
{
	if(!CL.isTableExist(table_name))
	{
		std::cout << ("Insert table " +  table_name + " error!:");
		std::cout << ("Table " + table_name + " not exists!") << '\n';
	}
	else
	{
		Tuple tmp(tuple);
		try
		{
			auto start = clock();
			RM.insertRecord(table_name, tmp);
			auto end = clock();
			std::cout << "Query OK, 1 row affected";
			std::cout << " (" << (double)(end - start)/CLOCKS_PER_SEC << " sec)" << '\n';
		}
		catch(minisql_exception &e)
		{
			e.add_msg("Insert table" + table_name  + " error!");
			e.print();
		}
	}
}


void API::Delete(const std::string &table_name, SelectCondition scondition)
{
	int result = 0;
	Table re;
	if(!CL.isTableExist(table_name))
	{
		std::cout << ("Delete from table " +  table_name + " error!:");
		std::cout << ("Table " + table_name + " not exists!") << '\n';
	}
	else
	{
		try
		{
			auto start = clock();
			if (scondition.amount == 0)
				result = RM.deleteRecord(table_name);
			else
			{
				result = RM.deleteRecord(table_name, scondition);
			}
			auto end = clock();
			if(result > 1)
			{
				std::cout << "Query OK," << " " << result << " rows affected";
			}
			else
			{
				std::cout << "Query OK," << " " << result << " row affected";
			}
			std::cout << " (" << (double)(end - start)/CLOCKS_PER_SEC << " sec)" << '\n';
		}
		catch(minisql_exception &e)
		{
			e.add_msg("Delete from table " + table_name + " error!");
			e.print();
		}
	}
}

void API::Select(const std::string &table_name, std::vector<std::string> attr, SelectCondition scondition)
{
	Table result;
	if(!CL.isTableExist(table_name))
	{
		std::cout << ("Delete from table " +  table_name + " error!:");
		std::cout << ("Table " + table_name + " not exists!") << '\n';
	}
	else
	{
		for(auto i = 0; i < scondition.amount; i++)
		{
			if(CL.isAttributeExist(table_name, scondition.attr[i]) == -1)
			{
				std::cout << ("Delete from table " +  table_name + " error!:");
				std::cout << ("Attribute " + scondition.attr[i] + " not exists!") << '\n';
				return;
			}
		}
		Attribute cur_attr = CL.GetTableAttribute(table_name);
		try
		{
			auto start = clock();
			if (scondition.amount == 0)
				result = RM.selectRecord(table_name);
			else
			{
				auto curwhere = Where{.data = scondition.key[0], .relation_character = op_table[scondition.operationtype[0]]};
				std::cout << curwhere.data.sdata << '\n';
				std::cout << curwhere.relation_character << '\n';
				result = RM.selectRecord(table_name, scondition.attr[0], curwhere );
				for (int i = 1; i < scondition.amount; i++)
				{
					std::cout << scondition.key[i].sdata << '\n';
					std::cout << scondition.operationtype[i] << '\n';
					result = Combine(result, table_name, scondition.attr[i], scondition.operationtype[i], scondition.key[i]);
				}
			}
			auto end = clock();
			result.PrintTable();
			if(result.size() > 1)
				std::cout << result.size() << " rows in set"; 
			else
				std::cout << result.size() << " row in set";
			std::cout << " (" << (double)(end - start)/CLOCKS_PER_SEC << " sec)" << '\n';
		}
		catch(minisql_exception &e)
		{
			e.add_msg("Select from table " + table_name + " error!");
			e.print();
		}
	}
}

Table API::Combine(Table &table1, const std::string &table_name, const std::string &tattr, int optype, Data key)
{	
	auto where = Where{.data = key, .relation_character = op_table[optype]};
	Table table2 = RM.selectRecord(table_name, tattr, where);
	Table res(table1.GetTableName(), table1.GetAttr());
	auto &tuple1 = table1.GetTuples();
	auto &tuple2 = table2.GetTuples();
	std::sort(tuple1.begin(), tuple1.end(), Datacompare);
	std::sort(tuple2.begin(), tuple2.end(), Datacompare);
	auto &tuple_res = res.GetTuples();
	int pos = 0;
	for(auto i = 0; i < tuple1.size(); i++)
	{
		for(auto j = pos; j < tuple2.size(); j++)
		{
			if(tuple1[i] == tuple2[j])
			{
				pos = j;
				tuple_res.push_back(tuple1[i]);
				break;
			}
		}
	}
	return res;
}

// Table API::ReMove(Table &table1, std::string tattr, int optype, Data key)
// {
// 	Table result;
// 	std::vector<Tuple>& rtuple = result.tuples;
// 	rtuple = table1.tuples;

// 	int curattr;
// 	for (curattr = 0; curattr < table1.attr.amount; curattr++)
// 		if (table1.attr.attr_name[curattr] == tattr)
// 			break;

// 	int i = 0;
// 	while( i != rtuple.size() )
// 	{
// 		std::vector<Data> curdata = rtuple[i].getData();
// 		int curre = 0;
// 		switch (key.type)
// 		{
// 		case -1:
// 		{
// 			if (optype == 0)//=
// 				curre = key.idata == curdata[curattr].idata;
// 			else if (optype == 1)//<>
// 				curre = key.idata != curdata[curattr].idata;
// 			else if (optype == 2)//<
// 				curre = key.idata > curdata[curattr].idata;
// 			else if (optype == 3)//>
// 				curre = key.idata < curdata[curattr].idata;
// 			else if (optype == 4)//<=
// 				curre = key.idata >= curdata[curattr].idata;
// 			else if (optype == 5)//>=
// 				curre = key.idata <= curdata[curattr].idata;
// 			break;
// 		}
// 		case 0:
// 		{
// 			if (optype == 0)//=
// 				curre = key.fdata == curdata[curattr].fdata;
// 			else if (optype == 1)//<>
// 				curre = key.fdata != curdata[curattr].fdata;
// 			else if (optype == 2)//<
// 				curre = key.fdata > curdata[curattr].fdata;
// 			else if (optype == 3)//>
// 				curre = key.fdata < curdata[curattr].fdata;
// 			else if (optype == 4)//<=
// 				curre = key.fdata >= curdata[curattr].fdata;
// 			else if (optype == 5)//>=
// 				curre = key.fdata <= curdata[curattr].fdata;
// 			break;
// 		}
// 		default:
// 		{
// 			if (optype == 0)//=
// 				curre = key.sdata == curdata[curattr].sdata;
// 			else if (optype == 1)//<>
// 				curre = key.sdata != curdata[curattr].sdata;
// 			else if (optype == 2)//<
// 				curre = key.sdata > curdata[curattr].sdata;
// 			else if (optype == 3)//>
// 				curre = key.sdata < curdata[curattr].sdata;
// 			else if (optype == 4)//<=
// 				curre = key.sdata >= curdata[curattr].sdata;
// 			else if (optype == 5)//>=
// 				curre = key.sdata <= curdata[curattr].sdata;
// 			break;
// 		}
// 		}//switch
// 		if (curre != 0)
// 			rtuple.erase(rtuple.begin() + i);
// 		else
// 			i++;
// 	}

// 	std::sort(rtuple.begin(), rtuple.end(), Datacompare);
// 	return result;
// }
