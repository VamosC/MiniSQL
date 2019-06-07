//
//interpreter.cpp
//operation here: 0- =	1- <>	2- <	3- >	4- <=	5- >=
//

std::ifstream file; 
int readmode = 0;//��ȡ�ļ�ģʽ��0-�������ȡ�� 1-���ļ��ж�ȡ 

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

//���룺��
//�����1-�����ɹ�����ִ����Ӧ�Ľ���� 0-ʧ�ܣ� 2-�˳� 
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
		if (singleword == "table")//�½������
		{
			//cout << "createtable" << endl;
			if(ExecCreateTable() == 0)
			{
				cout << "�½���ʧ��" <<endl;
				return 0;
			}
		}
		else if (singleword == "index")//�½���������
		{
			if(ExecCreateIndex() == 0)
			{
				cout << "�½�����ʧ��" << endl;
				return 0;
			}
		}
		else//�������
		{
			cout << "�﷨��������������" << endl; 
			return 0; 
		}
	}
	else if (singleword == "drop")
	{
		singleword = GetWord();
		if (singleword == "table")//ɾ�������
		{
			if(ExecDropTable() == 0)
			{
				cout << "ɾ����ʧ��" << endl;
				return 0;
			}
		}
		else if (singleword == "index")//ɾ����������
		{
			if(ExecDropIndex() == 0)
			{
				cout << "ɾ������ʧ��" << endl;
				return 0;
			}
		}
		else//�������
		{
			cout << "�﷨��������������" << endl; 
			return 0; 			
		}
	}
	else if (singleword == "select")//��������
	{
		if(ExecSelect() == 0)
		{
			cout << "����ʧ��" << endl;
			return 0;
		}
		else if(ExecSelect() == -1)
		 	cout << "������¼Ϊ��" << endl;
	}
	else if (singleword == "insert")//�������
		if(ExecInsert() == 0)
		{
			cout << "����ʧ��" << endl;
			return 0;
		}		
	else if (singleword == "delete")//ɾ��Ԫ�����
		if(ExecDelete() == 0)
		{
			cout << "����ʧ��" << endl;
			return 0;
		}		
	else if (singleword == "execfile")//ִ���ļ����ݲ���
	{
		ExecFile();
	}
	else//�������
	{
			cout << "�﷨��������������" << endl; 
			return 0; 		
	}
	
	return 1;
}

//����������� 
//�ж�����Ƿ���ȷ����������ԭ��/������ִ�к���

//��ʱ��create table���濪ʼ�� 
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
					cout << "�﷨��������������" << endl; 
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
					cout << "�����ڸ�����" << endl; 
					return 0; 						
				}
				
				cur_word = GetWord();
				if( cur_word != ")" )
				{
					cout << "�﷨��������������" << endl; 
					return 0;
				}
			}
			else
			{
				cout << "�﷨��������������" << endl; 
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
						cout << "�﷨��������������" << endl; 
						return 0; 	
					}
		
					cur_attr.attr_name[cur_attr.amount] = attrname
					cur_attr.attr_type[cur_attr.amount] = -1;
					if( cur_word == "unique" )	cur_attr.is_unique[cur_attr.amount] = true;
					cur_attr.amount++;					
				}
				else
				{
					cout << "�﷨��������������" << endl; 
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
						cout << "�﷨��������������" << endl; 
						return 0; 	
					}
		
					cur_attr.attr_name[cur_attr.amount] = attrname
					cur_attr.attr_type[cur_attr.amount] = 0;
					if( cur_word == "unique" )	cur_attr.is_unique[cur_attr.amount] = true;
					cur_attr.amount++;	
				}
				else
				{
					cout << "�﷨��������������" << endl; 
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
						cout << "�﷨��������������" << endl; 
						return 0; 	
					}
		
					cur_attr.attr_name[cur_attr.amount] = attrname
					cur_attr.attr_type[cur_attr.amount] = len;
					if( cur_word == "unique" )	cur_attr.is_unique[cur_attr.amount] = true;
					cur_attr.amount++;	
				}
				else
				{
					cout << "�﷨��������������" << endl; 
					return 0; 					
				}
			}	
			
			
		} 
		if( GetInstruction() != 1 )	 return 0;
		cur_word = GetWord();
		flag = 1;
	}
	
	if( flag != 1 || cur_attr.amount == 0 )//��һ��ʼ�����﷨���� 
	{
		cout << "�﷨��������������" << endl; 
		return 0; 
	}
	
	API curapi;
	if( curapi.CreateTable(nameoftable, cur_attr) == 1 ) 
	{
		cout << "�ɹ���ӱ��" << nameoftable << endl; 
		return 1;
	}
	else return 0;
}

