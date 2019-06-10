//
//interpreter.cpp
//operation here: 0- =	1- <>	2- <	3- >	4- <=	5- >=
//

#include "interpreter.h"
#include <iostream>
#include <iomanip>

Interpreter::Interpreter(API &api, CatalogManager &cm)
	: api(api), catalog_manager(cm), readmode(0)
{}

void Interpreter::GetInput(std::string &input)
{
	instruction = std::istringstream(input);
}

int Interpreter::GetInstruction()
{
	std::string input;
	if(readmode==0)
		getline(std::cin, input);
	else
		if(!file.eof())
			getline(file, input);
		else
			return 0; 
			
	instruction = std::istringstream(input);
	return 1;
}

std::string Interpreter::GetWord()
{
	std::string word;
	instruction >> word;
	return word;
}

//输入：无
//输出：1-操作成功，会执行相应的结果； 0-失败； 2-退出 
bool Interpreter::JudgeAndExec()
{
	std::string singleword = GetWord();

	if(singleword == "quit")
		return true;
	else if (singleword == "create")
	{
		singleword = GetWord();
		if (singleword == "table")//新建表操作
		{
			if(ExecCreateTable() == 0)
			{
				std::cout << "create table failed!" << std::endl;
			}
			return false;
		}
		else if (singleword == "index")//新建索引操作
		{
			if(ExecCreateIndex() == 0)
			{
				std::cout << "create index failed!" << std::endl;
			}
			return false;
		}
		else//输入错误
		{
			std::cout << "syntax error!" << std::endl; 
			return false; 
		}
	}
	else if (singleword == "drop")
	{
		singleword = GetWord();
		if (singleword == "table")//删除表操作
		{
			if(ExecDropTable() == 0)
			{
				std::cout << "delete table failed!" << std::endl;
			}
			return false;
		}
		else if (singleword == "index")//删除索引操作
		{
			if(ExecDropIndex() == 0)
			{
				std::cout << "delete index failed!" << std::endl;
			}
			return false;
		}
		else//输入错误
		{
			std::cout << "syntax error!" << std::endl; 
		}
		return false; 			
	}
	else if (singleword == "select")//搜索操作
	{
		int result = ExecSelect();
		if (result == 0)
		{
			std::cout << "select failed!" << std::endl;
		}
		else if (result == -1)
		{
			std::cout << "empty returned record!" << std::endl;
		}
		return false;
	}
	else if (singleword == "insert")//插入操作
	{
		if(ExecInsert() == 0)
		{
			std::cout << "insert failed!" << std::endl;
		}
		return false;
	}
	else if (singleword == "delete")//删除元组操作
	{
		if(ExecDelete() == 0)
		{
			std::cout << "delete failed!" << std::endl;
		}
		return false;
	}
	else if (singleword == "execfile")//执行文件内容操作
	{
		ExecFile();
		return false;
		// 遇到quit会怎么样
	}
	else//输入错误
	{
		std::cout << "syntax error!" << std::endl; 
		return false; 		
	}
}

//具体操作函数 
//判断语句是否正确，给出错误原因/拆解调用执行函数

