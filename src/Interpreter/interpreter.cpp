//
//interpreter.cpp
//operation here: 0- =	1- <>	2- <	3- >	4- <=	5- >=
//

#include "interpreter.h"
#include <iostream>
#include <iomanip>

Interpreter::Interpreter(API &api, CatalogManager &cm)
	: api(api), catalog_manager(cm)
{}

void Interpreter::GetInput(std::string &input)
{
	instruction = std::istringstream(input);
}

bool Interpreter::GetInstruction(std::ifstream* file)
{
	std::string input;
	if(file == nullptr)
	{
		do
		{
			getline(std::cin, input);
		}
		while(input == "");
		GetInput(input);
		return true;
	}
	else
	{
		do
		{
			getline(*file, input);
		}
		while(input == "" && !file->eof());
		if(file->eof())
		{
			return false;
		}
		else
		{
			GetInput(input);
			return true;
		}
	}
}


std::string Interpreter::GetWord()
{
	std::string word;
	instruction >> word;
	return word;
}

//输入：无
//输出：true 退出 false 继续
bool Interpreter::JudgeAndExec(std::ifstream *file)
{
	std::string singleword = GetWord();

	if(singleword == "quit")
		return true;
	else if (singleword == "create")
	{
		singleword = GetWord();
		if (singleword == "table")//新建表操作
		{
			ExecCreateTable(file);
			return false;
		}
		else if (singleword == "index")//新建索引操作
		{
			ExecCreateIndex();
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
			ExecDropTable();
			return false;
		}
		else if (singleword == "index")//删除索引操作
		{
			ExecDropIndex();
			return false;
		}
		else//输入错误
		{
			std::cout << "syntax error!" << std::endl; 
			return false;
		}	
	}
	else if (singleword == "select")//搜索操作
	{
		ExecSelect();
		return false;
	}
	else if (singleword == "insert")//插入操作
	{
		ExecInsert();
		return false;
	}
	else if (singleword == "delete")//删除元组操作
	{
		ExecDelete();
		return false;
	}
	else if (singleword == "execfile")//执行文件内容操作
	{
		return ExecFile();
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
void Interpreter::ExecCreateTable(std::ifstream *file)
{
	std::string nameoftable;
	std::string cur_word;
	Attribute cur_attr;
	cur_attr.primary_key = -1;
	int flag = 0;
	int isend = 0;

	nameoftable = GetWord();
	
	if (nameoftable[nameoftable.size() - 1] == '(')
		nameoftable.erase(nameoftable.size() - 1, 1);
	else if(GetWord() != "(")
	{
		std::cout << "syntax error!" << std::endl;
		return;
	}

	cur_attr.amount = 0;
	if (!GetInstruction(file))
	{
		std::cout << "read error!" << std::endl;
		return;
	}
	cur_word = GetWord();
	while (cur_word != ");")
	{
		if (cur_word == "primary")
		{
			cur_word = GetWord();
			if (cur_word.substr(0, 3) == "key")
			{
				if(cur_word.size() == 3)
				{
					cur_word = GetWord();
				}
				else
				{
					cur_word.erase(0, 3);
				}
				if (cur_word[0] != '(')
				{
					std::cout << "syntax error!" << std::endl;
					return;
				}

				if (cur_word.size() != 1)
					cur_word = cur_word.erase(0, 1);
				else
					cur_word = GetWord();

				if (cur_word[cur_word.size() - 1] == ')')
				{
					cur_word = cur_word.erase(cur_word.size() - 1, 1);
					isend = 1;
				}

				auto i = 0;
				for (i = 0; i < cur_attr.amount; i++)
				{
					if (cur_word == cur_attr.attr_name[i])
						break;
				}
				if (i < cur_attr.amount)
					cur_attr.primary_key = i;
				else
				{
					std::cout << "primary key is not in the attribute!" << std::endl;
					return;
				}
				
				cur_word = GetWord();
				if( (isend == 1 && cur_word != "" ) || (isend == 0 && cur_word != ")") )
				{
					std::cout << "syntax error!" << std::endl;
					return;
				}
			}
			else
			{
				std::cout << "syntax error!" << std::endl;
				return;
			}
		}
		else
		{
			std::string attrname, attrtype;
			attrname = cur_word;
			attrtype = GetWord();
			int isattrend = 0;
			if (attrtype.substr(0,3) == "int")
			{
				if (attrtype.size() == 4)
				{
					if(attrtype[3] == ',')
						isattrend = 1;
					else
					{
						std::cout << "syntax error!" << std::endl;
						return;
					}
				}
				cur_attr.attr_name[cur_attr.amount] = attrname;
				cur_attr.attr_type[cur_attr.amount] = -1;
				cur_attr.is_unique[cur_attr.amount] = false;
				if (isattrend == 0)
				{
					cur_word = GetWord();
					if (cur_word.substr(0,6) == "unique" || cur_word == ",")
					{
						if (cur_word == "unique," || (cur_word == "unique" && GetWord() == ",") || cur_word == ",")
							isattrend = 1;
						else
						{
							std::cout << "syntax error!" << std::endl;
							return;
						}
						if (cur_word.substr(0, 6) == "unique")
							cur_attr.is_unique[cur_attr.amount] = true;
					}
					else
					{
						std::cout << "syntax error!" << std::endl;
						return;
					}
				}
				cur_attr.amount++;
			}
			else if (attrtype.substr(0, 5) == "float")
			{
				if (attrtype.size() == 6)
				{
					if (attrtype[5] == ',')
						isattrend = 1;
					else
					{
						std::cout << "syntax error!" << std::endl;
						return;
					}
				}
				cur_attr.attr_name[cur_attr.amount] = attrname;
				cur_attr.attr_type[cur_attr.amount] = 0;
				cur_attr.is_unique[cur_attr.amount] = false;
				if (isattrend == 0)
				{
					cur_word = GetWord();
					if (cur_word.substr(0, 6) == "unique" || cur_word == ",")
					{
						if (cur_word == "unique," || (cur_word == "unique" && GetWord() == ",") || cur_word == "," )
							isattrend = 1;
						else
						{
							std::cout << "syntax error!" << std::endl;
							return;
						}

						if (cur_word.substr(0, 6) == "unique")
							cur_attr.is_unique[cur_attr.amount] = true;
					}
					else
					{
						std::cout << "syntax error!" << std::endl;
						return;
					}
				}
				cur_attr.amount++;
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
							return;
						pos++;
					}

				cur_word = GetWord();
				if (isattrend == 0)
				{
					if (cur_word.substr(0, 6) == "unique" || cur_word == ",")
					{
						if (cur_word == "unique," || (cur_word == "unique" && GetWord() == ",") || cur_word == ",")
							isattrend = 1;
						else
						{
							std::cout << "syntax error!" << std::endl;
							return;
						}

						cur_attr.attr_name[cur_attr.amount] = attrname;
						cur_attr.attr_type[cur_attr.amount] = len;
						if (cur_word.substr(0, 6) == "unique")
							cur_attr.is_unique[cur_attr.amount] = true;
						else
							cur_attr.is_unique[cur_attr.amount] = false;
						cur_attr.amount++;
					}
					else
					{
						std::cout << "syntax error!" << std::endl;
						return;
					}
				}
			}
			else
			{
				std::cout << "syntax error!" << std::endl;
				return;
			}

		}
		if (!GetInstruction(file))
		{
			std::cout << "read error!" << std::endl;
			return;
		}
		cur_word = GetWord();
		flag = 1;
	}


	api.CreateTable(nameoftable, cur_attr);
}

//此时从drop table后面开始读 
void Interpreter::ExecDropTable()
{
	std::string nameoftable;	
	std::string cur_word;
	nameoftable	= GetWord();
	if (nameoftable[nameoftable.size() - 1] == ';')
	{
		nameoftable.erase(nameoftable.size() - 1, 1);
		if(GetWord()!="")
		{
			std::cout << "syntax error!" << std::endl;
			return;
		}
	}
	else if (GetWord() != ";")
	{
		std::cout << "syntax error!" << std::endl;
		return;
	}

	api.DropTable(nameoftable);
}

//此时从create index后面开始读 
void Interpreter::ExecCreateIndex()
{
	std::string indexname, tablename, tattr;

	indexname = GetWord();
	if (GetWord() != "on")
	{
		std::cout << "syntax error!" << std::endl;
		return;
	}
	tablename = GetWord();
	tattr = GetWord();
	int isend = 0;

	if (tattr[0] == '(')
	{
		if (tattr.size() == 1)
			tattr = GetWord();
		else
			tattr.erase(0, 1);

		if (tattr[tattr.size() - 1] == ')')
		{
			tattr.erase(tattr.size() - 1, 1);
			isend = -1;
		}
		else if (tattr.substr(tattr.size() - 2, 2) == ");")
		{
			tattr.erase(tattr.size() - 2, 2);
			isend = 1;
		}
	}
	else
	{
		std::cout << "syntax error!" << std::endl;
		return;
	}

	if (isend == 0 && tattr == "" )  tattr = GetWord(); //只有(
	if ((isend == 1 && GetWord() != "") || (isend == -1 && GetWord() != ";") )
	{
		std::cout << "syntax error!" << std::endl;
		return;
	}
	else if (isend == 0 && tattr == ")")
	{	if (GetWord() != ";")
		{
			std::cout << "syntax error!" << std::endl;
			return;
		}
	}
	else if (isend == 0 && tattr != ");")
	{
		std::cout << "syntax error!" << std::endl;
		return;
	}

	api.CreateIndex(tablename, tattr, indexname);
}

//语法有所修改，示例：drop index xxxx on tablename ; 
void Interpreter::ExecDropIndex()
{
	std::string indexname, tablename, tattr;

	indexname = GetWord();

	if( GetWord() != "on" )
	{
		std::cout << "syntax error!" << std::endl; 
		return;
	}	
	tablename = GetWord();

	if (tablename[tablename.size() - 1] == ';')
	{
		tablename.erase(tablename.size() - 1, 1);
		if (GetWord() != "")
		{
			std::cout << "syntax error!" << std::endl;
			return;
		}
	}
	else if (GetWord() != ";")
	{
		std::cout << "syntax error!" << std::endl;
		return;
	}
	api.DropIndex(tablename, indexname);
}

//operation here: 0- =	1- <>	2- <	3- >	4- <=	5- >=
void Interpreter::ExecSelect()
{
	std::string table_name;
	std::string curword;
	std::vector<std::string> targetattr;
	SelectCondition scondition;
	Attribute curattr; 
	int targetan = 0;
	int isAll = 0; 
	Table selectresult;	
	int isall = 0;
	int isend = 0;

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
				return;	
			}
		}
	}
	if( curword != "from" )
	{
		std::cout << "syntax error!" << std::endl; 
		return;	
	} 
	
	table_name = GetWord();

	if (table_name[table_name.size() - 1] == ';')
	{
		table_name.erase(table_name.size() - 1, 1);
		isend = 1;
	}

	//检验该表是否存在 
	if(!catalog_manager.isTableExist(table_name))
	{
		std::cout << "Table " << table_name << " not exists!" << std::endl; 
		return; 		
	}

	curattr = catalog_manager.GetTableAttribute(table_name);
	
	curword = GetWord();
	
	//区分是否有查找条件 
	if( (isend == 1 && curword == "") || (isend == 0 && curword == ";") )
	{
		isAll = 1;//表示选择全部信息 
	} 
	else if( isend == 0 && curword != "where" )
	{
		std::cout << "syntax error!" << std::endl; 
		return; 
	}
	
	//开始添加查找条件 
	if( curword == "where" )
	{
		while (curword != ";")
		{
			int position;
			//属性
			// 
			curword = GetWord();
			//属性是否存在 
			
			position = catalog_manager.isAttributeExist(table_name, curword);
			/*if( position == -1)
			{
				std::cout << "attributes error!" << std::endl;
				return 0;
			}
			*/
			scondition.attr[scondition.amount] = curword;
			scondition.amount++;

			//条件
			//
			curword = GetWord();
			if (curword == "=")
				scondition.operationtype[scondition.amount - 1] = 0;
			else if (curword == "<>")
				scondition.operationtype[scondition.amount - 1] = 1;
			else if (curword == "<")
				scondition.operationtype[scondition.amount - 1] = 2;
			else if (curword == ">")
				scondition.operationtype[scondition.amount - 1] = 3;
			else if (curword == "<=")
				scondition.operationtype[scondition.amount - 1] = 4;
			else if (curword == ">=")
				scondition.operationtype[scondition.amount - 1] = 5;
			else
			{
				std::cout << "syntax error!" << std::endl;
				return;
			}

			//关键值 
			// 
			curword = GetWord();
			if (curword[curword.size() - 1] == ';')
			{
				curword.erase(curword.size() - 1, 1);
				isend = 1;
			}

			int type = curattr.attr_type[position];
			if( type == -1 )//int
			{
				scondition.key[scondition.amount-1].type = -1;
				scondition.key[scondition.amount-1].idata = atoi(curword.c_str());
			}
			else if( type == 0 )//float
			{
				scondition.key[scondition.amount-1].type = 0;
				scondition.key[scondition.amount-1].fdata = atof(curword.c_str());
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
				return;					
			}			
			
			curword =  GetWord();
			//连接字符-and
			if (isend == 1)
			{
				if (curword == "")	break;
				else
				{
					std::cout << "syntax error!" << std::endl;
					return;
				}
			}
			else if( curword != "and" && curword != ";" )
			{
				std::cout << "syntax error!" << std::endl; 
				return;					
			}						 
		}	
	}
	
	if( isAll == 1 )	
		scondition.amount = 0;
	
	//正式调用API函数查找
	if(isall == 1)
		api.Select(table_name, targetattr, scondition);
	else//select *
	{
		for (int i = 0; i < curattr.amount; i++)
			targetattr[i] = curattr.attr_name[i];
		api.Select(table_name, targetattr, scondition );
	}
	
	// //输出结果 
	// //表名
	// std::cout << "------------------" << selectresult.table_name << "------------------" << std::endl;
	// //属性名 
	// int namelength = 0;
	// Attribute tmp_attr = selectresult.attr; 
	
	// for( int i = 0; i < tmp_attr.amount; i++ )
	// {
	// 	if( tmp_attr.attr_name[i].length() > namelength )
	// 		namelength = tmp_attr.attr_name[i].length();
	// }
	
	// for( int i = 0; i < tmp_attr.amount; i++ )
	// 	std::cout << std::left << std::setw( namelength+5 ) << tmp_attr.attr_name[i] << '|';
	// std::cout << std::endl;
	
	// //元组 
	// std::vector<Tuple>::iterator it;
	// for( it = selectresult.tuples.begin(); it != selectresult.tuples.end(); it++ )
	// 	it->Printdata();
	
	// return 1;
} 

