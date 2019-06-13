#define _CRT_SECURE_NO_WARNINGS
//	catalogmanager.cpp
//	目录管理


#include "catalogmanager.h"
#include <cassert>

std::string CatalogManager::TABLE_PATH = "./database/catalog/CatalogFile";

//将数字字符串转化为数字 
int CatalogManager::String2Num(const std::string &tmp)
{
	return atoi(tmp.c_str());
}

//将数字转化为字符串 
std::string CatalogManager::Num2String(int tmp)
{
	std::string result = "";
	if (tmp < 0)
	{
		tmp = -tmp;
		result += "-";
	}
	
	std::stringstream ss;
	ss << tmp;
	result += ss.str();
	return result;
}

//得到存放某表格信息的块数 -----------------------------------------
//块数至少为1 
int CatalogManager::getBlockNum(const std::string &table_name)
{
	char* pagecontent;
	int block_num = -1;
	do {
		pagecontent = buffer_manager.getPage(table_name, block_num + 1);
		block_num++;
	} while (pagecontent[0] != '\0');
	return block_num;
}

std::string CatalogManager::getTableName(char* buffer, int start, int end)
{
	int tmp;
	while(start <= end && buffer[start] != ' ')
		start++;
	tmp = start+1;
	while(tmp <= end && buffer[tmp] != ' ')
		tmp++;
	assert(start < tmp);
	return std::string(buffer+start+1, tmp-start-1);
}


//关于表格的操作 

//创建表格
//输入：表格名称、表格属性、索引对象、主码 
void CatalogManager::CreateTable(const std::string &table_name, Attribute attr, Index indices, int primary_key)
{
	if(!is_file_exist(TABLE_PATH))
	{
		std::fstream file;
		file.open(TABLE_PATH, std::ios::out);
	}
	// 检测是否有同名表的存在 
	if(isTableExist(table_name))
	{
		throw minisql_exception("Table " + table_name + " already exists!");
	}
	//把所有信息保存到字符串中用于输出到文件中
	//格式,由于用流可以一个单词一个单词的读入，所有这里一般信息用空格隔开 
	//@@ tablename attribute_number attrbute_name type is_unique(按顺序) primarykeynumber
	// index_number index_name towhatattribute#
	std::string outputstr = "@ ";
	outputstr += table_name;
	outputstr += (" " + Num2String(attr.amount));

	std::string TorF;
	for (int i = 0; i < attr.amount; i++)
	{
		if (attr.is_unique[i])
			TorF = " true";
		else
			TorF = " false";

		outputstr += (" " + attr.attr_name[i] + " " + Num2String(attr.attr_type[i]) + TorF);
	}

	outputstr += (" " + Num2String(attr.primary_key));

	outputstr += (" " + Num2String(indices.amount));
	for (int i = 0; i < indices.amount; i++)
		outputstr += (" " + indices.name[i] + " " + Num2String(indices.whose[i]));
	outputstr += " #";
	//还涉及到存入块的问题?? 
		//计算每条信息的长度
		//计算所用的块数
		//遍历所有的块寻找合适的位置，如果之前的块不够用，清出一块/新建一块插入
	int BlockNum = getBlockNum(TABLE_PATH);
	for (int i = 0; i < BlockNum; i++)
	{
		char* buffer = buffer_manager.getPage(TABLE_PATH, i);
		int PID = buffer_manager.getPageId(TABLE_PATH, i);
		bool flag = false;
		auto start = 0;
		auto size = 0;
		for(auto j = 0; j < _PAGESIZE; j++)
		{
			if(flag)
			{
				if(buffer[j] == '@')
				{
					flag = false;
					if(size >= outputstr.size())
					{
						memcpy(buffer+start, outputstr.c_str(), outputstr.size());
						buffer_manager.modifyPage(PID);
						return;
					}
				}
				else
				{
					size++;
					if(j+1 >= _PAGESIZE)
					{
						flag = false;
						if(size >= outputstr.size())
						{
							memcpy(buffer+start, outputstr.c_str(), outputstr.size());
							buffer_manager.modifyPage(PID);
							return;
						}
					}
				}
			}
			else
			{
				if(buffer[j] == '#')
				{
					flag = true;
					size = 0;
					if(j == 0)
					{
						start = j;
						size++;
					}
					else
					{
						start = j+1;
					}
				}
			}
		}
	}
	char* buffer = buffer_manager.getPage(TABLE_PATH, BlockNum);
	int PID = buffer_manager.getPageId(TABLE_PATH, BlockNum);
	strcat(buffer, outputstr.c_str());
	buffer_manager.modifyPage(PID);
	return;
}

