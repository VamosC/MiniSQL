//
//interpreter.cpp
//operation here: 0- =	1- <>	2- <	3- >	4- <=	5- >=
//

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
		getline(std::cin, input);
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
	cout << singleword << endl;

	if( singleword == "quit" )
		return 2;
	else if (singleword == "create")
	{
		singleword = GetWord();
		if (singleword == "table")//新建表操作
		{
			//cout << "createtable" << endl;
			if(ExecCreateTable() == 0)
			{
				cout << "新建表失败" <<endl;
				return 0;
			}
		}
		else if (singleword == "index")//新建索引操作
		{
			if(ExecCreateIndex() == 0)
			{
				cout << "新建索引失败" << endl;
				return 0;
			}
		}
		else//输入错误
		{
			cout << "语法错误，请重新输入" << endl; 
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
				cout << "删除表失败" << endl;
				return 0;
			}
		}
		else if (singleword == "index")//删除索引操作
		{
			if(ExecDropIndex() == 0)
			{
				cout << "删除索引失败" << endl;
				return 0;
			}
		}
		else//输入错误
		{
			cout << "语法错误，请重新输入" << endl; 
			return 0; 			
		}
	}
	else if (singleword == "select")//搜索操作
	{
		if(ExecSelect() == 0)
		{
			cout << "搜索失败" << endl;
			return 0;
		}
		else if(ExecSelect() == -1)
		 	cout << "搜索记录为空" << endl;
	}
	else if (singleword == "insert")//插入操作
		if(ExecInsert() == 0)
		{
			cout << "插入失败" << endl;
			return 0;
		}		
	else if (singleword == "delete")//删除元组操作
		if(ExecDelete() == 0)
		{
			cout << "插入失败" << endl;
			return 0;
		}		
	else if (singleword == "execfile")//执行文件内容操作
	{
		ExecFile();
	}
	else//输入错误
	{
			cout << "语法错误，请重新输入" << endl; 
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

	nameoftable = GetWord();	
	cur_attr.amount = 0;
	if(GetInstruction() != 1) return 0;
	cur_word = GetWord();
	while( cur_word != ");" )
	{
		if( cur_word == "primary" )
		{
			if( cur_word == "key" )
			{
				cur_word = GetWord();
				if( cur_word != "(" )
				{
					cout << "语法错误，请重新输入" << endl; 
					return 0;
				}
				
				cur_word = GetWord();
				for( int i = 0; i < cur_attr.amount; i++ )
				{
					if( cur_word == cur_attr.attr_name[i] )
						break;
				}
				if( i < amount )
					cur_attr.primary_key = i;
				else
				{
					cout << "不存在该主码" << endl; 
					return 0; 						
				}
				
				cur_word = GetWord();
				if( cur_word != ")" )
				{
					cout << "语法错误，请重新输入" << endl; 
					return 0;
				}
			}
			else
			{
				cout << "语法错误，请重新输入" << endl; 
				return 0; 				
			}
		}
		else
		{		
			std::string attrname, attrtype;
			attrname = cur_word;
			attrtype = GetWord();
			if( attrtype == "int" )
			{
				cur_word = GetWord();
				if( cur_word =="unique" || cur_word =="," )
				{
					if( cur_word == "unique" && GetWord() != "," )
					{
						cout << "语法错误，请重新输入" << endl; 
						return 0; 	
					}
		
					cur_attr.attr_name[cur_attr.amount] = attrname
					cur_attr.attr_type[cur_attr.amount] = -1;
					if( cur_word == "unique" )	cur_attr.is_unique[cur_attr.amount] = true;
					cur_attr.amount++;					
				}
				else
				{
					cout << "语法错误，请重新输入" << endl; 
					return 0; 					
				}
			}
			else if( attrtype == "float" )
			{
				cur_word = GetWord();
				if( cur_word =="unique" || cur_word =="," )
				{
					if( cur_word == "unique" && GetWord() != "," )
					{
						cout << "语法错误，请重新输入" << endl; 
						return 0; 	
					}
		
					cur_attr.attr_name[cur_attr.amount] = attrname
					cur_attr.attr_type[cur_attr.amount] = 0;
					if( cur_word == "unique" )	cur_attr.is_unique[cur_attr.amount] = true;
					cur_attr.amount++;	
				}
				else
				{
					cout << "语法错误，请重新输入" << endl; 
					return 0; 					
				}
			}
			else if( attrtype.substr( 0, 3 ) == "char" )
			{
				attrtype.erase(0, 4);
				int len = 0, pos = 1;
				if( attrtype[0] == '(' )
				{
					while( attrtype[pos] != ')' )
					{
						len = 10 * len + attrtype[pos] -'0';
						if( len > 255 || len == 0 || pos >= 4 )
							return 0; 
						pos++;
					}
					
				cur_word = GetWord();
				if( cur_word =="unique" || cur_word =="," )
				{
					if( cur_word == "unique" && GetWord() != "," )
					{
						cout << "语法错误，请重新输入" << endl; 
						return 0; 	
					}
		
					cur_attr.attr_name[cur_attr.amount] = attrname
					cur_attr.attr_type[cur_attr.amount] = len;
					if( cur_word == "unique" )	cur_attr.is_unique[cur_attr.amount] = true;
					cur_attr.amount++;	
				}
				else
				{
					cout << "语法错误，请重新输入" << endl; 
					return 0; 					
				}
			}	
			
			
		} 
		if( GetInstruction() != 1 )	 return 0;
		cur_word = GetWord();
		flag = 1;
	}
	
	if( flag != 1 || cur_attr.amount == 0 )//从一开始就有语法错误 
	{
		cout << "语法错误，请重新输入" << endl; 
		return 0; 
	}
	
	API curapi;
	if( curapi.CreateTable(nameoftable, cur_attr) == 1 ) 
	{
		cout << "成功添加表格" << nameoftable << endl; 
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
		cout << "成功删除表格" << nameoftable << endl; 
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
		cout << "语法错误，请重新输入" << endl; 
		return 0; 
	}	
	tablename = GetWord();
	if( GetWord() != "(" )
	{
		cout << "语法错误，请重新输入" << endl; 
		return 0; 
	}
	tattr = GetWord();
	if( GetWord() != ");" )
	{
		cout << "语法错误，请重新输入" << endl; 
		return 0; 
	}
	
	//接下来检查该表该属性是否存在
	Catalog curc;
	if ( curc.isTableExist(tablename) == 1 && isAttributeExist(tablename, tattr) == true )
	{
		API curapi;
		if( curapi.CreateIndex(tablename, attr, indexname) == 1 ) 
		{
			cout << "成功插入索引" << indexname << endl; 
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
		cout << "语法错误，请重新输入" << endl; 
		return 0; 
	}	
	tablename = GetWord();

	//接下来检查该表该属性是否存在
	Catalog curc;
	if ( curc.isTableExist(tablename) == 1 && isIndexExist(tablename, indexname) == true )
	{
		API curapi;
		if( curapi.DropIndex(tablename, indexname) == 1 ) 
		{
			cout << "成功删除索引" << indexname << endl; 
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
	Catolog curcatolog;
	std::vector<std::string> targetattr;
	SelectCondition scondition;
	Attribute curattr; 
	int targetan = 0;
	int isAll = 0; 
	Table selectresult;	
	API curapi;

	scondition.amount = 0;
	curword = GetWord();
	if( curword == "*" )
	{
		curword = GetWord();
	}
	else
	{
		while( curword != "from" )
		{
			targetattr[targetan] = curword;
			targetan++;
			curword = GetWord();
			if(curword == ",")
				curword = GetWord();
			
			if( targetan > 30 )
			{
				cout << "搜索属性数量超标，搜索无效" << endl; 
				return 0;	
			}
		}
	}
	if( curword != "from" )
	{
		cout << "语法错误，请重新输入" << endl; 
		return 0;	
	} 
	
	tablename = GetWord();
	//检验该表是否存在 
	if(curcatolog.isTableExist(tablename) != 1)
	{
		cout << "不存在该表，输入错误" << endl; 
		return 0; 		
	}
	//同时可以检验原来的那些属性是否存在
	for( int i = 0; i < targetan; i++ )
		if(isAttributeExist(tablename, targetattr[targetan]) == -1)
		{
			cout << "查找属性输入错误，搜索无效" << endl; 
			return 0; 		
		}
	curattr = curcatolog.GetTableAttribute(tablename);
	
	curword = GetWord();
	
	//区分是否有查找条件 
	if( curword == ";" )
	{
		isAll = 1;//表示选择全部信息 
	} 
	else if( curword != "where" )
	{
		cout << "语法错误，请重新输入" << endl; 
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
			position = isAttributeExist(tablename, curword);
			if( position == -1)
			{
				cout << "查找属性输入错误，搜索无效" << endl; 
				return 0; 		
			}
			scondition.attr[scondition.amount] = curword;
			scondition.amount ++; 
			
			//条件
			//
			curword =  GetWord();
			if( curword == "=" )
				scondition.attr_type[scondition.amount-1] = 0;
			else if( curword == "<>" )
				scondition.attr_type[scondition.amount-1] = 1;
			else if( curword == "<" )
				scondition.attr_type[scondition.amount-1] = 2;
			else if( curword == ">" )
				scondition.attr_type[scondition.amount-1] = 3;
			else if( curword == "<=" )
				scondition.attr_type[scondition.amount-1] = 4;
			else if( curword == ">=" )
				scondition.attr_type[scondition.amount-1] = 5;
			else
			{
				cout << "语法错误，请重新输入" << endl; 
				return 0;					
			}
			
			//关键值 
			// 
			curword =  GetWord();
			int type = curattr.attr_type[position];
			if( type == -1 )//int
			{
				scondition.key[scondition.amount-1].type = -1;
				scondition.key[scondition.amount-1].idata = atoi(curword);
			}
			else if( type == 0 )//float
			{
				scondition.key[scondition.amount-1].type = 0;
				scondition.key[scondition.amount-1].fdata = atoi(curword);		
			}
			else if( type > 0 )//string
			{
				scondition.key[scondition.amount-1].type = type;
				scondition.key[scondition.amount-1].sdata = curword;	
			} 
			else
			{
				cout << "语法错误，请重新输入" << endl; 
				return 0;					
			}			
			
			curword =  GetWord();
			//连接字符-and
			if( curword != "and" || curword != ";" )
			{
				cout << "语法错误，请重新输入" << endl; 
				return 0;					
			}						 
		}
		
		//正式调用API函数查找
		if(isall!=1)
			selectresult = curapi.Select( tablename, targetattr, scondition );
		else
			selectresult = curapi.Select( tablename, curattr, scondition );
	}
	
	//输出结果 
	//表名
	cout << "------------------" << selectresult.table_name << "------------------" << endl;
	//属性名 
	int namelength = 0;
	Attribute tmp_attr = selectresult.attr; 
	
	for( int i = 0; i < tmp_attr.amount; i++ )
	{
		if( tmp_attr.attr_name[i].length() > namelength )
			namelength = tmp_attr.attr_name[i].length();
	}
	
	for( int i = 0; i < tmp_attr.amount; i++ )
		cout << left << setw( namelength+5 ) << tmp_attr.attr_name[i] << '|';
	cout << endl;
	
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
		cout << "语法错误，请重新输入" << endl; 
		return 0;
	}
	
	tablename = GetWord();
	//检验该表是否存在 
	if(curcatolog.isTableExist(tablename) != 1)
	{
		cout << "不存在该表，输入错误" << endl; 
		return 0; 		
	}
	
	curword = GetWord();
	if( curword != "values" )
	{
		cout << "语法错误，请重新输入" << endl; 
		return 0;
	}

	curword = GetWord();
	if( curword != "(" )
	{
		cout << "语法错误，请重新输入" << endl; 
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
			cout << "语法错误，请重新输入" << endl; 
			return 0;
		} 
		
		if( curword[curword.length()-1] == '\'' )
			curword.erase( curword.length()-1, 1 );
		else
		{
			cout << "语法错误，请重新输入" << endl; 
			return 0;
		} 
		
		tmp.type = curattr.attr_type[number];
		if( tmp.type == -1 )//int
			tmp.idata = atoi(curword);
		else if( tmp.type == 0 )//float
			tmp.fdata = atoi(curword);		
		else if( tmp.type > 0 )//string
			tmp.sdata = curword;	
		else
		{
			cout << "语法错误，请重新输入" << endl; 
			return 0;					
		}
		
		curword = GetWord();
		if( curword != "," )
		{
			cout << "语法错误，请重新输入" << endl; 
			return 0;			
		}
		else if( curword != ";" )
		{
			tuple[number] = tmp; 
			number++;	
			curword = GetWord();	
		}
	}
	
	if( curapi.Insert(tablename, tuple) == 1 ) 
	{
		cout << "成功插入元组" << endl; 
		return 1;
	}
	else return 0;	
}

//一定要有where的条件，不然不知道删除什么元组 
int Interpreter::ExecDelete()
{
	std::string tablename;
	std::string curword;
	Catolog curcatolog;
	SelectCondition scondition;
	Attribute curattr; 
	int targetan = 0;
	API curapi;

	scondition.amount = 0;
	curword = GetWord();
	if( curword != "from" )
	{
		cout << "语法错误，请重新输入" << endl; 
		return 0;	
	} 
	
	tablename = GetWord();
	//检验该表是否存在 
	if(curcatolog.isTableExist(tablename) != 1)
	{
		cout << "不存在该表，输入错误" << endl; 
		return 0; 		
	}

	curattr = curcatolog.GetTableAttribute(tablename);
	
	curword = GetWord();
	
	//区分是否有查找条件 
	if( curword != "where" )
	{
		cout << "语法错误，请重新输入" << endl; 
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
			position = isAttributeExist(tablename, curword);
			if( position == -1)
			{
				cout << "查找属性输入错误，搜索无效" << endl; 
				return 0; 		
			}
			scondition.attr[scondition.amount] = curword;
			scondition.amount ++; 
			
			//条件
			//
			curword =  GetWord();
			if( curword == "=" )
				scondition.attr_type[scondition.amount-1] = 0;
			else if( curword == "<>" )
				scondition.attr_type[scondition.amount-1] = 1;
			else if( curword == "<" )
				scondition.attr_type[scondition.amount-1] = 2;
			else if( curword == ">" )
				scondition.attr_type[scondition.amount-1] = 3;
			else if( curword == "<=" )
				scondition.attr_type[scondition.amount-1] = 4;
			else if( curword == ">=" )
				scondition.attr_type[scondition.amount-1] = 5;
			else
			{
				cout << "语法错误，请重新输入" << endl; 
				return 0;					
			}
			
			//关键值 
			// 
			curword =  GetWord();
			int type = curattr.attr_type[position];
			if( type == -1 )//int
			{
				scondition.key[scondition.amount-1].type = -1;
				scondition.key[scondition.amount-1].idata = atoi(curword);
			}
			else if( type == 0 )//float
			{
				scondition.key[scondition.amount-1].type = 0;
				scondition.key[scondition.amount-1].fdata = atoi(curword);		
			}
			else if( type > 0 )//string
			{
				scondition.key[scondition.amount-1].type = type;
				scondition.key[scondition.amount-1].sdata = curword;	
			} 
			else
			{
				cout << "语法错误，请重新输入" << endl; 
				return 0;					
			}			
			
			curword =  GetWord();
			//连接字符-and
			if( curword != "and" || curword != ";" )
			{
				cout << "语法错误，请重新输入" << endl; 
				return 0;					
			}						 
		}
	}
	
	//正式调用API函数删除 
	int deletetuplenum = 0; 
	deletetuplenum = curapi.Delete( tablename, scondition );
	
	if( deletetuplenum != -1 )
	{
		cout << "成功删除" << deletetuplenum << "元组" << endl; 
		return 1;
	}	
	else
		return 0;
}

void ExecFile()
{
	std::string fileaddress;
	fileaddress = GetWord();
	
	file = ifstream(fileaddress);
	if( !file.open() )
	{
		cout << "打开文件失败" << endl; 
		return 0; 		
	}
	readmode = 1;
	
	int execresult;
	//每次操作 
	while( !file.eof() )
	{
		if( GetInstruction() == 1 )
		{
			execresult = JudgeAndExec();
			if(execresult == 0)
			{
				break; 
			}
		}
		else
		{
			cout << "文件读取错误" << endl; 
			break;	
		}	
	}
		
	if( !file.eof() )	cout << "文件执行错误" << endl; 
	
	//执行完切换回输入模式 
	readmode = 0; 
}