//此时从create table后面开始读 
int Interpreter::ExecCreateTable()
{
	std::string nameoftable;
	std::string cur_word;
	Attribute cur_attr;
	int flag = 0, primarykey;

	nameoftable = GetWord();
	if(catalog_manager.isTableExist(nameoftable) == 1)
	{
		std::cout << "table already exists!" << std::endl;
		return 0;		
	}
	
	if(GetWord() != "(")
	{
		std::cout << "syntax error!" << std::endl;
		return 0;
	}
	cur_attr.amount = 0;
	if (GetInstruction() == 0)
	{
		std::cout << "read error!" << std::endl;
		return 0;
	}
	cur_word = GetWord();
	while (cur_word != ");")
	{
		if (cur_word == "primary")
		{
			cur_word = GetWord();
			if (cur_word == "key")
			{
				cur_word = GetWord();
				if (cur_word != "(")
				{
					std::cout << "syntax error!" << std::endl;
					return 0;
				}

				cur_word = GetWord();
				int i;
				for (i = 0; i < cur_attr.amount; i++)
				{
					if (cur_word == cur_attr.attr_name[i])
						break;
				}
				if (i < cur_attr.amount)
					cur_attr.primary_key = i;
				else
				{
					std::cout << "no such attribute!" << std::endl;
					return 0;
				}

				cur_word = GetWord();
				if (cur_word != ")")
				{
					std::cout << "syntax error!" << std::endl;
					return 0;
				}
			}
			else
			{
				std::cout << "syntax error!" << std::endl;
				return 0;
			}
		}
		else
		{
			std::string attrname, attrtype;
			attrname = cur_word;
			attrtype = GetWord();


			if (attrtype == "int")
			{
				cur_word = GetWord();
				if (cur_word == "unique" || cur_word == ",")
				{
					if (cur_word == "unique" && GetWord() != ",")
					{
						std::cout << "syntax error!" << std::endl;
						return 0;
					}

					cur_attr.attr_name[cur_attr.amount] = attrname;
					cur_attr.attr_type[cur_attr.amount] = -1;
					if (cur_word == "unique")	cur_attr.is_unique[cur_attr.amount] = true;
					cur_attr.amount++;
				}
				else
				{
					std::cout << "syntax error!" << std::endl;
					return 0;
				}
			}
			else if (attrtype == "float")
			{
				cur_word = GetWord();
				if (cur_word == "unique" || cur_word == ",")
				{
					if (cur_word == "unique" && GetWord() != ",")
					{
						std::cout << "syntax error!" << std::endl;
						return 0;
					}

					cur_attr.attr_name[cur_attr.amount] = attrname;
					cur_attr.attr_type[cur_attr.amount] = 0;
					if (cur_word == "unique")	cur_attr.is_unique[cur_attr.amount] = true;
					cur_attr.amount++;
				}
				else
				{
					std::cout << "syntax error!" << std::endl;
					return 0;
				}
			}
			else if (attrtype.substr(0, 4) == "char")
			{
				attrtype.erase(0, 4);
				int len = 0, pos = 1;
				if (attrtype[0] == '(')
					while (attrtype[pos] != ')')
					{
						len = 10 * len + attrtype[pos] - '0';
						if (len > 255 || len == 0 || pos >= 4)
							return 0;
						pos++;
					}

				cur_word = GetWord();
				if (cur_word == "unique" || cur_word == ",")
				{
					if (cur_word == "unique" && GetWord() != ",")
					{
						std::cout << "syntax error!" << std::endl;
						return 0;
					}

					cur_attr.attr_name[cur_attr.amount] = attrname;
					cur_attr.attr_type[cur_attr.amount] = len;
					if (cur_word == "unique")	cur_attr.is_unique[cur_attr.amount] = true;
					cur_attr.amount++;
				}
				else
				{
					std::cout << "syntax error!" << std::endl;
					return 0;
				}
			}
			else
			{
				std::cout << "syntax error!" << std::endl;
				return 0;
			}

		}
		if (GetInstruction() == 0)
		{
			std::cout << "read error!" << std::endl;
			return 0;
		}
		cur_word = GetWord();
		flag = 1;
	}
	
	
	if( api.CreateTable(nameoftable, cur_attr) == 1 ) 
	{
		std::cout << "create table success!" << nameoftable << std::endl; 
		return 1;
	}
	else return 0;
}

//此时从drop table后面开始读 
int Interpreter::ExecDropTable()
{
	std::string nameoftable;	
	std::string cur_word;
	nameoftable	= GetWord();
	if( nameoftable[ nameoftable.size()-1 ] == ';' )
		nameoftable.erase(nameoftable.size()-1,1);
	
	if( api.DropTable(nameoftable) == 1 ) 
	{
		std::cout << "delete table success!" << nameoftable << std::endl; 
		return 1;
	}
	else return 0;
}

//此时从create index后面开始读 
int Interpreter::ExecCreateIndex()
{
	std::string indexname, tablename, tattr;

	indexname = GetWord();
	if( GetWord() != "on" )
	{
		std::cout << "syntax error!" << std::endl; 
		return 0; 
	}	
	tablename = GetWord();
	if( GetWord() != "(" )
	{
		std::cout << "syntax error!" << std::endl; 
		return 0; 
	}
	tattr = GetWord();
	if( GetWord() != ");" )
	{
		std::cout << "syntax error!" << std::endl; 
		return 0; 
	}
	
	//接下来检查该表该属性是否存在
	if ( catalog_manager.isTableExist(tablename) == 1 && catalog_manager.isAttributeExist(tablename, tattr) == true )
	{
		if( api.CreateIndex(tablename, tattr, indexname) == 1 ) 
		{
			std::cout << "insert index success!" << indexname << std::endl; 
			return 1;
		}
		else 
			return 0;	
	}
	else
	{
		std::cout << "attribute not exists!" << '\n';
		return 0;
	}
	
}

