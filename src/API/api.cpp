// 
//	api.cpp
//	

#include "api.h"
static bool Datacompare(const Tuple &tuple1, const Tuple &tuple2)
{
	std::vector<Data> data1 = tuple1.getData();
	std::vector<Data> data2 = tuple2.getData();

	if (data1[0].type == -1)
		return data1[0].idata < data2[0].idata;
	else if(data1[0].type == 0)
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

	// 检查primary key是否unique
	if(!attr.is_unique[attr.primary_key])
	{
		std::cout << "Create table " << table_name << " error!" << ":Primary key must be unique" << '\n';
		return;
	}

	try
	{
		CL.CreateTable(table_name, attr, tmp_index, attr.primary_key);
		RM.createTableFile(table_name);
		std::cout << "Create table " << table_name << " success!" << '\n';
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
		std::cout << ("Table " + table_name + " not exists!") << '\n';
	}
	else
	{
		try
		{
			Index tmp_index= CL.GetTableIndex(table_name);
			Attribute tmp_attr = CL.GetTableAttribute(table_name);
			for (int i = 0; i < tmp_index.amount; i++)
				IM.drop_index(table_name, tmp_index.name[i], tmp_attr.attr_type[tmp_index.whose[i]]);
			CL.DropTable(table_name);
			RM.dropTableFile(table_name);\
			std::cout << ("Delete table " + table_name + " success!") << '\n';
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
			CL.CreateIndex(table_name, attr, index_name);
			IM.create_index(table_name, index_name, curattr.attr_type[i]);
			RM.createIndex(table_name, index_name, attr);
			std::cout << "Create index " + index_name + " on " + table_name + " success!" << '\n';
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
		std::cout << ("Table " + table_name + " not exists!") << '\n';
	}
	else
	{
		auto index_pos = CL.isIndexExist(table_name, index_name);
		if (index_pos == -1)
		{
			std::cout << ("Index " + index_name + " on " + table_name + " not exists!") << '\n';
			return;
		}
		try
		{
			auto cur_index = CL.GetTableIndex(table_name);
			auto cur_attr = CL.GetTableAttribute(table_name);
			IM.drop_index(table_name, index_name, cur_attr.attr_type[cur_index.whose[index_pos]]);
			CL.DropIndex(table_name, index_name);
			std::cout << "Drop index " + index_name + " on table " + table_name + " success!" << '\n';
		}
		catch(minisql_exception &e)
		{
			e.add_msg("Drop index " + index_name + " on table " + table_name + " error!");
			e.print();
		}
	}
}

int API::Insert(const std::string &table_name, std::vector<Data> tuple)
{
	Tuple tmp;
	for (int i = 0; i < tuple.size(); i++ )
		tmp.addData(tuple[i]);

	RM.insertRecord( table_name, tmp);
	return 1;
}


int API::Delete(const std::string &table_name, SelectCondition scondition)
{
	int result = 0;
	Table re;

	if (scondition.amount == 0)
		result = RM.deleteRecord(table_name);
	else
	{
		Where curwhere;
		curwhere.data = scondition.key[0];
		switch (scondition.operationtype[0])
		{
			case 0:
			{
				curwhere.relation_character = EQUAL;
				break;
			}
			case 1:
			{
				curwhere.relation_character = NOT_EQUAL;
				break;
			}
			case 2:
			{
				curwhere.relation_character = LESS;
				break;
			}
			case 3:
			{
				curwhere.relation_character = GREATER;
				break;
			}
			case 4:
			{
				curwhere.relation_character = LESS_OR_EQUAL;
				break;
			}
			case 5:
			{
				curwhere.relation_character = GREATER_OR_EQUAL;
				break;
			}
			default: break;
		}

		result = RM.deleteRecord(table_name, scondition.attr[0], curwhere);

		//for (int i = 1; i < scondition.amount; i++)
			//re = ReMove(result, scondition.attr[i], scondition.operationtype[i], scondition.key[i]);

	}

	return result;
}

Table API::Select(const std::string &table_name, std::vector<std::string> attr, SelectCondition scondition)
{
	Table result;
	Attribute cur_attr = CL.GetTableAttribute(table_name);
	if (attr.size() == cur_attr.amount)
		return RM.selectRecord(table_name);
	else
	{
		Where curwhere;
		curwhere.data = scondition.key[0];
		switch (scondition.operationtype[0])
		{
			case 0:
			{
				curwhere.relation_character = EQUAL;
				break;
			}
			case 1:
			{
				curwhere.relation_character = NOT_EQUAL;
				break;
			}
			case 2:
			{
				curwhere.relation_character = LESS;
				break;
			}
			case 3:
			{
				curwhere.relation_character = GREATER;
				break;
			}
			case 4:
			{
				curwhere.relation_character = LESS_OR_EQUAL;
				break;
			}
			case 5:
			{
				curwhere.relation_character = GREATER_OR_EQUAL;
				break;
			}
			default: break;
		}

		result = RM.selectRecord( table_name, scondition.attr[0], curwhere );

		for (int i = 1; i < scondition.amount; i++)
			result = Combine(result, scondition.attr[i], scondition.operationtype[i], scondition.key[i]);
	
		return result;
	}
}

Table API::Combine(Table &table1, std::string tattr, int optype, Data key)
{
	Table result;
	std::vector<Tuple>& rtuple = result.tuples;
	std::vector<Tuple> curtuple = table1.tuples;

	int curattr;
	for (curattr = 0; curattr < table1.attr.amount; curattr++)
		if (table1.attr.attr_name[curattr] == tattr)
			break;

	for (int i = 0; i < curtuple.size(); i++)
	{
		std::vector<Data> curdata = curtuple[i].getData();
		int curre = 0;
		switch (key.type)
		{
			case -1:
			{
				if (optype == 0)//=
					curre = key.idata == curdata[curattr].idata;
				else if (optype == 1)//<>
					curre = key.idata != curdata[curattr].idata;
				else if (optype == 2)//<
					curre = key.idata > curdata[curattr].idata;
				else if (optype == 3)//>
					curre = key.idata < curdata[curattr].idata;
				else if (optype == 4)//<=
					curre = key.idata >= curdata[curattr].idata;
				else if (optype == 5)//>=
					curre = key.idata <= curdata[curattr].idata;
				break;
			}
			case 0:
			{
				if (optype == 0)//=
					curre = key.fdata == curdata[curattr].fdata;
				else if (optype == 1)//<>
					curre = key.fdata != curdata[curattr].fdata;
				else if (optype == 2)//<
					curre = key.fdata > curdata[curattr].fdata;
				else if (optype == 3)//>
					curre = key.fdata < curdata[curattr].fdata;
				else if (optype == 4)//<=
					curre = key.fdata >= curdata[curattr].fdata;
				else if (optype == 5)//>=
					curre = key.fdata <= curdata[curattr].fdata;
				break;
			}
			default:
			{
				if (optype == 0)//=
					curre = key.sdata == curdata[curattr].sdata;
				else if (optype == 1)//<>
					curre = key.sdata != curdata[curattr].sdata;
				else if (optype == 2)//<
					curre = key.sdata > curdata[curattr].sdata;
				else if (optype == 3)//>
					curre = key.sdata < curdata[curattr].sdata;
				else if (optype == 4)//<=
					curre = key.sdata >= curdata[curattr].sdata;
				else if (optype == 5)//>=
					curre = key.sdata <= curdata[curattr].sdata;
				break;
			}
		}//switch
		if (curre != 0)
			rtuple.push_back(curtuple[i]);
	}

	std::sort(rtuple.begin(), rtuple.end(), Datacompare);
	return result;
}

Table API::ReMove(Table &table1, std::string tattr, int optype, Data key)
{
	Table result;
	std::vector<Tuple>& rtuple = result.tuples;
	rtuple = table1.tuples;

	int curattr;
	for (curattr = 0; curattr < table1.attr.amount; curattr++)
		if (table1.attr.attr_name[curattr] == tattr)
			break;

	int i = 0;
	while( i != rtuple.size() )
	{
		std::vector<Data> curdata = rtuple[i].getData();
		int curre = 0;
		switch (key.type)
		{
		case -1:
		{
			if (optype == 0)//=
				curre = key.idata == curdata[curattr].idata;
			else if (optype == 1)//<>
				curre = key.idata != curdata[curattr].idata;
			else if (optype == 2)//<
				curre = key.idata > curdata[curattr].idata;
			else if (optype == 3)//>
				curre = key.idata < curdata[curattr].idata;
			else if (optype == 4)//<=
				curre = key.idata >= curdata[curattr].idata;
			else if (optype == 5)//>=
				curre = key.idata <= curdata[curattr].idata;
			break;
		}
		case 0:
		{
			if (optype == 0)//=
				curre = key.fdata == curdata[curattr].fdata;
			else if (optype == 1)//<>
				curre = key.fdata != curdata[curattr].fdata;
			else if (optype == 2)//<
				curre = key.fdata > curdata[curattr].fdata;
			else if (optype == 3)//>
				curre = key.fdata < curdata[curattr].fdata;
			else if (optype == 4)//<=
				curre = key.fdata >= curdata[curattr].fdata;
			else if (optype == 5)//>=
				curre = key.fdata <= curdata[curattr].fdata;
			break;
		}
		default:
		{
			if (optype == 0)//=
				curre = key.sdata == curdata[curattr].sdata;
			else if (optype == 1)//<>
				curre = key.sdata != curdata[curattr].sdata;
			else if (optype == 2)//<
				curre = key.sdata > curdata[curattr].sdata;
			else if (optype == 3)//>
				curre = key.sdata < curdata[curattr].sdata;
			else if (optype == 4)//<=
				curre = key.sdata >= curdata[curattr].sdata;
			else if (optype == 5)//>=
				curre = key.sdata <= curdata[curattr].sdata;
			break;
		}
		}//switch
		if (curre != 0)
			rtuple.erase(rtuple.begin() + i);
		else
			i++;
	}

	std::sort(rtuple.begin(), rtuple.end(), Datacompare);
	return result;
}
