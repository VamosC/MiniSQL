#include "RecordManager.h"
#include <cassert>

void RecordManager::insertRecord(std::string tablename, Tuple& tuple) {
	std::string tmp_tablename = tablename;
	tablename = "./database/data/" + tablename;

	//�쳣���
	if (!catalog_manager.isTableExist(tmp_tablename)) {
		throw TABLE_NOT_EXISTED();
		//�������쳣
	}
	Attribute attr = catalog_manager.GetTableAttribute(tmp_tablename);
	std::vector<Data> v = tuple.getData();
	
	for (int i = 0; i < v.size(); i++) {
		if (v[i].type != attr.attr_type[i])
		{
			throw TUPLE_ATTR_NOT_MATCH();
			//���Բ�ƥ���쳣
		}
			
	}
	Table table = selectRecord(tmp_tablename);
	Index indexs = table.GetIndex();
	std::vector<Tuple>& tuples = table.GetTuples();
	if (attr.primary_key >= 0) {
		if (isConflict(tuples, v, attr.primary_key) == true)
			//������ͻ�쳣
			throw PRIM_KEY_CONFLICT();
	}
	for (int i = 0; i < attr.amount; i++) {
		if (attr.is_unique[i] == true) {
			if (isConflict(tuples, v, i) == true)
				//����unqiue��ͻ�쳣
				throw UNIQUE_CONFLICT();
		}
	}


	//��ȡ����ռ�Ŀ������
	int blockAccount = getBlockNum(tablename);

	if (blockAccount == 0)
		blockAccount = 1;
	
	char* p = buffer_manager.getPage(tablename, blockAccount - 1);
	int i,j;

	for (i = 0; p[i] != '\0' && i < _PAGESIZE; i++);

	int len = 0;
	//��������tuple�ĳ���
	for (j = 0; j < v.size(); j++) {
		Data tmp_data = v[j];

		if (tmp_data.type == INT)
		{
			int l = getDataLength(tmp_data.idata);
			len += l;
		}
		else if (tmp_data.type == FLOAT)
		{
			int l = getDataLength(tmp_data.fdata);
			len += l;
		}
		else
		{
			len += tmp_data.sdata.length();
		}
	}
	len += v.size() + 7;
	int block_offset;

	int PID;
	if (_PAGESIZE - i >= len) {
		block_offset = blockAccount - 1;
		//�����Ԫ��
		DoInsertOnRecord(p, i, len, v);
		//д�ر��ļ�
		PID = buffer_manager.getPageId(tablename, blockAccount - 1);
		buffer_manager.modifyPage(PID);
	}
	//���ʣ��Ŀռ䲻��
	else {
		block_offset = blockAccount;
		//����һ����
		char* p = buffer_manager.getPage(tablename, blockAccount);
		//����Ԫ��
		DoInsertOnRecord(p, 0, len, v);
		//д�ر��ļ�
		PID = buffer_manager.getPageId(tablename, blockAccount);
		buffer_manager.modifyPage(PID);
	}



	//��������
	for (int i = 0; i < indexs.amount; i++)
	{
		std::vector<Data> tmp_data = tuple.getData();
		index_manager.insert_index(tablename, indexs.name[i], tmp_data[i], block_offset);
	}
	
	/*for (int i = 0; i < attr.amount; i++) {
		if (attr.has_index[i] == true) {
			std::string attr_name = attr.attr_name[i];
			std::string FilePath = "INDEX_FILE_" + attr_name + "_" + tmp_tablename;
			std::vector<Data> tmp_data = tuple.getData();
			index_manager.insertIndex(FilePath, tmp_data[i], block_offset);
		}
	}*/
}

