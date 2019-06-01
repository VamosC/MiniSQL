//
//	catalogmanager.h
//	Ŀ¼����

#ifndef _CATALOGMANAGER_H_
#define _CATALOGMANAGER_H_ 1

#include <iostream>
#include <math>
#include <string>
#include <vector>
#include <sstream>
#include "../base.h" 
using namespace std;

class Catalog{
	private:
		//�ѱ����Ϣд��д���ļ�ʱ��Ҫ�õ�ת���ĺ��� 
		int String2Num(string tmp);
		string Num2String(int tmp);
		
		//�õ����ĳ�����Ϣ�Ŀ��� 
		int GetBlockAmount(string tablename);
		
	public:
		//���ڱ��Ĳ��� 
		
		//�������
		//���룺������ơ�������ԡ������������� 
		//���: 1-�ɹ��� 0-ʧ��,�����쳣 
		int CreateTable(string tablename, Attribute attr, Index indices, int primary_key);
		
		//ɾ�����
		//���룺�������
		//�����1-�ɹ��� 0-ʧ��,�����쳣  
		int DropTable(string tablename);
		 
		//ͨ�������鿴���Ƿ����	
		//���룺�������
		//�����������-������ţ� 0-������	
		int isTableExist(string tablename);
		
		//��ӡ�����Ϣ  ??��������֪����ѯ����ķ�����ʽ 
		void PrintTable(string tablename, Attribute tattr); 
	
	
		//�������Ժ�����
		
		//ĳһ�����Ƿ����
		//���룺������ơ���������
		//�����true-���ڣ� false-������		 
		bool isAttributeExist(string tablename, string tattr);
				
		//�õ�ĳ���ȫ������,�����ڱ����ʱ�ſ����� 
		//���룺�������
		//�����Attribute�ṹ����
		Attribute GetTableAttribute(string tablename);
		
		//��ָ�������Ͻ�������
		//���룺������ơ��������ơ���������
		//�����1-�ɹ��� 0-ʧ��,�����쳣
		int CreateIndex(string tablename, string tattr, string indexname);
		
		//ɾ������
		//���룺������ơ���������
		//�����1-�ɹ��� 0-ʧ��,�����쳣
		int DropIndex(string tablename, string indexname);
		
		//�����Ƿ����
		//���룺������ơ���������
		//�����true-���ڣ� false-������		 
		bool isIndexExist(string tablename, string indexname);
		 
		//�õ�ĳ���ȫ������,�����ڱ����ʱ�ſ����� 
		//���룺�������
		//�����Index�ṹ����
		Index GetTableIndex(string tablename);
};

#endif 
 
