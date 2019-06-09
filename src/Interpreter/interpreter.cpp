//
//interpreter.cpp
//operation here: 0- =	1- <>	2- <	3- >	4- <=	5- >=
//

#include "interpreter.h"
std::ifstream file; 
int readmode = 0;//读取文件模式：0-靠输入读取； 1-从文件中读取 

Interpreter::Interpreter(string &input)
{
	instruction = istringstream(input);
}

int Interpreter::GetInstruction()
{
	std::string input;
	if(readmode==0)
		getline(cin, input);
	else
		if(!file.eof())
			getline(file, input);
		else
			return 0; 
			
	instruction = istringstream(input);
	return 1;
}

string Interpreter::GetWord()
{
	std::string word;
	instruction >> word;
	return word;
}

//输入：无
//输出：1-操作成功，会执行相应的结果； 0-失败； 2-退出 
int Interpreter::JudgeAndExec()
{
	std::string singleword;

	singleword = GetWord();
	std::cout << singleword << std::endl;

	if( singleword == "quit" )
		return 2;
	else if (singleword == "create")
	{
		singleword = GetWord();
		if (singleword == "table")//新建表操作
		{
			//std::cout << "createtable" << std::endl;
			if(ExecCreateTable() == 0)
			{
				std::cout << "新建表失败" << std::endl;
				return 0;
			}
		}
		else if (singleword == "index")//新建索引操作
		{
			if(ExecCreateIndex() == 0)
			{
				std::cout << "新建索引失败" << std::endl;
				return 0;
			}
		}
		else//输入错误
		{
			std::cout << "语法错误，请重新输入" << std::endl; 
			return 0; 
		}
	}
	else if (singleword == "drop")
	{
		singleword = GetWord();
		if (singleword == "table")//删除表操作
		{
			if(ExecDropTable() == 0)
			{
				std::cout << "删除表失败" << std::endl;
				return 0;
			}
		}
		else if (singleword == "index")//删除索引操作
		{
			if(ExecDropIndex() == 0)
			{
				std::cout << "删除索引失败" << std::endl;
				return 0;
			}
		}
		else//输入错误
		{
			std::cout << "语法错误，请重新输入" << std::endl; 
			return 0; 			
		}
	}
	else if (singleword == "select")//搜索操作
	{
		int result = ExecSelect();
		if (result == 0)
		{
			std::cout << "搜索失败" << std::endl;
			return 0;
		}
		else if (result == -1)
			std::cout << "搜索记录为空" << std::endl;
	}
	else if (singleword == "insert")//插入操作
	{
		if(ExecInsert() == 0)
		{
			std::cout << "插入失败" << std::endl;
			return 0;
		}	
	}
	else if (singleword == "delete")//删除元组操作
	{
		if(ExecDelete() == 0)
		{
			std::cout << "插入失败" << std::endl;
			return 0;
		}	
	}
	else if (singleword == "execfile")//执行文件内容操作
	{
		ExecFile();
	}
	else//输入错误
	{
			std::cout << "语法错误，请重新输入" << std::endl; 
			return 0; 		
	}
	
	return 1;
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
	API curapi;

	nameoftable = GetWord();
	if( curapi.CL.isTableExist(nameoftable) == 1)
	{
		std::cout << "该表已存在，请重新输入" << std::endl;
		return 0;		
	}
	
	if(GetWord() != "(")
	{
		std::cout << "语法错误，请重新输入" << std::endl;
		return 0;
	}
	cur_attr.amount = 0;
	if (GetInstruction() == 0)
	{
		std::cout << "读入错误" << std::endl;
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
					std::cout << "语法错误，请重新输入" << std::endl;
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
					std::cout << "不存在该属性" << std::endl;
					return 0;
				}

				cur_word = GetWord();
				if (cur_word != ")")
				{
					std::cout << "语法错误，请重新输入" << std::endl;
					return 0;
				}
			}
			else
			{
				std::cout << "语法错误，请重新输入" << std::endl;
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
						std::cout << "语法错误，请重新输入" << std::endl;
						return 0;
					}

					cur_attr.attr_name[cur_attr.amount] = attrname;
					cur_attr.attr_type[cur_attr.amount] = -1;
					if (cur_word == "unique")	cur_attr.is_unique[cur_attr.amount] = true;
					cur_attr.amount++;
				}
				else
				{
					std::cout << "语法错误，请重新输入" << std::endl;
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
						std::cout << "语法错误，请重新输入" << std::endl;
						return 0;
					}

					cur_attr.attr_name[cur_attr.amount] = attrname;
					cur_attr.attr_type[cur_attr.amount] = 0;
					if (cur_word == "unique")	cur_attr.is_unique[cur_attr.amount] = true;
					cur_attr.amount++;
				}
				else
				{
					std::cout << "语法错误，请重新输入" << std::endl;
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
						std::cout << "语法错误，请重新输入" << std::endl;
						return 0;
					}

					cur_attr.attr_name[cur_attr.amount] = attrname;
					cur_attr.attr_type[cur_attr.amount] = len;
					if (cur_word == "unique")	cur_attr.is_unique[cur_attr.amount] = true;
					cur_attr.amount++;
				}
				else
				{
					std::cout << "语法错误，请重新输入" << std::endl;
					return 0;
				}
			}
			else
			{
				std::cout << "语法错误，请重新输入" << std::endl;
				return 0;
			}

		}
		if (GetInstruction() == 0)
		{
			std::cout << "读入错误" << std::endl;
			return 0;
		}
		cur_word = GetWord();
		flag = 1;
	}
	
	
	if( curapi.CreateTable(nameoftable, cur_attr) == 1 ) 
	{
		std::cout << "成功添加表格" << nameoftable << std::endl; 
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
	
	API curapi;
	if( curapi.DropTable(nameoftable) == 1 ) 
	{
		std::cout << "成功删除表格" << nameoftable << std::endl; 
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
		std::cout << "语法错误，请重新输入" << std::endl; 
		return 0; 
	}	
	tablename = GetWord();
	if( GetWord() != "(" )
	{
		std::cout << "语法错误，请重新输入" << std::endl; 
		return 0; 
	}
	tattr = GetWord();
	if( GetWord() != ");" )
	{
		std::cout << "语法错误，请重新输入" << std::endl; 
		return 0; 
	}
	
	//接下来检查该表该属性是否存在
	Catalog curc;
	if ( curc.isTableExist(tablename) == 1 && curc.isAttributeExist(tablename, tattr) == true )
	{
		API curapi;
		if( curapi.CreateIndex(tablename, tattr, indexname) == 1 ) 
		{
			std::cout << "成功插入索引" << indexname << std::endl; 
			return 1;
		}
		else return 0;	
	}
	
}