int RecordManager::deleteRecord(std::string tablename) {
	std::string tmp_name = tablename;
	tablename = "./database/data/" + tablename;
	//�����Ƿ����
	if (!catalog_manager.isTableExist(tmp_name)) {
		throw TABLE_NOT_EXISTED();
	}
	int blockAccount = getBlockNum(tablename);
	//���ļ���СΪ0ʱֱ�ӷ���
	if (blockAccount == 0)
		return 0;
	Attribute attr = catalog_manager.GetTableAttribute(tmp_name);


	Table table = selectRecord(tmp_name);
	Index indexs = table.GetIndex();
	int count = 0;
	//�������п�
	for (int i = 0; i < blockAccount; i++) {
		//��ȡ��ǰ��ľ��
		char* p = buffer_manager.getPage(tablename, i);
		char* t = p;
		//�����е�ÿһ��Ԫ���¼����Ϊ��ɾ��
		while (*p != '\0' && p < t + _PAGESIZE) {
			//��������
			Tuple tuple = readTuple(p, attr);
			for (int j = 0; i < indexs.amount; i++)
			{
				std::vector<Data> tmp_data = tuple.getData();
				index_manager.delete_index(tablename, indexs.name[i], tmp_data[j]);
			}
			
			//ɾ����¼
			p = SetDeleteOnRecord(p);
			count++;
		}
		//����д�ر��ļ�
		int PID = buffer_manager.getPageId(tablename, i);
		buffer_manager.modifyPage(PID);
	}
	return count;
}

int RecordManager::deleteRecord(std::string tablename, std::string to_attr, Where where) {
	std::string tmp_name = tablename;
	tablename = "./database/data/" + tablename;
	//�����Ƿ����
	if (!catalog_manager.isTableExist(tmp_name)) {
		throw TABLE_NOT_EXISTED();
	}
	Attribute attr = catalog_manager.GetTableAttribute(tmp_name);
	int index = -1;
	bool flag = false;
	Table table = selectRecord(tmp_name);
	Index indexs = table.GetIndex();
	//��ȡĿ�����Զ�Ӧ�ı��
	for (int i = 0; i < attr.amount; i++) 
	{
		if (attr.attr_name[i] == to_attr) 
		{
			index = i;
			break;
		}
	}
	for (int i = 0; i < indexs.amount;i++)
	{
		if (attr.attr_name[index] == indexs.name[i])
		{
			flag = true;
			break;
		}
	}


	//Ŀ�����Բ����ڣ��׳��쳣
	if (index == -1) {
		//Ŀ�����Բ������쳣
		throw ATTR_NOT_EXIST();
	}
	else if (attr.attr_type[index] !=where.data.type) {
		//where�����е��������ݵ����Ͳ�ƥ���쳣
		throw WHERE_TYPE_NOT_MATCH();
	}

	//�쳣�������

	int count = 0;
	//���Ŀ��������������
	if (flag == true && where.relation_character != NOT_EQUAL) {
		std::vector<int> block_ids;
		//ͨ��������ȡ���������ļ�¼���ڵĿ��
		searchWithIndex(tmp_name, attr.attr_name[index], where, block_ids);
		for (int i = 0; i < block_ids.size(); i++) {
			count += queryDeleteInBlock(tmp_name, block_ids[i], attr, index, where);
		}
	}
	else {
		int blockAccount = getBlockNum(tablename);
		//�ļ���СΪ0ֱ�ӷ���
		if (blockAccount == 0)
			return 0;
		//�������еĿ�
		for (int i = 0; i < blockAccount; i++) {
			count += queryDeleteInBlock(tmp_name, i, attr, index, where);
		}
	}
	return count;
}