//insert into s values ( 'sss', 'aaa' ); 
//对于结尾的判断：...' - 1; ...') - 2; ...'); -3 ...', -4
void Interpreter::ExecInsert()
{
	std::vector<Data> tuple;
	Attribute curattr;
	std::string table_name, curword;
	
	curword = GetWord();
	if( curword != "into" )
	{
		std::cout << "syntax error!" << std::endl; 
		return;
	}
	
	table_name = GetWord();
	//检验该表是否存在 
	if(!catalog_manager.isTableExist(table_name))
	{
		std::cout << "Insert into table " << table_name << " error!:";
		std::cout << "Table " << table_name << " not exists!" << std::endl; 
		return;	
	}
	
	curword = GetWord();

	if( curword.substr(0, 6) != "values" )
	{
		std::cout << "syntax error!" << std::endl;
		return;
	}

	if(curword.size() == 6)
	{
		curword = GetWord();
	}
	else
	{
		curword.erase(0, 6);
	}
	if (curword[0] != '(')
	{
		std::cout << "syntax error!" << std::endl;
		return;
	}
	else
		curword.erase(0, 1);
	
	//处理第一个值
	if(curword == "")  
		curword = GetWord();
	curattr = catalog_manager.GetTableAttribute(table_name);
	int number = 0;
	int isend = 0;
	int endtype = 4;
	int curpos = 0;
	while( endtype == 4 )
	{
		int isavalueend = 0;	
		//对于结尾的判断：... - 1; ...) - 2; ...); -3 ..., -4
		if (curword[curword.length() - 1] == ';')//...);
		{
			if (curword[curword.length() - 2] != ')')
			{
				std::cout << "syntax error!" << std::endl;
				return;
			}

			endtype = 3;
			curword.erase(curword.length() - 2, 2);
		}
		else if (curword[curword.length() - 1] == ')')//...)  
		{
			if (GetWord() != ";")
			{
				std::cout << "syntax error!" << std::endl;
				return;
			}
			endtype = 2;
			curword.erase(curword.length() - 1, 1);
		}
		else if (curword[curword.length() - 1] == ',')//...,  
		{
			curword.erase(curword.length() - 1, 1);
			endtype = 4;
		}
		else
		{
			endtype = 1;
		}
		while(!curword.empty())
		{
			auto comma = curword.find(",");
			std::string item;
			if(comma != std::string::npos)
			{
				item = curword.substr(0, comma);
				curword.erase(0, comma+1);
			}
			else
			{
				item = curword;
				curword = "";
			}
			Data tmp;
			tmp.type = curattr.attr_type[number];
			if (tmp.type == -1)//int
				tmp.idata = atoi(item.c_str());
			else if (tmp.type == 0)//float
				tmp.fdata = atof(item.c_str());
			else if (tmp.type > 0)//string
			{
				if (curword[0] != '\'' && curword[item.size() - 1] != '\'')
				{
					std::cout << "syntax error!" << std::endl;
					return;
				}
				item.erase(0, 1);
				item.erase(item.size() - 1, 1);
				tmp.sdata = item;
			}
			else
			{
				std::cout << "MiniSQL not support such type!" << std::endl;
				return;
			}
			tuple.push_back(tmp);
			number++;
		}

		if (endtype == 3 || endtype == 2)	
			break;

		curword = GetWord();
		if (endtype == 1)
		{
			if (curword == ")")
			{
				if (GetWord() == ";")
					break;
				else
				{
					std::cout << "syntax error!" << std::endl;
					return;
				}
			}
			else if (curword == ");")	break;
			else if (curword != ",")
			{
				std::cout << "syntax error!" << std::endl;
				return;
			}
		}
		endtype = 4;
	}
	
	if( GetWord() != "" )
	{
		std::cout << "syntax error!" << std::endl;
		return;
	}
	// for(auto it : tuple)
	// {
	// 	if(it.type == INT)
	// 	{
	// 		std::cout << it.idata << '\n';
	// 	}
	// 	else if(it.type == FLOAT)
	// 	{
	// 		std::cout << it.fdata << '\n';
	// 	}
	// 	else
	// 	{
	// 		std::cout << it.sdata << '\n';
	// 	}
	// }
	api.Insert(table_name, tuple);
}

