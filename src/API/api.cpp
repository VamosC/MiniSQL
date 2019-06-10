// 
//	api.cpp
//	

#include "api.h"

int API::CreateTable(std::string tablename, Attribute attr)
{
	Index tmpindex;
	tmpindex.amount = 0;

	if (CL.CreateTable(tablename, attr, tmpindex, attr.primary_key) != 1)
	{
		std::cout << "创建表格档案失败" << std::endl;
		return 0;
	}
	RM.createTableFile(tablename);

	return 1;
}

int API::DropTable(std::string tablename)
{
	Index tmpindex= CL.GetTableIndex(tablename);
	Attribute tmpattr = CL.GetTableAttribute(tablename);

	//删除索引？？不知道在record manager里会不会实现
	for (int i = 0; i < tmpindex.amount; i++)
		drop_index(tablename, tmpindex.name[i], tmpattr.attr_type[tmpindex.whose[i]]);

	//删除档案信息
	if (CL.DropTable(tablename) != 1)
	{
		std::cout << "删除表格档案失败" << std::endl;
		return 0;
	}
	RM.dropTableFile(tablename);

	return 1;
}

int API::CreateIndex(std::string tablename, std::string attr, std::string indexname);
int API::DropIndex(std::string tablename, std::string indexname);

int API::Insert(std::string tablename, std::vector<Data> tuple)
{
	Tuple tmp;
	for (int i = 0; i < tuple.size(); i++ )
		tmp.addData(tuple[i]);

	RM.insertRecord( tablename, tmp);
	return 1;
}


//delete只允许一条删除条件
int API::Delete(std::string tablename, SelectCondition scondition)
{
	int result = 0;
	Table re;

	if (scondition.amount == 0)
		result = RM.deleteRecord(tablename);
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

		result = RM.deleteRecord(tablename, scondition.attr[0], curwhere);

		//for (int i = 1; i < scondition.amount; i++)
			//re = ReMove(result, scondition.attr[i], scondition.operationtype[i], scondition.key[i]);

	}

	return result;
}
Table API::Select(std::string tablename, std::vector<std::string> attr, SelectCondition scondition)
{
	Table result;
	Attribute cur_attr = CL.GetTableAttribute(tablename);
	if (attr.size == cur_attr.amount)//选择整张表
		return RM.selectRecord(tablename);
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

		result = RM.selectRecord( tablename, scondition.attr[0], curwhere );

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

	i = 0;
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

//第一个属性必须是主键
bool Datacompare(const Tuple &tuple1, const Tuple &tuple2)
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