bool CatalogManager::GetTablePlace(const std::string &table_name, int &block_id, int& start, int& end)
{
	int block_num = getBlockNum(TABLE_PATH);
	//遍历所有的块
	for (int current_block = 0; current_block < block_num; current_block++) {
		char* buffer = buffer_manager.getPage(TABLE_PATH, current_block);

		auto pos_start = 0;
		auto pos_end = 0;
		do {
			while(pos_start < _PAGESIZE && buffer[pos_start] != '@')
				pos_start++;
			if(pos_start >= _PAGESIZE)
				break;
			pos_end = pos_start+1;
			while(pos_end < _PAGESIZE && buffer[pos_end] != '#')
				pos_end++;
			if(pos_end >= _PAGESIZE)
				break;
			//得到table的名字，如果与输入的名字相同，则return true
			if (getTableName(buffer, pos_start+1, pos_end) == table_name)
			{
				start = pos_start;
				end = pos_end;
				block_id = current_block;
				return true;
			}
			pos_start = pos_end;
		} while (pos_start < _PAGESIZE);  //判断是否到头
	}
	return false;
}

//删除表格
//输入：表格名称 
void CatalogManager::DropTable(const std::string &table_name)
{
	auto block_id = 0, start = 0, end = 0;
	if(!GetTablePlace(table_name, block_id, start, end))
	{
		throw minisql_exception("Table " + table_name + " not exists!");
	}
	//刷新页面 
	char* buffer = buffer_manager.getPage(TABLE_PATH, block_id);
	auto PID = buffer_manager.getPageId(TABLE_PATH, block_id);
	//抹掉数据
	for(auto i = start; i <= end; i++)
	{
		if(i ==0)
			buffer[i] = '#';
		else
		{
			buffer[i] = '\0';
		}
	}
	buffer_manager.modifyPage(PID);
}

//通过表名查看表是否存在	
//输入：表格名称
//输出：true-存在； false-不存在
bool CatalogManager::isTableExist(const std::string &table_name)
{
	int start = 0, end = 0, block_id = 0;
	return GetTablePlace(table_name, block_id, start, end);
}

Attribute CatalogManager::GetTableAttribute(const std::string &table_name)
{
	Attribute result;
	auto block_id = 0, start = 0, end = 0;
	if(!GetTablePlace(table_name, block_id, start, end))
	{
		throw minisql_exception("Table " + table_name + " not exists!");
	}
	char* buffer = buffer_manager.getPage(TABLE_PATH, block_id);
	std::string attr(buffer+start+2, end - start - 3);
	std::istringstream info = std::istringstream(attr);
	std::string table;
	info >> table;
	std::string singleword;
	//属性数量 
	info >> singleword;
	result.amount = String2Num(singleword);
	//各属性信息
	for (int i = 0; i < result.amount; i++)
	{
		//属性
		info >> singleword;
		result.attr_name[i] = std::string(singleword);
		//类型
		info >> singleword;
		result.attr_type[i] = String2Num(singleword);
		//是否唯一
		info >> singleword;
		if (singleword == "true")
			result.is_unique[i] = true;
		else
			result.is_unique[i] = false;
	}
	//主码 
	info >> singleword;
	result.primary_key = String2Num(singleword);

	return result;
}

//关于属性和索引

//某一属性是否存在,必须在表存在时才能使用 
//输入：表格名称、属性名称
//输出：位置-存在； -1 - 不存在	 
int CatalogManager::isAttributeExist(const std::string &table_name, const std::string &attr)
{
	Attribute tmp_attr = GetTableAttribute(table_name);
	for (int i = 0; i < tmp_attr.amount; i++)
	{
		if (tmp_attr.attr_name[i] == attr)
			return i;
	}
	return -1;
}

//打印表格信息  ??待定，不知道查询结果的反馈方式 
void CatalogManager::PrintTable(const std::string &table_name)
{
	if(!isTableExist(table_name))
	{
		throw minisql_exception("Table " + table_name + " not exists!");
		return;
	}

	//打印表的信息
	std::cout << "------------------" << table_name << "------------------" << std::endl;
	// //打印属性信息
	int namelength = 0;
	Attribute tmp_attr = GetTableAttribute(table_name);
	std::cout << ">>>Primary key:  ";
	for (int i = 0; i < tmp_attr.amount; i++)
	{
		if (tmp_attr.attr_name[i].size() > namelength)
			namelength = tmp_attr.attr_name[i].size();
		if (i == tmp_attr.primary_key)
			std::cout << tmp_attr.attr_name[i] << std::endl;
	}

	std::cout << ">>>Attribute   " << "number:" << tmp_attr.amount << '\n';
	std::cout << std::left << std::setw(namelength+3) << "Name" << std::left << std::setw(12) << "| Type" << "| is unique?" << std::endl;
	for (int i = 0; i < tmp_attr.amount; i++)
	{
		//属性名
		std::string type;
		std::cout << std::left << std::setw(namelength+3) << tmp_attr.attr_name[i];
		//属性类型
		switch (tmp_attr.attr_type[i])
		{
			case 0:
			{
				std::cout << std::left << std::setw(15) << "float";
				break;
			}
			case -1:
			{
				std::cout << std::left << std::setw(15) << "int";
				break;
			}
			default:
			{
				type = "char(";
				type.append(Num2String(tmp_attr.attr_type[i]));
				type.append(")");
				std::cout << std::left << std::setw(15) << type;
				break;
			}
		}
		//是否唯一 
		if (tmp_attr.is_unique[i])
		{
			std::cout << "T" << std::endl;
		}
		else
		{
			std::cout << "F" << std::endl;
		}
	}

	//打印索引
	Index tmp_ind = GetTableIndex(table_name);
	std::cout << ">>>Index   number:" << tmp_ind.amount << std::endl;
	std::cout << std::left << std::setw(namelength + 3) << "Name" << std::left << std::setw(10) << "| Attribute" << std::endl;
	for (int i = 0; i < tmp_ind.amount; i++)
	{
		//索引名
		std::cout << std::left << std::setw(namelength + 3) << tmp_ind.name[i] << '\t';
		//索引对应属性
		std::cout << std::left << std::setw(10) << tmp_attr.attr_name[tmp_ind.whose[i]] << std::endl;
	}
}