//语法有所修改，示例：drop index xxxx on tablename ; 
int Interpreter::ExecDropIndex()
{
	std::string indexname, tablename, tattr;

	indexname = GetWord();
	if( GetWord() != "on" )
	{
		std::cout << "语法错误，请重新输入" << std::endl; 
		return 0; 
	}	
	tablename = GetWord();

	if (GetWord() != ";")
	{
		std::cout << "语法错误，请重新输入" << std::endl;
		return 0;
	}
	//接下来检查该表该属性是否存在
	Catalog curc;
	if ( curc.isTableExist(tablename) == 1 && curc.isIndexExist(tablename, indexname) == true )
	{
		API curapi;
		if( curapi.DropIndex(tablename, indexname) == 1 ) 
		{
			std::cout << "成功删除索引" << indexname << std::endl; 
			return 1;
		}
		else return 0;	
	}
}

//operation here: 0- =	1- <>	2- <	3- >	4- <=	5- >=
int Interpreter::ExecSelect()
{
	std::string tablename;
	std::string curword;
	Catalog curCatalog;
	std::vector<std::string> targetattr;
	SelectCondition scondition;
	Attribute curattr; 
	int targetan = 0;
	int isAll = 0; 
	Table selectresult;	
	API curapi;
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
				std::cout << "搜索属性数量超标，搜索无效" << std::endl; 
				return 0;	
			}
		}
	}
	if( curword != "from" )
	{
		std::cout << "语法错误，请重新输入" << std::endl; 
		return 0;	
	} 
	
	tablename = GetWord();
	//检验该表是否存在 
	if(curCatalog.isTableExist(tablename) != 1)
	{
		std::cout << "不存在该表，输入错误" << std::endl; 
		return 0; 		
	}
	//同时可以检验原来的那些属性是否存在
	for( int i = 0; i < targetan; i++ )
		if(curCatalog.isAttributeExist(tablename, targetattr[targetan]) == -1)
		{
			std::cout << "查找属性输入错误，搜索无效" << std::endl; 
			return 0; 		
		}
	curattr = curCatalog.GetTableAttribute(tablename);
	
	curword = GetWord();
	
	//区分是否有查找条件 
	if( curword == ";" )
	{
		isAll = 1;//表示选择全部信息 
	} 
	else if( curword != "where" )
	{
		std::cout << "语法错误，请重新输入" << std::endl; 
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
			position = curCatalog.isAttributeExist(tablename, curword);
			if( position == -1)
			{
				std::cout << "查找属性输入错误，搜索无效" << std::endl; 
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
				std::cout << "语法错误，请重新输入" << std::endl; 
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
				std::cout << "语法错误，请重新输入" << std::endl; 
				return 0;					
			}			
			
			curword =  GetWord();
			//连接字符-and
			if( curword != "and" && curword != ";" )
			{
				std::cout << "语法错误，请重新输入" << std::endl; 
				return 0;					
			}						 
		}	
	}
	
	if( isAll == 1 )	scondition.amount = 0;
	
	//正式调用API函数查找
	if(isall!=1)
		selectresult = curapi.Select( tablename, targetattr, scondition );
	else//select *
	{
		for (int i = 0; i < curattr.amount; i++)
			targetattr[i] = curattr.attr_name[i];

		selectresult = curapi.Select(tablename, targetattr, scondition );
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
		std::cout << left << setw( namelength+5 ) << tmp_attr.attr_name[i] << '|';
	std::cout << std::endl;
	
	//元组 
	std::vector<Tuple>::iterator it;
	for( it = selectresult.tuples.begin(); it != selectresult.tuples.end(); it++ )
		it->Printdata(namelength);
	
	return 1;
} 