//��ʱ��drop table���濪ʼ�� 
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
		cout << "�ɹ�ɾ�����" << nameoftable << endl; 
		return 1;
	}
	else return 0;
}

//��ʱ��create index���濪ʼ�� 
int Interpreter::ExecCreateIndex()
{
	std::string indexname, tablename, tattr;

	indexname = GetWord();
	if( GetWord() != "on" )
	{
		cout << "�﷨��������������" << endl; 
		return 0; 
	}	
	tablename = GetWord();
	if( GetWord() != "(" )
	{
		cout << "�﷨��������������" << endl; 
		return 0; 
	}
	tattr = GetWord();
	if( GetWord() != ");" )
	{
		cout << "�﷨��������������" << endl; 
		return 0; 
	}
	
	//���������ñ�������Ƿ����
	Catalog curc;
	if ( curc.isTableExist(tablename) == 1 && isAttributeExist(tablename, tattr) == true )
	{
		API curapi;
		if( curapi.CreateIndex(tablename, attr, indexname) == 1 ) 
		{
			cout << "�ɹ���������" << indexname << endl; 
			return 1;
		}
		else return 0;	
	}
	
}

//�﷨�����޸ģ�ʾ����drop index xxxx on tablename ; 
int Interpreter::ExecDropIndex()
{
	std::string indexname, tablename, tattr;

	indexname = GetWord();
	if( GetWord() != "on" )
	{
		cout << "�﷨��������������" << endl; 
		return 0; 
	}	
	tablename = GetWord();

	//���������ñ�������Ƿ����
	Catalog curc;
	if ( curc.isTableExist(tablename) == 1 && isIndexExist(tablename, indexname) == true )
	{
		API curapi;
		if( curapi.DropIndex(tablename, indexname) == 1 ) 
		{
			cout << "�ɹ�ɾ������" << indexname << endl; 
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
				cout << "���������������꣬������Ч" << endl; 
				return 0;	
			}
		}
	}
	if( curword != "from" )
	{
		cout << "�﷨��������������" << endl; 
		return 0;	
	} 
	
	tablename = GetWord();
	//����ñ��Ƿ���� 
	if(curcatolog.isTableExist(tablename) != 1)
	{
		cout << "�����ڸñ��������" << endl; 
		return 0; 		
	}
	//ͬʱ���Լ���ԭ������Щ�����Ƿ����
	for( int i = 0; i < targetan; i++ )
		if(isAttributeExist(tablename, targetattr[targetan]) == -1)
		{
			cout << "���������������������Ч" << endl; 
			return 0; 		
		}
	curattr = curcatolog.GetTableAttribute(tablename);
	
	curword = GetWord();
	
	//�����Ƿ��в������� 
	if( curword == ";" )
	{
		isAll = 1;//��ʾѡ��ȫ����Ϣ 
	} 
	else if( curword != "where" )
	{
		cout << "�﷨��������������" << endl; 
		return 0; 
	}
	
	//��ʼ��Ӳ������� 
	if( curword == "where" )
	{
		while( curword != ";" )
		{
			int position;
			//����
			// 
			curword =  GetWord();
			//�����Ƿ���� 
			position = isAttributeExist(tablename, curword);
			if( position == -1)
			{
				cout << "���������������������Ч" << endl; 
				return 0; 		
			}
			scondition.attr[scondition.amount] = curword;
			scondition.amount ++; 
			
			//����
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
				cout << "�﷨��������������" << endl; 
				return 0;					
			}
			
			//�ؼ�ֵ 
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
				cout << "�﷨��������������" << endl; 
				return 0;					
			}			
			
			curword =  GetWord();
			//�����ַ�-and
			if( curword != "and" || curword != ";" )
			{
				cout << "�﷨��������������" << endl; 
				return 0;					
			}						 
		}
		
		//��ʽ����API��������
		if(isall!=1)
			selectresult = curapi.Select( tablename, targetattr, scondition );
		else
			selectresult = curapi.Select( tablename, curattr, scondition );
	}
	
	//������ 
	//����
	cout << "------------------" << selectresult.table_name << "------------------" << endl;
	//������ 
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
	
	//Ԫ�� 
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
		cout << "�﷨��������������" << endl; 
		return 0;
	}
	
	tablename = GetWord();
	//����ñ��Ƿ���� 
	if(curcatolog.isTableExist(tablename) != 1)
	{
		cout << "�����ڸñ��������" << endl; 
		return 0; 		
	}
	
	curword = GetWord();
	if( curword != "values" )
	{
		cout << "�﷨��������������" << endl; 
		return 0;
	}

	curword = GetWord();
	if( curword != "(" )
	{
		cout << "�﷨��������������" << endl; 
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
			cout << "�﷨��������������" << endl; 
			return 0;
		} 
		
		if( curword[curword.length()-1] == '\'' )
			curword.erase( curword.length()-1, 1 );
		else
		{
			cout << "�﷨��������������" << endl; 
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
			cout << "�﷨��������������" << endl; 
			return 0;					
		}
		
		curword = GetWord();
		if( curword != "," )
		{
			cout << "�﷨��������������" << endl; 
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
		cout << "�ɹ�����Ԫ��" << endl; 
		return 1;
	}
	else return 0;	
}