//在指定属性上建立索引
//输入：表格名称、属性名称、索引名称
void CatalogManager::CreateIndex(const std::string &table_name, const std::string &attr, const std::string &index_name)
{
	auto attr_pos = isAttributeExist(table_name, attr);
	Index cur_index = GetTableIndex(table_name);
	Attribute cur_attr = GetTableAttribute(table_name);

	// 超过10个
	if(cur_index.amount == 10)
	{
		throw minisql_exception("Too many index on table " + table_name);
	}

	for(auto i = 0; i < cur_index.amount; i++)
	{
		if(cur_index.whose[i] == attr_pos)
		{
			throw minisql_exception("Attribute " + attr + " on table " + table_name + " already has index!");
		}
	}

	//检查无误，正式开始添加索引
	cur_index.amount++;
	cur_index.name[cur_index.amount - 1] = index_name;
	cur_index.whose[cur_index.amount - 1] = attr_pos;

	//由于原来的表已经计入，不能肯定它之后是否有其他信息，所以需要整个表删掉重新添加 
	try
	{
		DropTable(table_name);
		CreateTable(table_name, cur_attr, cur_index, cur_attr.primary_key);
	}
	catch(minisql_exception &e)
	{
		e.add_msg("Create index " + index_name + " error!");
		throw e;
	}
}

//删除索引
//输入：表格名称、索引名称
void CatalogManager::DropIndex(const std::string &table_name, const std::string &index_name)
{
	Index cur_index = GetTableIndex(table_name);
	Attribute cur_attr = GetTableAttribute(table_name);
	auto index_pos = isIndexExist(table_name, index_name);
	//检查无误，正式开始删除索引
	if(index_pos != cur_index.amount-1)
	{
		cur_index.name[index_pos] = cur_index.name[cur_index.amount-1];
		cur_index.whose[index_pos] = cur_index.whose[cur_index.amount-1];
	}
	cur_index.amount--;

	//由于原来的表已经计入，不能肯定它之后是否有其他信息，所以需要整个表删掉重新添加 
	try
	{
		DropTable(table_name);
		CreateTable(table_name, cur_attr, cur_index, cur_attr.primary_key);
	}
	catch(minisql_exception &e)
	{
		e.add_msg("Delete index " + index_name + " error!");
		throw e;
	}
}

//索引是否存在
//输入：表格名称、索引名称
//输出：正整数-索引序号； -1-不存在		 

int CatalogManager::isIndexExist(const std::string &table_name, const std::string &index_name)
{
	Index cur_index = GetTableIndex(table_name);

	for (int i = 0; i < cur_index.amount; i++)
		if (cur_index.name[i] == index_name)
			return i;

	return -1;
}

//得到某表的全部索引,必须在表存在时才可以用 
//输入：表格名称
//输出：Index结构数据
Index CatalogManager::GetTableIndex(const std::string &table_name)
{
	Index result;
	auto block_id = 0, start = 0, end = 0;
	if(!GetTablePlace(table_name, block_id, start, end))
	{
		throw minisql_exception("Table " + table_name + " not exists!");
	}
	char* buffer = buffer_manager.getPage(TABLE_PATH, block_id);
	std::string attr(buffer+start+2, end - start - 3);
	std::istringstream info = std::istringstream(attr);
	std::string table;
	info >> table;
	std::string singleword;
	//属性数量 
	info >> singleword;
	auto amount = String2Num(singleword);
	//各属性信息
	for (auto i = 0; i < amount; i++)
	{
		//属性
		info >> singleword;
		//类型
		info >> singleword;
		//是否唯一
		info >> singleword;
	}
	//主码 
	info >> singleword;
	// index数量
	info >> singleword;
	result.amount = String2Num(singleword);
	for(auto i = 0; i < result.amount; i++)
	{
		// 索引名字
		info >> singleword;
		result.name[i] = std::string(singleword);
		// 索引对应属性
		info >> singleword;
		result.whose[i] = String2Num(singleword);
	}
	return result;
}