Table RecordManager::selectRecord(std::string tablename, std::string result_table_name) {
	std::string tmp_name = tablename;
	tablename = "./database/data/" + tablename;
	//�����Ƿ����
	if (!catalog_manager.isTableExist(tmp_name)) {
		throw TABLE_NOT_EXISTED();
	}
	//��ȡ�ļ���ռ�Ŀ������
	int blockAccount = getBlockNum(tablename);
	//�����ļ���СΪ0���������
	if (blockAccount <= 0)
		blockAccount = 1;
	//��ȡ�������
	Attribute attr = catalog_manager.GetTableAttribute(tmp_name);
	//����table���ʵ��
	Table table(result_table_name, attr);
	std::vector<Tuple> & v = table.GetTuples();
	//�������п�
	for (int i = 0; i < blockAccount; i++) {
		//��ȡ��ǰ��ľ��
		char* p = buffer_manager.getPage(tablename, i);
		char* t = p;
		//�����������м�¼
		while (*p != '\0' && p < t + _PAGESIZE) {
			//��ȡ��¼
			Tuple tuple = readTuple(p, attr);
			//�����¼û�б�ɾ����������ӵ�table��
			if (tuple.isDeleted() == false)
				v.push_back(tuple);
			int len = getTupleLength(p);
			p = p + len;
		}
	}
	return table;
}

Table RecordManager::selectRecord(std::string tablename, std::string to_attr, Where where, std::string result_table_name) {
	std::string tmp_name = tablename;
	tablename = "./database/data/" + tablename;

	if (!catalog_manager.isTableExist(tmp_name)) {
		throw TABLE_NOT_EXISTED();
	}
	Attribute attr = catalog_manager.GetTableAttribute(tmp_name);
	int index = -1;
	bool flag = false;
	//��ȡĿ�����Եı��
	Table table = selectRecord(tmp_name);
	Index indexs = table.GetIndex();


	for (int i = 0; i < attr.amount; i++) {
		if (attr.attr_name[i] == to_attr) {
			index = i;
			break;
		}
	}
	for (int i = 0; i < indexs.amount; i++)
	{
		if (attr.attr_name[index] == indexs.name[i])
		{
			flag = true;
			break;
		}
	}
	if (index == -1) {
		//Ŀ�����Բ������쳣
		throw ATTR_NOT_EXIST();
	}
	else if (attr.attr_type[index] != where.data.type) {
		// where�����е��������ݵ����Ͳ�ƥ���쳣
		throw WHERE_TYPE_NOT_MATCH();
	}



	//����table
	Table new_table(result_table_name, attr);
	std::vector<Tuple>& v = new_table.GetTuples();
	if (flag == true && where.relation_character != NOT_EQUAL) {
		std::vector<int> block_ids;
		searchWithIndex(tmp_name, to_attr, where, block_ids);
		for (int i = 0; i < block_ids.size(); i++) {
			querySelectInBlock(tmp_name, block_ids[i], attr, index, where, v);
		}
	}
	else {
		//��ȡ�ļ���ռ�������
		int blockAccount = getBlockNum(tablename);
		if (blockAccount == 0)
			blockAccount = 1;
		//�������п�
		for (int i = 0; i < blockAccount; i++) {
			querySelectInBlock(tmp_name, i, attr, index, where, v);
		}
	}
	return new_table;
}

void RecordManager::createIndex(const std::string &table_name, const std::string &index_name, const std::string &attr) {
	auto file_path = "./database/data/" + table_name;
	Attribute attr_t = catalog_manager.GetTableAttribute(table_name);
	int index = -1;
	//��ȡĿ�����Եı��
	for (int i = 0; i < attr_t.amount; i++) {
		if (attr_t.attr_name[i] == attr) {
			index = i;
			break;
		}
	}
	//��ȡ�ļ���ռ�Ŀ������
	int blockAccount = getBlockNum(file_path);
	//�����ļ���СΪ0���������
	if (blockAccount == 0)
		blockAccount = 1;
	//��ȡ�������
	//std::string FilePath = "INDEX_FILE_" + to_attr + "_" + tmp_name;
	//�������п�
	for (int i = 0; i < blockAccount; i++) {
		//��ȡ��ǰ��ľ��
		char* p = buffer_manager.getPage(file_path, i);
		char* t = p;
		//�����������м�¼
		while (*p != '\0' && p < t + _PAGESIZE) {
			//��ȡ��¼
			Tuple tuple = readTuple(p, attr_t);
			if (tuple.isDeleted() == false) {
				std::vector<Data> v = tuple.getData();
				//ͨ��������
				index_manager.insert_index(table_name, index_name, v[index], i);
			}
			p = p + getTupleLength(p);
		}
	}
}