int Interpreter::ExecInsert()
{
	std::vector<Data> tuple;
	Catalog curcatalog;
	Attribute curattr;
	API curapi;
	std::string tablename, curword;
	
	curword = GetWord();
	if( curword != "into" )
	{
		std::cout << "语法错误，请重新输入" << std::endl; 
		return 0;
	}
	
	tablename = GetWord();
	//检验该表是否存在 
	if(curcatalog.isTableExist(tablename) != 1)
	{
		std::cout << "不存在该表，输入错误" << std::endl; 
		return 0; 		
	}
	
	curword = GetWord();
	if( curword != "values" )
	{
		std::cout << "语法错误，请重新输入" << std::endl; 
		return 0;
	}

	curword = GetWord();
	if( curword != "(" )
	{
		std::cout << "语法错误，请重新输入" << std::endl; 
		return 0;
	}	
	
	curword = GetWord();
	curattr = curcatalog.GetTableAttribute(tablename);
	int number = 0;
	while(curword != ")" )
	{
		Data tmp;
		if( curword[0] == '\'' )
			curword.erase( 0, 1 );
		else
		{
			std::cout << "语法错误，请重新输入" << std::endl; 
			return 0;
		} 
		
		if( curword[curword.length()-1] == '\'' )
			curword.erase( curword.length()-1, 1 );
		else
		{
			std::cout << "语法错误，请重新输入" << std::endl; 
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
			std::cout << "语法错误，请重新输入" << std::endl; 
			return 0;					
		}
		
		curword = GetWord();
		if( curword != "," && curword != ")" )
		{
			std::cout << "语法错误，请重新输入" << std::endl; 
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
		std::cout << "语法错误，请重新输入" << std::endl;
		return 0;
	}
	
	if( curapi.Insert(tablename, tuple) == 1 ) 
	{
		std::cout << "成功插入元组" << std::endl; 
		return 1;
	}
	else return 0;	
}

//一定要有where的条件，不然不知道删除什么元组 
int Interpreter::ExecDelete()
{
	std::string tablename;
	std::string curword;
	Catalog curCatalog;
	SelectCondition scondition;
	Attribute curattr; 
	int targetan = 0;
	API curapi;

	scondition.amount = 0;
	curword = GetWord();
	if( curword != "from" )
	{
		std::cout << "语法错误，请重新输入" << std::endl; 
		return 0;	
	} 
	
	tablename = GetWord();
	//检验该表是否存在 
	if(curCatalog.isTableExist(tablename) != 1)
	{
		std::cout << "不存在该表，输入错误" << std::endl; 
		return 0; 		
	}

	curattr = curCatalog.GetTableAttribute(tablename);
	
	curword = GetWord();
	
	//区分是否有查找条件 
	if( curword != "where" )
	{
		std::cout << "语法错误，请重新输入" << std::endl; 
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
			position = curCatalog.isAttributeExist(tablename, curword);
			if( position == -1)
			{
				std::cout << "查找属性输入错误，搜索无效" << std::endl; 
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
				std::cout << "语法错误，请重新输入" << std::endl; 
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
				std::cout << "语法错误，请重新输入" << std::endl; 
				return 0;					
			}			
			
			curword =  GetWord();
			//连接字符-and
			if( curword != "and" && curword != ";" )
			{
				std::cout << "语法错误，请重新输入" << std::endl; 
				return 0;					
			}						 
		}	
	}
	
	//正式调用API函数删除 
	int deletetuplenum = 0; 
	deletetuplenum = curapi.Delete( tablename, scondition );
	
	if( deletetuplenum != -1 )
	{
		std::cout << "成功删除" << deletetuplenum << "元组" << std::endl; 
		return 1;
	}	
	else
		return 0;
}

void Interpreter::ExecFile()
{
	std::string fileaddress;
	fileaddress = GetWord();
	
	file.open(fileaddress.c_str());
	if( !file.is_open() )
	{
		std::cout << "打开文件失败" << std::endl; 
		return; 		
	}
	readmode = 1;
	
	int execresult;
	//每次操作 
	while( !file.eof() )
	{
		if (GetInstruction() == 0)
		{
			std::cout << "读入错误" << std::endl;
			return;
		}
		execresult = JudgeAndExec();
		if(execresult == 0)
			break; 

	}
		
	if( !file.eof() )	std::cout << "文件执行错误" << std::endl; 
	
	//执行完切换回输入模式 
	readmode = 0; 
}