//语法有所修改，示例：drop index xxxx on tablename ; 
int Interpreter::ExecDropIndex()
{
	std::string indexname, tablename, tattr;

	indexname = GetWord();
	if( GetWord() != "on" )
	{
		std::cout << "syntax error!" << std::endl; 
		return 0; 
	}	
	tablename = GetWord();

	if (GetWord() != ";")
	{
		std::cout << "syntax error!" << std::endl;
		return 0;
	}
	//接下来检查该表该属性是否存在
	if ( catalog_manager.isTableExist(tablename) == 1 && catalog_manager.isIndexExist(tablename, indexname) == true )
	{
		if( api.DropIndex(tablename, indexname) == 1 ) 
		{
			std::cout << "delete index success!" << indexname << std::endl; 
			return 1;
		}
		else
		{
			return 0;
		} 	
	}
	else
	{
		std::cout << "attribute not exists!" << '\n';
		return 0;
	}
}

//operation here: 0- =	1- <>	2- <	3- >	4- <=	5- >=
int Interpreter::ExecSelect()
{
	std::string tablename;
	std::string curword;
	std::vector<std::string> targetattr;
	SelectCondition scondition;
	Attribute curattr; 
	int targetan = 0;
	int isAll = 0; 
	Table selectresult;	
	int isall = 0;

	scondition.amount = 0;
	curword = GetWord();
	if( curword == "*" )
	{
		curword = GetWord();
		isall = 1;
	}
	else
	{
		while( curword != "from" )
		{
			targetattr.push_back(curword);
			targetan++;
			curword = GetWord();
			if(curword == ",")
				curword = GetWord();
			
			if( targetan > 30 )
			{
				std::cout << "too many attributes!" << std::endl; 
				return 0;	
			}
		}
	}
	if( curword != "from" )
	{
		std::cout << "syntax error!" << std::endl; 
		return 0;	
	} 
	
	tablename = GetWord();
	//检验该表是否存在 
	if(catalog_manager.isTableExist(tablename) != 1)
	{
		std::cout << "table not exists!" << std::endl; 
		return 0; 		
	}
	//同时可以检验原来的那些属性是否存在
	for( int i = 0; i < targetan; i++ )
		if(catalog_manager.isAttributeExist(tablename, targetattr[targetan]) == -1)
		{
			std::cout << "attributes error!" << std::endl; 
			return 0; 		
		}
	curattr = catalog_manager.GetTableAttribute(tablename);
	
	curword = GetWord();
	
	//区分是否有查找条件 
	if( curword == ";" )
	{
		isAll = 1;//表示选择全部信息 
	} 
	else if( curword != "where" )
	{
		std::cout << "syntax error!" << std::endl; 
		return 0; 
	}
	
	//开始添加查找条件 
	if( curword == "where" )
	{
		while( curword != ";" )
		{
			int position;
			//属性
			// 
			curword =  GetWord();
			//属性是否存在 
			position = catalog_manager.isAttributeExist(tablename, curword);
			if( position == -1)
			{
				std::cout << "attributes error!" << std::endl; 
				return 0; 		
			}
			scondition.attr[scondition.amount] = curword;
			scondition.amount ++; 
			
			//条件
			//
			curword =  GetWord();
			if( curword == "=" )
				scondition.operationtype[scondition.amount-1] = 0;
			else if( curword == "<>" )
				scondition.operationtype[scondition.amount-1] = 1;
			else if( curword == "<" )
				scondition.operationtype[scondition.amount-1] = 2;
			else if( curword == ">" )
				scondition.operationtype[scondition.amount-1] = 3;
			else if( curword == "<=" )
				scondition.operationtype[scondition.amount-1] = 4;
			else if( curword == ">=" )
				scondition.operationtype[scondition.amount-1] = 5;
			else
			{
				std::cout << "syntax error!" << std::endl; 
				return 0;					
			}
			
			//关键值 
			// 
			curword =  GetWord();
			int type = curattr.attr_type[position];
			if( type == -1 )//int
			{
				scondition.key[scondition.amount-1].type = -1;
				scondition.key[scondition.amount-1].idata = atoi(curword.c_str());
			}
			else if( type == 0 )//float
			{
				scondition.key[scondition.amount-1].type = 0;
				scondition.key[scondition.amount-1].fdata = atoi(curword.c_str());
			}
			else if( type > 0 )//string
			{
				scondition.key[scondition.amount-1].type = type;
				curword = curword.erase(0,1);
				curword = curword.erase(curword.length()-1,1);
				scondition.key[scondition.amount-1].sdata = curword;	
			} 
			else
			{
				std::cout << "syntax error!" << std::endl; 
				return 0;					
			}			
			
			curword =  GetWord();
			//连接字符-and
			if( curword != "and" && curword != ";" )
			{
				std::cout << "syntax error!" << std::endl; 
				return 0;					
			}						 
		}	
	}
	
	if( isAll == 1 )	scondition.amount = 0;
	
	//正式调用API函数查找
	if(isall!=1)
		selectresult = api.Select( tablename, targetattr, scondition );
	else//select *
	{
		for (int i = 0; i < curattr.amount; i++)
			targetattr[i] = curattr.attr_name[i];

		selectresult = api.Select(tablename, targetattr, scondition );
	}
	
	//输出结果 
	//表名
	std::cout << "------------------" << selectresult.table_name << "------------------" << std::endl;
	//属性名 
	int namelength = 0;
	Attribute tmp_attr = selectresult.attr; 
	
	for( int i = 0; i < tmp_attr.amount; i++ )
	{
		if( tmp_attr.attr_name[i].length() > namelength )
			namelength = tmp_attr.attr_name[i].length();
	}
	
	for( int i = 0; i < tmp_attr.amount; i++ )
		std::cout << std::left << std::setw( namelength+5 ) << tmp_attr.attr_name[i] << '|';
	std::cout << std::endl;
	
	//元组 
	std::vector<Tuple>::iterator it;
	for( it = selectresult.tuples.begin(); it != selectresult.tuples.end(); it++ )
		it->Printdata();
	
	return 1;
} 