//��ȡ�ļ���С
int RecordManager::getBlockNum(std::string tablename) {
	char* p;
	int blockAccount = -1;
	do {
		p = buffer_manager.getPage(tablename, blockAccount + 1);
		blockAccount++;
	} while (p[0] != '\0');
	return blockAccount;
}

void RecordManager::DoInsertOnRecord(char* p, int offset, int len, const std::vector<Data> & v) {
	std::stringstream stream;
	stream << len;
	std::string s = stream.str();
	while (s.length() < 4)
		s = "0" + s;
	for (int j = 0; j < s.length(); j++)
		p[offset++] = s[j];
	for (int j = 0; j < v.size(); j++) {
		p[offset++] = ' ';
		Data d = v[j];
		if (d.type == INT)
		{
			CopyFunc(p, offset, d.idata);
		}
		else if (d.type == FLOAT)
		{
			CopyFunc(p, offset, d.fdata);
		}
		else
		{
			CopyFunc(p, offset, d.sdata);
		}
	}
	p[offset] = ' ';
	p[offset + 1] = '0';
	p[offset + 2] = '\n';
}

char* RecordManager::SetDeleteOnRecord(char* p) {
	p = p + getTupleLength(p);
	*(p - 2) = IS_DELETED;
	return p;
}

//���ڴ��ж�ȡһ��tuple
Tuple RecordManager::readTuple(const char* p, Attribute attr) {
	Tuple tuple;
	p = p + 5;
	for (int i = 0; i < attr.amount; i++) {
		Data data;
		data.type = attr.attr_type[i];
		char tmp[100];
		int j;
		for (j = 0; *p != ' '; j++, p++) {
			tmp[j] = *p;
		}
		tmp[j] = '\0';
		p++;
		std::string s(tmp);
		if (data.type == INT)
		{
			std::stringstream stream(s);
			stream >> data.idata;
		}
		else if (data.type == FLOAT)
		{
			std::stringstream stream(s);
			stream >> data.fdata;
		}
		else
		{
			data.sdata = s;
		}
		tuple.addData(data);
	}
	if (*p == IS_DELETED)
		tuple.setDeleted();
	return tuple;
}

//��ȡһ��tuple�ĳ���
int RecordManager::getTupleLength(char* p) {
	int i;
	char tmp[100];
	for (i = 0; p[i] != ' '; i++)
		tmp[i] = p[i];
	tmp[i] = '\0';
	std::string s=tmp;
	return (int)stoi(s);
}

//�жϲ���ļ�¼�Ƿ��ͻ
bool RecordManager::isConflict(std::vector<Tuple> & tuples, std::vector<Data> & v, int index) {
	for (int i = 0; i < tuples.size(); i++) {
		if (tuples[i].isDeleted() == true)
			continue;
		std::vector<Data> d = tuples[i].getData();
		if (v[index].type == INT)
		{
			if (v[index].idata == d[index].idata)
				return true;
		}
		else if (v[index].type == FLOAT)
		{
			if (v[index].fdata == d[index].fdata)
				return true;
		}
		else
		{
			if (v[index].sdata == d[index].sdata)
				return true;
		}
	}
	return false;
}

