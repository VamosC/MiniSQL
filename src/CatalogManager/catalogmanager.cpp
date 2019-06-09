//
//	catalogmanager.cpp
//	Ŀ¼����

#include "catalogmanager.h"

//�������ַ���ת��Ϊ���� 
int Catolog::String2Num(string tmp)
{
	return atoi(tmp.c_str());
}

//������ת��Ϊ�ַ��� 
string Catolog::Num2String(int tmp)
{
	string result = "";
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

//�õ����ĳ�����Ϣ�Ŀ��� -----------------------------------------
//��������Ϊ0 
int Catolog::GetBlockAmount(string tablename)
{
	char *pagecontent;
	int num = 0;
	
	pagecontent = //�õ�buffer_manager��һ�����������,��ʹ�õ�������num�� 
	while( pagecontent[0] != '\0' )
	{
		num++;
		pagecontent = //�õ�buffer_manager��һ�����������,��ʹ�õ�������num�� 
	}
	
	return num; 
}


//���ڱ��Ĳ��� 
		
//�������
//���룺������ơ�������ԡ������������� 
//���: 1-�ɹ��� 0-ʧ��,�����쳣 
int Catolog::CreateTable(string tablename, Attribute attr, Index indices, int primary_key)
{
	//����Ƿ���ͬ����Ĵ��� 
	if( isTableExist(tablename) == true )
	{
		cout << "�Ѵ��ڸñ��������" << endl;
		return 0; 
	} 
	
	//��������Ϣ���浽�ַ���������������ļ���
	//��ʽ,������������һ������һ�����ʵĶ��룬��������һ����Ϣ�ÿո���� 
	//@@ tablename attribute_number attrbute_name type is_unique(��˳��) primarykeynumber
	// index_number index_name towhatattribute
	//\n
	string outputstr = "@@ ";
	outputstr += tablename;
	output += (" " + Num2String(attr.amount));
	
	string TorF;
	for(int i = 0; i < attr.amount; i++)
	{
		if(attr.is_unique == true)
			TorF = "true";
		else
			TorF = "false";
		
		outputstr += ( " " + attr.attr_name[i] + Num2String(attr.attr_type[i]) + TorF );		
	}
	
	outputstr += ( " " + Num2String(attr.primary_key) );
	
	outputstr += ( " " + Num2String(indices.amount) );
	for(int i = 0; i < attr.amount; i++)
		outputstr += ( " " + indices.attr_name[i] + Num2String(indices.whose[i]) );		
	outputstr += "\n";
	
	//���漰������������??
		//����ÿ����Ϣ�ĳ���
		//�������õĿ���
		//�������еĿ�Ѱ�Һ��ʵ�λ�ã����֮ǰ�Ŀ鲻���ã����һ��/�½�һ����� 
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
 

//ɾ�����
//���룺�������
//�����1-�ɹ��� 0-ʧ��,�����쳣  
int Catolog::DropTable(string tablename)
{
	if( isTableExist(tablename) == false )
	{
		cout << "�����ڸñ��������" << endl;
		return 0; 
	} 
	//�ҵ���Ӧ�Ŀ�
	int block;
	int begin = GetTablePlace(tablename, block);

	char* buffer = buffer_manager.getPage(TABLE_PATH, block);
	int PID = buffer_manager.getPageId(TABLE_PATH, block);

	string check = buffer;

	//ɾ����Ӧ����Ϣ������������Ժ�������Ϣ 
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
	
	//ˢ��ҳ�� 
	buffer_manager.modifyPage(PID);

	return 1;
} 
 
//ͨ�������鿴���Ƿ����	
//���룺�������
//�����true-���ڣ� false-������
bool Catolog::isTableExist(string tablename)
{
	//�������еĿ飬ͨ��@@��ͷ�ֱ�����Ϣ
		//���������ͬ�ı������ͷ���true
		if( tmp_name = tablename )	return true;
		//����һ�������Ϣȥ 
	
	
	return false; 
} 

//��ӡ�����Ϣ  ??��������֪����ѯ����ķ�����ʽ 
void Catolog::PrintTable(string tablename, Attribute tattr)
{
	if( isTableExist( tablename ) == false )
	{
		cout << "�����ڸñ��������" << endl;
		return 0; 
	}
	
	//��ӡ�����Ϣ
	cout << "------------------" << tablename << "------------------" << endl;
	//��ӡ������Ϣ
	int namelength = 0;
	Attribute tmp_attr = GetTableAttribute(tablename); 
	cout << ">>>Primary key:  "; 
	
	for( int i = 0; i < tmp_attr.amount; i++ )
	{
		if( tmp_attr.attr_name[i].length() > namelength )
			namelength = tmp_attr.attr_name[i].length();
		if( i == tmp_attr.primary_key )
			cout << tmp_attr.attr_name[i] << endl;
	}
	
	cout << ">>>Attribute   " << "number:" << tmp_attr.amount << endl;
	cout << left << setw(namelength + 3) << "Name"  << left << setw(12) << "| Type"  << "| is unique?" << endl;
	for( int i = 0; i < tmp_attr.amount; i++ )
	{
		//������
		cout << left << setw( namelength + 3 ) << tmp_attr.attr_name[i];
		//��������
		switch( tmp_attr.attr_type[i] )
		{
			case 0:
			{
				cout << left << setw( 10 ) << "float";
				break;
			}
			case -1:
			{
				cout << left << setw( 10 ) << "int";
				break;
			}
			default:
			{
				cout << "varchar("<< left << setw( 3 ) << tmp_attr.attr_type[i] << ")";
				break;
			}
		}
		//�Ƿ�Ψһ 
		if( tmp_attr.is_unique[i] == true )
			cout << " T" << endl;
		else
			cout << " F" << endl;
	}
	
	//��ӡ����
	Index tmp_ind = GetTableIndex(tablename);
	cout << ">>>Index   number:" << tmp_ind.amount << endl; 
	cout << left << setw(namelength + 3) << "Name"  << "| Attribute" << endl;
	for( int i = 0; i < tmp_ind.amount; i++ )
	{
		//������
		cout << left << setw( namelength + 3 ) << tmp_ind.attr_name[i];
		//������Ӧ����
		cout << tmp_attr.attr_name[tmp_ind.whose[i]] << endl;
	}
} 	
 


//�������Ժ�����

//ĳһ�����Ƿ����,�����ڱ����ʱ����ʹ�� 
//���룺������ơ���������
//�����true-���ڣ� false-������		 
bool Catolog::isAttributeExist(string tablename, string tattr)
{
	Attribute tmp_attr = GetAttributeMessage( tablename );
	for( i = 0; i < tmp_attr.amount; i++ )
	{
		if( tmp_attr.attr_name[i] == tattr )
			return true;
	}
	return false;
}

//�õ�ĳ���ȫ������
//���룺�������
//�����Attribute�ṹ����
Attribute Catolog::GetTableAttribute(string tablename)
{
	string tattr = "";
	//�ҵ��������Ǹ�����
	int block;
	int start = GetTablePlace(tablename, block);


	//��ȡ������Ϣ
	char* buffer = buffer_manager.getPage(TABLE_PATH, block);
	string check(buffer);
	
	//�õ����������Ϣ
	int end = 0;
	string attr_name = getTableName(check, start, end);
	//�õ�attribute���ֵ���Ϣ�������ַ���tattr 
	
	Attribute result;

	istringstream instruction = istringstream(tattr);
	string singleword;
	//�������� 
	instruction >> singleword;
	result.amount = String2Num(singleword);
	//��������Ϣ
	for( int i = 0; i < result.amount; i++ )
	{
		//����
		instruction >> singleword;
		result.attr_name[i] = string(singleword); 
		//����
		instruction >> singleword;
		result.attr_type[i] = String2Num( singleword );
		//�Ƿ�Ψһ
		instruction >> singleword;
		if( singleword == "true" )
			result.is_unique[i] = true;
		else
			result.is_unique[i] = false;
	} 
	//���� 
	instruction >> singleword;
	result.primary_key = String2Num( singleword );

	return result;
}

//��ָ�������Ͻ�������
//���룺������ơ��������ơ���������
//�����1-�ɹ��� 0-ʧ��,�����쳣
int Catolog::CreateIndex(string tablename, string tattr, string indexname)
{
	if( isTableExist( tablename ) == false )
	{
		cout << "�����ڸñ��������" << endl;
		return 0; 
	}
	if( isAttributeExist( tablename, tattr) == false )
	{
		cout << "�����ڸ����ԣ��������" << endl;
		return 0; 
	}
	
	//�ж��Ƿ�Խ������ظ� 
	Index cur_index = GetTableIndex(tablename);
	Attribute cur_attr = GetTableAttribute(tablename); 
	int numberofattr = 0;
	
	if( cur_index.amount == 10 )
	{
		cout << "��ǰ�����������Ѵﵽ���ޣ�������Ч" << endl;
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
			cout << "��ǰ�����Ѵ���������������Ч" << endl;
			return 0;			
		}
		if( cur_index.attr_name[i] == indexname )
		{
			cout << "��ǰ�������ѱ�ʹ�ã�������Ч" << endl;
			return 0;			
		}
	} 
	
	//���������ʽ��ʼ�������
	cur_index.amount++;
	cur_index.attr_name[amount-1] = indexname;
	cur_index.whose[amount-1] = numberofattr;
	
	//����ԭ���ı��Ѿ����룬���ܿ϶���֮���Ƿ���������Ϣ��������Ҫ������ɾ��������� 
	if( DropTable(tablename) == 0 )
	{
		cout << "������Ϣʧ�ܣ��޷�ɾ��ԭ��������Ч" << endl;
		return 0		
	}
	if(CreateTable(tablename, cur_attr, cur_indexs, cur_attr.primary_key) == 0)
	{
		cout << "������Ϣʧ�ܣ��޷������������Ч" << endl;
		return 0		
	}
	
	return 1; 
}

//ɾ������
//���룺������ơ���������
//�����1-�ɹ��� 0-ʧ��,�����쳣
int Catolog::DropIndex(string tablename, string indexname)
{
	//�����ڲ������������ͼ���ϸ�ڸ�һ�� 
	if( isTableExist( tablename ) == false )
	{
		cout << "�����ڸñ��������" << endl;
		return 0; 
	}
	
	//�ж��Ƿ�Խ������ظ� 
	Index cur_index = GetTableIndex(tablename);
	Attribute cur_attr = GetTableAttribute(tablename); 
	int numberofindex = 0; 
	 	
	numberofindex = isIndexExist(tablename, indexname);	  
	if( numberofindex == 0 )
	{
		cout << "��ǰ��û�и�������������Ч" << endl;
		return 0;		
	}

	//���������ʽ��ʼɾ������
	cur_index.amount--;
	if( i != amount )
	{
		cur_index.attr_name[i] = cur_index.attr_name[amount];
		cur_index.whose[i] = cur_index.whose[amount];		
	}
	
	//����ԭ���ı��Ѿ����룬���ܿ϶���֮���Ƿ���������Ϣ��������Ҫ������ɾ��������� 
	if( DropTable(tablename) == 0 )
	{
		cout << "ɾ������ʧ�ܣ��޷�ɾ��ԭ��������Ч" << endl;
		return 0		
	}
	if(CreateTable(tablename, cur_attr, cur_indexs, cur_attr.primary_key) == 0)
	{
		cout << "ɾ������ʧ�ܣ��޷������������Ч" << endl;
		return 0		
	}
	
	return 1; 
}

//�����Ƿ����
//���룺������ơ���������
//�����������-������ţ� 0-������		 
int Catolog::isIndexExist(string tablename, string indexname)
{
	Index cur_index = GetTableIndex(tablename);

	for( int i = 0; i < cur_index.amount; i++ )
		if( cur_index.attr_name[i] == indexname )
			return i; 
	
	return 0;
}

//�õ�ĳ���ȫ������,�����ڱ����ʱ�ſ����� 
//���룺�������
//�����Index�ṹ����
Index GetTableIndex(string tablename)
{
	Index result;
	string sindex;
	//�õ��������ڱ��λ��
	//�õ�������Ϣ
	//����Ϣһ��һ��¼��ṹ��
	
	
	return result; 
}

//�õ�����
string GetWord( string input )
{
	string word = "";
	istringstream instruction = istringstream(input);
	instruction >> word;
	return word;
} 

