//
//	catalogmanager.cpp
//	目录管理

#include "catalogmanager.h"

//将数字字符串转化为数字 
int Catolog::String2Num(std::string tmp)
{
	return atoi(tmp.c_str());
}

//将数字转化为字符串 
std::string Catolog::Num2String(int tmp)
{
	std::string result = "";
	if( tmp < 0 )
	{
		tmp = -tmp;
		result += "-";
	}
	
	stringstream ss;
	ss << tmp;
	result = ss.str();
	return result;
} 

//得到存放某表格信息的块数 -----------------------------------------
//块数至少为0 
int Catolog::GetBlockAmount(std::string tablename)
{
	char *pagecontent;
	int num = 0;
	
	pagecontent = //得到buffer_manager中一块的所有内容,需使用到表名和num？ 
	while( pagecontent[0] != '\0' )
	{
		num++;
		pagecontent = //得到buffer_manager中一块的所有内容,需使用到表名和num？ 
	}
	
	return num; 
}


//关于表格的操作 
		
//创建表格
//输入：表格名称、表格属性、索引对象、主码 
//输出: 1-成功； 0-失败,包含异常 
int Catolog::CreateTable(std::string tablename, Attribute attr, Index indices, int primary_key)
{
	//检测是否有同名表的存在 
	if( isTableExist(tablename) == true )
	{
		std::cout << "已存在该表，输入错误" << std::endl;
		return 0; 
	} 
	
	//把所有信息保存到字符串中用于输出到文件中
	//格式,由于用流可以一个单词一个单词的读入，所有这里一般信息用空格隔开 
	//@@ tablename attribute_number attrbute_name type is_unique(按顺序) primarykeynumber
	// index_number index_name towhatattribute
	//\n
	std::string outputstr = "@@ ";
	outputstr += tablename;
	output += (" " + Num2String(attr.amount));
	
	std::string TorF;
	for(int i = 0; i < attr.amount; i++)
	{
		if(attr.is_unique)
			TorF = "true";
		else
			TorF = "false";
		
		outputstr += ( " " + attr.attr_name[i] + Num2String(attr.attr_type[i]) + TorF );		
	}
	
	outputstr += ( " " + Num2String(attr.primary_key) );
	
	outputstr += ( " " + Num2String(indices.amount) );
	for(int i = 0; i < attr.amount; i++)
		outputstr += ( " " + indices.name[i] + Num2String(indices.whose[i]) );		
	outputstr += "\n";
	
	//还涉及到存入块的问题??
		//计算每条信息的长度
		//计算所用的块数
		//遍历所有的块寻找合适的位置，如果之前的块不够用，清出一块/新建一块插入 
	int BlockNum = GetBlockAmount(TABLE_PATH) / _PAGESIZE;
	if (!BlockNum)
	{
		BlockNum = 1;
	}
	for (int i = 0; i < BlockNum; i++)
	{
		char* buffer = buffer_manager.getPage(TABLE_PATH, i);
		int PID = buffer_manager.getPageId(TABLE_PATH, i);

		int size;
		for (size = 0; size < _PAGESIZE && buffer[size] != '\0' && buffer[size] != '#'; size++);
	

		if (size + outputstr.length() < _PAGESIZE)
		{
			if (size != 0 && buffer[size - 1] == '#')
			{
				buffer[size - 1] = '\0';
			}
			else if (buffer[size] == '#')
			{
				buffer[size] = '\0';
			}
			strcat(buffer, outputstr.c_str());

			buffer_manager.modifyPage(PID);
			return 1;
		}
	}
	char* buffer = buffer_manager.getPage(TABLE_PATH, BlockNum);
	int PID = buffer_manager.getPageId(TABLE_PATH, BlockNum);
	strcat(buffer, outputstr.c_str());
	buffer_manager.modifyPage(PID);
	return 1;
}
 