//һ��Ҫ��where����������Ȼ��֪��ɾ��ʲôԪ�� 
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
		cout << "�﷨��������������" << endl; 
		return 0;	
	} 
	
	tablename = GetWord();
	//����ñ��Ƿ���� 
	if(curcatolog.isTableExist(tablename) != 1)
	{
		cout << "�����ڸñ��������" << endl; 
		return 0; 		
	}

	curattr = curcatolog.GetTableAttribute(tablename);
	
	curword = GetWord();
	
	//�����Ƿ��в������� 
	if( curword != "where" )
	{
		cout << "�﷨��������������" << endl; 
		return 0; 
	}
	
	//��ʼ���ɾ������ 
	if( curword == "where" )
	{
		while( curword != ";" )
		{
			int position;
			//����
			// 
			curword =  GetWord();
			//�����Ƿ���� 
			position = isAttributeExist(tablename, curword);
			if( position == -1)
			{
				cout << "���������������������Ч" << endl; 
				return 0; 		
			}
			scondition.attr[scondition.amount] = curword;
			scondition.amount ++; 
			
			//����
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
				cout << "�﷨��������������" << endl; 
				return 0;					
			}
			
			//�ؼ�ֵ 
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
				cout << "�﷨��������������" << endl; 
				return 0;					
			}			
			
			curword =  GetWord();
			//�����ַ�-and
			if( curword != "and" || curword != ";" )
			{
				cout << "�﷨��������������" << endl; 
				return 0;					
			}						 
		}
	}
	
	//��ʽ����API����ɾ�� 
	int deletetuplenum = 0; 
	deletetuplenum = curapi.Delete( tablename, scondition );
	
	if( deletetuplenum != -1 )
	{
		cout << "�ɹ�ɾ��" << deletetuplenum << "Ԫ��" << endl; 
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
		cout << "���ļ�ʧ��" << endl; 
		return 0; 		
	}
	readmode = 1;
	
	int execresult;
	//ÿ�β��� 
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
			cout << "�ļ���ȡ����" << endl; 
			break;	
		}	
	}
		
	if( !file.eof() )	cout << "�ļ�ִ�д���" << endl; 
	
	//ִ�����л�������ģʽ 
	readmode = 0; 
}