//����������
void RecordManager::searchWithIndex(const std::string &table_name, const std::string &index_name, Where where, std::vector<int> &block_ids) 
{
	if(where.relation_character == LESS || where.relation_character == LESS_OR_EQUAL) {
		condition cond;
		cond.l_op = -1; // <
		if (where.relation_character == LESS)
		{
			cond.r_op = 2;
		}
		else
		{
			cond.r_op = 4;
		}
		cond.end = where.data; // 22

		index_manager.find_range_key(table_name, index_name, cond, block_ids);
	}
	else if(where.relation_character == GREATER || where.relation_character == GREATER_OR_EQUAL) {
		condition cond;
		cond.r_op = -1;
		if (where.relation_character == GREATER)
		{
			cond.l_op = 2;
		}
		else
		{
			cond.l_op = 4;
		}
		cond.start = where.data; // 22
		index_manager.find_range_key(table_name, index_name, cond, block_ids);
	}
	else if(where.relation_character == EQUAL)
	{
		index_manager.find_key(table_name, index_name, where.data, block_ids);
	}
	else
	{
		// �����ܳ����������
		// ������һ���Ǳ�̳�����
		assert(false);
	}
}

//�ڿ��н�������ɾ��
int RecordManager::queryDeleteInBlock(std::string tablename, int block_id, Attribute attr, int index, Where where) {
	//��ȡ��ǰ��ľ��
	tablename = "./database/data/" + tablename;//����
	char* p = buffer_manager.getPage(tablename, block_id);
	char* t = p;
	int count = 0;
	//�����������м�¼
	while (*p != '\0' && p < t + _PAGESIZE) {
		//��ȡ��¼
		Tuple tuple = readTuple(p, attr);
		std::vector<Data> d = tuple.getData();
		//������������ִ�в���
		if (attr.attr_type[index] == 0)
		{
			if (QueryJudge(d[index].idata, where.data.idata, where.relation_character))
			{
				//����¼ɾ��
				p = SetDeleteOnRecord(p);
				count++;
			}
			else
			{
				p = p + getTupleLength(p);
			}
		}
		else if (attr.attr_type[index] == 0)
		{
			if (QueryJudge(d[index].fdata, where.data.fdata, where.relation_character)) {
				p = SetDeleteOnRecord(p);
				count++;
			}
			else {
				p = p + getTupleLength(p);
			}
		}
		else
		{
			if (QueryJudge(d[index].sdata, where.data.sdata, where.relation_character)) {
				p = SetDeleteOnRecord(p);
				count++;
			}
			else {
				p = p + getTupleLength(p);
			}
		}
	}
	int PID = buffer_manager.getPageId(tablename, block_id);
	buffer_manager.modifyPage(PID);
	return count;
}

//�ڿ��н���������ѯ
void RecordManager::querySelectInBlock(std::string tablename, int block_id, Attribute attr, int index, Where where, std::vector<Tuple> & v) {
	//��ȡ��ľ��
	tablename = "./database/data/" + tablename;
	char* p = buffer_manager.getPage(tablename, block_id);
	char* t = p;
	//�������м�¼
	while (*p != '\0' && p < t + _PAGESIZE) {
		//��ȡ��¼
		Tuple tuple = readTuple(p, attr);
		//�����¼�ѱ�ɾ���������ü�¼
		if (tuple.isDeleted() == true) {
			int len = getTupleLength(p);
			p = p + len;
			continue;
		}
		std::vector<Data> d = tuple.getData();

		if (attr.attr_type[index] == INT)
		{
			if (QueryJudge(d[index].idata, where.data.idata, where.relation_character)) 
			{
				v.push_back(tuple);
			}
		}
		else if (attr.attr_type[index] == FLOAT)
		{
			if (QueryJudge(d[index].fdata, where.data.fdata, where.relation_character)) {
				v.push_back(tuple);
			}
		}
		else
		{
			if (QueryJudge(d[index].sdata, where.data.sdata, where.relation_character)) {
				v.push_back(tuple);
			}
		}
		p = p + getTupleLength(p);
	}
}

void RecordManager::createTableFile(const std::string &table_name) {
	auto file_path = "./database/data/"+table_name;
	FILE* f = fopen(file_path.c_str(), "w");
	if (f == NULL)
	{
		throw minisql_exception("Can not create the table file!");
	}
	fclose(f);
}

void RecordManager::dropTableFile(const std::string &table_name) {
	auto file_path = "./database/data/" + table_name;
	remove(file_path.c_str());
}