//删除表格
//输入：表格名称
//输出：1-成功； 0-失败,包含异常  
int Catolog::DropTable(std::string tablename)
{
	if( isTableExist(tablename) == false )
	{
		std::cout << "不存在该表，输入错误" << std::endl;
		return 0; 
	} 
	//找到相应的块
	int block;
	int begin = GetTablePlace(tablename, block);

	char* buffer = buffer_manager.getPage(TABLE_PATH, block);
	int PID = buffer_manager.getPageId(TABLE_PATH, block);

	std::string check = buffer;

	//删除对应的信息，包括表的属性和索引信息 
	int end = start + String2Num(check.substr(begin, 4);
	int index = 0;
	int current_index = 0;
	if (index < begin || index >= end)
	{
		buffer[current_index] = buffer[index];
		current_index++;
	}
	index++;
	while (buffer[index] != '#')
	{
		if (index < begin || index >= end)
		{
			buffer[current_index] = buffer[index];
			current_index++;
		}
		index++;
	}
	buffer[current_index] = '#';
	buffer[++current_index] = '\0';
	
	//刷新页面 
	buffer_manager.modifyPage(PID);

	return 1;
} 
 
//通过表名查看表是否存在	
//输入：表格名称
//输出：true-存在； false-不存在
bool Catolog::isTableExist(std::string tablename)
{
	//遍历所有的块，通过@@开头分辨表的信息
		//如果存在相同的表明，就返回true

	int block_num = GetBlockAmount(TABLE_PATH) / PAGESIZE;
	if (block_num <= 0)
		block_num = 1;
	//遍历所有的块
	for (int current_block = 0; current_block < block_num; current_block++) {
		char* buffer = buffer_manager.getPage(TABLE_PATH, current_block);
		std::string buffer_check(buffer);
		std::string str_tmp = "";
		int start_index = 0, end_index = 0;
		do {
			//如果一开始就是#，则检查下一块
			if (buffer_check[0] == '#')
				break;
			//得到table的名字，如果与输入的名字相同，则return true
			else if (getTableName(buffer, start_index, end_index) == table_name) {
				return true;
			}
			else {
				//通过字符串长度来重新确定下一个table的位置
				start_index += String2Num(buffer_check.substr(start_index, 4));
				//排除空文档的特殊条件
				if (!start_index)
					break;
			}
		} while (buffer_check[start_index] != '#');  //判断是否到头
	}
	return false;
} 

//打印表格信息  ??待定，不知道查询结果的反馈方式 
void Catolog::PrintTable(std::string tablename, Attribute tattr)
{
	if( isTableExist( tablename ) == false )
	{
		std::cout << "不存在该表，输入错误" << std::endl;
		return; 
	}
	
	//打印表的信息
	std::cout << "------------------" << tablename << "------------------" << std::endl;
	//打印属性信息
	int namelength = 0;
	Attribute tmp_attr = GetTableAttribute(tablename); 
	std::cout << ">>>Primary key:  "; 
	
	for( int i = 0; i < tmp_attr.amount; i++ )
	{
		if( tmp_attr.attr_name[i].length() > namelength )
			namelength = tmp_attr.attr_name[i].length();
		if( i == tmp_attr.primary_key )
			cout << tmp_attr.attr_name[i] << endl;
	}
	
	std::cout << ">>>Attribute   " << "number:" << tmp_attr.amount << std::endl;
	std::cout << left << setw(namelength + 3) << "Name"  << left << setw(12) << "| Type"  << "| is unique?" << std::endl;
	for( int i = 0; i < tmp_attr.amount; i++ )
	{
		//属性名
		std::cout << left << setw( namelength + 3 ) << tmp_attr.attr_name[i];
		//属性类型
		switch( tmp_attr.attr_type[i] )
		{
			case 0:
			{
				std::cout << left << setw( 10 ) << "float";
				break;
			}
			case -1:
			{
				std::cout << left << setw( 10 ) << "int";
				break;
			}
			default:
			{
				std::cout << "char("<< left << setw( 3 ) << tmp_attr.attr_type[i] << ")";
				break;
			}
		}
		//是否唯一 
		if( tmp_attr.is_unique[i] == true )
			std::cout << " T" << std::endl;
		else
			std::cout << " F" << std::endl;
	}
	
	//打印索引
	Index tmp_ind = GetTableIndex(tablename);
	std::cout << ">>>Index   number:" << tmp_ind.amount << std::endl; 
	std::cout << left << setw(namelength + 3) << "Name"  << "| Attribute" << std::endl;
	for( int i = 0; i < tmp_ind.amount; i++ )
	{
		//索引名
		std::cout << left << setw( namelength + 3 ) << tmp_ind.attr_name[i];
		//索引对应属性
		std::cout << tmp_attr.attr_name[tmp_ind.whose[i]] << std::endl;
	}
} 	
 


//关于属性和索引

//某一属性是否存在,必须在表存在时才能使用 
//输入：表格名称、属性名称
//输出：位置-存在； -1 - 不存在	 
int Catolog::isAttributeExist(std::string tablename, std::string tattr)
{
	Attribute tmp_attr = GetAttributeMessage( tablename );
	for( i = 0; i < tmp_attr.amount; i++ )
	{
		if( tmp_attr.attr_name[i] == tattr )
			return i;
	}
	return -1;
}

//得到某表的全部属性
//输入：表格名称
//输出：Attribute结构数据
Attribute Catolog::GetTableAttribute(std::string tablename)
{
	std::string tattr = "";
	//找到表格的在那个块中
	int block;
	int start = GetTablePlace(tablename, block);


	//读取整块信息
	char* buffer = buffer_manager.getPage(TABLE_PATH, block);
	std::string check(buffer);
	
	//得到整个表的信息
	int end = 0;
	std::string attr_name = getTableName(check, start, end);
	//得到attribute部分的信息，存入字符串tattr 
	
	Attribute result;

	std::istringstream instruction = istringstream(tattr);
	std::string singleword;
	//属性数量 
	instruction >> singleword;
	result.amount = String2Num(singleword);
	//各属性信息
	for( int i = 0; i < result.amount; i++ )
	{
		//属性
		instruction >> singleword;
		result.attr_name[i] = std::string(singleword); 
		//类型
		instruction >> singleword;
		result.attr_type[i] = String2Num( singleword );
		//是否唯一
		instruction >> singleword;
		if( singleword == "true" )
			result.is_unique[i] = true;
		else
			result.is_unique[i] = false;
	} 
	//主码 
	instruction >> singleword;
	result.primary_key = String2Num( singleword );

	return result;
}

//在指定属性上建立索引
//输入：表格名称、属性名称、索引名称
//输出：1-成功； 0-失败,包含异常
int Catolog::CreateIndex(std::string tablename, std::string tattr, std::string indexname)
{
	if( isTableExist( tablename ) == false )
	{
		cout << "不存在该表，输入错误" << endl;
		return 0; 
	}
	if( isAttributeExist( tablename, tattr) == -1 )
	{
		std::cout << "不存在该属性，输入错误" << std::endl;
		return 0; 
	}
	
	//判断是否越界或者重复 
	Index cur_index = GetTableIndex(tablename);
	Attribute cur_attr = GetTableAttribute(tablename); 
	int numberofattr = 0;
	
	if( cur_index.amount == 10 )
	{
		std::cout << "当前表索引数量已达到上限，操作无效" << std::endl;
		return 0;		
	} 

	int flag = 0;
	for( int i = 0; i < cur_attr.amount; i++ )
	{
		if( cur_attr.attr_name[i] == tattr )
		{
			numberofattr = i;
			break;
		}
	} 
	
	for( int i = 0; i < cur_index.amount; i++ )
	{
		if( cur_index.whose[i] == numberofattr )
		{
			std::cout << "当前属性已存在索引，操作无效" << std::endl;
			return 0;			
		}
		if( cur_index.name[i] == indexname )
		{
			std::cout << "当前索引名已被使用，操作无效" << std::endl;
			return 0;			
		}
	} 
	
	//检查无误，正式开始添加索引
	cur_index.amount++;
	cur_index.attr_name[amount-1] = indexname;
	cur_index.whose[amount-1] = numberofattr;
	
	//由于原来的表已经计入，不能肯定它之后是否有其他信息，所以需要整个表删掉重新添加 
	if( DropTable(tablename) == 0 )
	{
		std::cout << "更新信息失败，无法删除原表，操作无效" << std::endl;
		return 0		
	}
	if(CreateTable(tablename, cur_attr, cur_indexs, cur_attr.primary_key) == 0)
	{
		std::cout << "更新信息失败，无法插入表，操作无效" << std::endl;
		return 0		
	}
	
	return 1; 
}

//删除索引
//输入：表格名称、索引名称
//输出：1-成功； 0-失败,包含异常
int Catolog::DropIndex(std::string tablename, std::string indexname)
{
	//类似于插入索引操作就几个细节改一下 
	if( isTableExist( tablename ) == false )
	{
		std::cout << "不存在该表，输入错误" << std::endl;
		return 0; 
	}
	
	//判断是否越界或者重复 
	Index cur_index = GetTableIndex(tablename);
	Attribute cur_attr = GetTableAttribute(tablename); 
	int numberofindex = 0; 
	 	
	numberofindex = isIndexExist(tablename, indexname);	  
	if( numberofindex == 0 )
	{
		std::cout << "当前表没有该索引，操作无效" << std::endl;
		return 0;		
	}

	//检查无误，正式开始删除索引
	cur_index.amount--;
	if( i != amount )
	{
		cur_index.name[i] = cur_index.name[amount];
		cur_index.whose[i] = cur_index.whose[amount];		
	}
	
	//由于原来的表已经计入，不能肯定它之后是否有其他信息，所以需要整个表删掉重新添加 
	if( DropTable(tablename) == 0 )
	{
		std::cout << "删除索引失败，无法删除原表，操作无效" << std::endl;
		return 0		
	}
	if(CreateTable(tablename, cur_attr, cur_index, cur_attr.primary_key) == 0)
	{
		std::cout << "删除索引失败，无法插入表，操作无效" << std::endl;
		return 0		
	}
	
	return 1; 
}

//索引是否存在
//输入：表格名称、索引名称
//输出：正整数-索引序号； 0-不存在		 
int Catolog::isIndexExist(std::string tablename, std::string indexname)
{
	Index cur_index = GetTableIndex(tablename);

	for( int i = 0; i < cur_index.amount; i++ )
		if( cur_index.attr_name[i] == indexname )
			return i; 
	
	return 0;
}

//得到某表的全部索引,必须在表存在时才可以用 
//输入：表格名称
//输出：Index结构数据
Index GetTableIndex(std::string tablename)
{
	Index result;
	std::string sindex;
	//得到索引所在表的位置
	//得到索引信息
	//把信息一个一个录入结构中
	
	
	return result; 
}

//得到单词
std::string GetWord( std::string input )
{
	std::string word = "";
	std::istringstream instruction = std::istringstream(input);
	instruction >> word;
	return word;
} 