int Interpreter::ExecInsert()
{
	std::vector<Data> tuple;
	Attribute curattr;
	std::string tablename, curword;
	
	curword = GetWord();
	if( curword != "into" )
	{
		std::cout << "syntax error!" << std::endl; 
		return 0;
	}
	
	tablename = GetWord();
	//检验该表是否存在 
	if(catalog_manager.isTableExist(tablename) != 1)
	{
		std::cout << "table not exists!" << std::endl; 
		return 0; 		
	}
	
	curword = GetWord();
	if( curword != "values" )
	{
		std::cout << "syntax error!" << std::endl; 
		return 0;
	}

	curword = GetWord();
	if( curword != "(" )
	{
		std::cout << "syntax error!" << std::endl; 
		return 0;
	}	
	
	curword = GetWord();
	curattr = catalog_manager.GetTableAttribute(tablename);
	int number = 0;
	while(curword != ")" )
	{
		Data tmp;
		if( curword[0] == '\'' )
			curword.erase( 0, 1 );
		else
		{
			std::cout << "syntax error!" << std::endl; 
			return 0;
		} 
		
		if( curword[curword.length()-1] == '\'' )
			curword.erase( curword.length()-1, 1 );
		else
		{
			std::cout << "syntax error!" << std::endl; 
			return 0;
		} 
		
		tmp.type = curattr.attr_type[number];
		if( tmp.type == -1 )//int
			tmp.idata = atoi(curword.c_str());
		else if( tmp.type == 0 )//float
			tmp.fdata = atoi(curword.c_str());
		else if( tmp.type > 0 )//string
			tmp.sdata = curword;	
		else
		{
			std::cout << "syntax error!" << std::endl; 
			return 0;					
		}
		
		curword = GetWord();
		if( curword != "," && curword != ")" )
		{
			std::cout << "syntax error!" << std::endl; 
			return 0;			
		}
		else
		{
			tuple.push_back(tmp);
			number++;	
			if( curword != ")" )	curword = GetWord();
		}
	}
	
	if( GetWord() != ";" )
	{
		std::cout << "syntax error!" << std::endl;
		return 0;
	}
	
	if( api.Insert(tablename, tuple) == 1 ) 
	{
		std::cout << "insert table success!" << std::endl; 
		return 1;
	}
	else return 0;	
}