//一定要有where的条件，不然不知道删除什么元组 
void Interpreter::ExecDelete()
{
	std::string table_name;
	std::string curword;
	SelectCondition scondition;
	Attribute curattr; 
	int targetan = 0;
	int isend = 0;

	scondition.amount = 0;
	curword = GetWord();
	if( curword != "from" )
	{
		std::cout << "syntax error!" << std::endl; 
		return;	
	} 
	
	table_name = GetWord();
	if (table_name[table_name.size() - 1] == ';')
	{
		isend = 1;
		table_name.erase(table_name.size() - 1, 1);
	}
	
	//检验该表是否存在 
	if(!catalog_manager.isTableExist(table_name))
	{
		std::cout << "Table " << table_name<< " not exists!" << std::endl; 
		return; 		
	}
	
	curattr = catalog_manager.GetTableAttribute(table_name);
	
	curword = GetWord();
	if( (isend == 0 && curword == ";") || (isend == 1 && curword == ""))
	{
		//正式调用API函数删除 
		api.Delete( table_name, scondition );
		return;	
	}
			
	//区分是否有查找条件 
	if( isend == 1 || curword != "where" )
	{
		std::cout << "syntax error!" << std::endl; 
		return; 
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
			position = catalog_manager.isAttributeExist(table_name, curword);
			/*if( position == -1)
			{
				std::cout << "attributes error!" << std::endl; 
				return 0; 		
			}
			*/
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
				return;					
			}
			
			//关键值 
			// 
			curword = GetWord();
			if (curword[curword.size() - 1] == ';')
			{
				curword.erase(curword.size() - 1, 1);
				isend = 1;
			}

			int type = curattr.attr_type[position];
			if (type == -1)//int
			{
				scondition.key[scondition.amount - 1].type = -1;
				scondition.key[scondition.amount - 1].idata = atoi(curword.c_str());
			}
			else if (type == 0)//float
			{
				scondition.key[scondition.amount - 1].type = 0;
				scondition.key[scondition.amount - 1].fdata = atof(curword.c_str());
			}	
			else if (type > 0)//string
			{
				scondition.key[scondition.amount - 1].type = type;
				curword = curword.erase(0, 1);
				curword = curword.erase(curword.length() - 1, 1);
				scondition.key[scondition.amount - 1].sdata = curword;
			}
			else
			{
				std::cout << "syntax error!" << std::endl;
				return;
			}

			curword = GetWord();
			//连接字符-and
			if (isend == 1)
			{
				if (curword == "")	break;
				else
				{
					std::cout << "syntax error!" << std::endl;
					return;
				}
			}
			else if (curword != "and" && curword != ";")
			{
				std::cout << "syntax error!" << std::endl;
				return;
			}
		}	
	}
	
	//正式调用API函数删除 
	api.Delete( table_name, scondition );
}

bool Interpreter::ExecFile()
{
	std::string input;
	std::ifstream file;
	std::string fileaddress;
	fileaddress = GetWord();

	if(fileaddress[fileaddress.size() - 1] == ';')
	{
		fileaddress.erase(fileaddress.size() - 1, 1);
	}
	// 读;
	else
	{
		GetWord();
	}
	file.open(fileaddress.c_str()); 

	if (!file)
	{
		std::cout << "open file failed!" << std::endl;
		return false;
	}

	int execresult;
	//每次操作 
	while (!file.eof())
	{
		if(!GetInstruction(&file))
		{
			return false;
		}
		if(JudgeAndExec(&file))
		{
			return true;
		}
	}

	if (!file.eof())	
		std::cout << "file exec error!" << std::endl;

	//执行完切换回输入模式 
	return false;
}
