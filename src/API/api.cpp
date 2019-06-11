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

int API::CreateTable(const std::string &table_name, Attribute attr)
{
	Index tmpindex;
	tmpindex.amount = 0;

	if (!CL.CreateTable(table_name, attr, tmpindex, attr.primary_key))
	{
		std::cout << "create table failed!" << std::endl;
		return 0;
	}
	RM.createTableFile(table_name);

	return 1;
}

int API::DropTable(const std::string &table_name)
{
	Index tmpindex= CL.GetTableIndex(table_name);
	Attribute tmpattr = CL.GetTableAttribute(table_name);

	for (int i = 0; i < tmpindex.amount; i++)
		IM.drop_index(table_name, tmpindex.name[i], tmpattr.attr_type[tmpindex.whose[i]]);
	if (!CL.DropTable(table_name))
	{
		std::cout << "delete table failed!" << std::endl;
		return 0;
	}
	RM.dropTableFile(table_name);

	return 1;
}


int API::CreateIndex(const std::string &table_name, const std::string &attr, const std::string &index_name)
{
	Attribute curattr = CL.GetTableAttribute(table_name);
	Index curindex = CL.GetTableIndex(table_name);
	int i = CL.isAttributeExist(table_name, attr);
	int j = CL.isIndexExist(table_name, index_name);

	if (j != -1)
	{
		std::cout << "index exists!" << std::endl;
		return 0;
	}
	IM.create_index(table_name, index_name, curattr.attr_type[i]);
	CL.CreateIndex(table_name, attr, index_name);
	RM.createIndex(table_name, attr);

	return 1;
}

int API::DropIndex(const std::string &table_name, const std::string &index_name)
{
	Attribute curattr = CL.GetTableAttribute(table_name);
	Index curindex = CL.GetTableIndex(table_name);
	int i;
	for ( i = 0; i < curindex.amount; i++)
		if (curindex.name[i] == index_name)
			break;

	std::string attr = curattr.attr_name[curindex.whose[i]];
	i = CL.isAttributeExist(table_name, attr);
	IM.drop_index(table_name, index_name, curattr.attr_type[i]);
	CL.DropIndex(table_name, index_name);

	return 1;
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