//一定要有where的条件，不然不知道删除什么元组 
int Interpreter::ExecDelete()
{
	std::string tablename;
	std::string curword;
	SelectCondition scondition;
	Attribute curattr; 
	int targetan = 0;

	scondition.amount = 0;
	curword = GetWord();
	if( curword != "from" )
	{
		std::cout << "syntax error!" << std::endl; 
		return 0;	
	} 
	
	tablename = GetWord();
	//检验该表是否存在 
	if(catalog_manager.isTableExist(tablename) != 1)
	{
		std::cout << "table not exists!" << std::endl; 
		return 0; 		
	}

	curattr = catalog_manager.GetTableAttribute(tablename);
	
	curword = GetWord();
	if(curword == ";")
	{
		//正式调用API函数删除 
		int deletetuplenum = 0; 
		deletetuplenum = api.Delete( tablename, scondition );

		if( deletetuplenum != -1 )
		{
			std::cout << "delete " << deletetuplenum << " success" << std::endl; 
			return 1;
		}	
		else
			return 0;		
	}
			
	//区分是否有查找条件 
	if( curword != "where" )
	{
		std::cout << "syntax error!" << std::endl; 
		return 0; 
	}
	
	//开始添加删除条件 
	if( curword == "where" )
	{
		while( curword != ";" )
		{
			int position;
			//属性
			// 
			curword =  GetWord();
			//属性是否存在 
			position = catalog_manager.isAttributeExist(tablename, curword);
			if( position == -1)
			{
				std::cout << "attributes error!" << std::endl; 
				return 0; 		
			}
			scondition.attr[scondition.amount] = curword;
			scondition.amount ++; 
			
			//条件
			//
			curword =  GetWord();
			if( curword == "=" )
				scondition.operationtype[scondition.amount-1] = 0;
			else if( curword == "<>" )
				scondition.operationtype[scondition.amount-1] = 1;
			else if( curword == "<" )
				scondition.operationtype[scondition.amount-1] = 2;
			else if( curword == ">" )
				scondition.operationtype[scondition.amount-1] = 3;
			else if( curword == "<=" )
				scondition.operationtype[scondition.amount-1] = 4;
			else if( curword == ">=" )
				scondition.operationtype[scondition.amount-1] = 5;
			else
			{
				std::cout << "syntax error!" << std::endl; 
				return 0;					
			}
			
			//关键值 
			// 
			curword =  GetWord();
			int type = curattr.attr_type[position];
			if( type == -1 )//int
			{
				scondition.key[scondition.amount-1].type = -1;
				scondition.key[scondition.amount-1].idata = atoi(curword.c_str());
			}
			else if( type == 0 )//float
			{
				scondition.key[scondition.amount-1].type = 0;
				scondition.key[scondition.amount-1].fdata = atoi(curword.c_str());
			}
			else if( type > 0 )//string
			{
				scondition.key[scondition.amount-1].type = type;
				curword = curword.erase(0,1);
				curword = curword.erase(curword.length()-1,1);
				scondition.key[scondition.amount-1].sdata = curword;	
			} 
			else
			{
				std::cout << "syntax error!" << std::endl; 
				return 0;					
			}			
			
			curword =  GetWord();
			//连接字符-and
			if( curword != "and" && curword != ";" )
			{
				std::cout << "syntax error!" << std::endl; 
				return 0;					
			}						 
		}	
	}
	
	//正式调用API函数删除 
	int deletetuplenum = 0; 
	deletetuplenum = api.Delete( tablename, scondition );
	
	if( deletetuplenum != -1 )
	{
		std::cout << "delete " << deletetuplenum << " success" << std::endl; 
		return 1;
	}	
	else
		return 0;
}

void Interpreter::ExecFile()
{
	std::string fileaddress;
	fileaddress = GetWord();

	if (fileaddress[0] != '\"')
	{
		std::cout << "syntax error!" << std::endl;
		return;
	}
	fileaddress.erase(0, 1);
	if (fileaddress[fileaddress.length()-1] != '\"')
	{
		std::cout << "syntax error!" << std::endl;
		return;
	}
	fileaddress.erase(fileaddress.length() - 1, 1);
		
	file.open(fileaddress.c_str()); 

	if (!file)
	{
		std::cout << "open file failed!" << std::endl;
		return;
	}

	fileaddress = GetWord();
	readmode = 1;

	int execresult;
	//每次操作 
	while (!file.eof())
	{
		if (GetInstruction() == 0)
		{
			std::cout << "read error!" << std::endl;
			return;
		}
		execresult = JudgeAndExec();
		if (execresult == 0)
			break;

	}

	if (!file.eof())	
		std::cout << "file exec error!" << std::endl;

	//执行完切换回输入模式 
	readmode = 0;

	if (fileaddress != ";")
	{
		std::cout << "syntax error!" << std::endl;
		return;
	}
}
