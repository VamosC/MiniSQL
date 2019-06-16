#include "RecordManager.h"
#include <cassert>

void RecordManager::insertRecord(const std::string &table_name, Tuple& tuple) {
	auto file_path = "./database/data/" + table_name;

	// ��һ������
	// Interpreter����

	Attribute attr = catalog_manager.GetTableAttribute(table_name);
	std::vector<Data> v = tuple.getData();

	if(v.size() != attr.amount)
	{
		throw minisql_exception("Attribute number not match!");
	}

	for (int i = 0; i < v.size(); i++) {
		if (v[i].type != attr.attr_type[i])
		{
			//���Բ�ƥ���쳣
			throw minisql_exception("Attribute type not match!");
		}
			
	}
	Table table = selectRecord(table_name);
	std::vector<Tuple>& tuples = table.GetTuples();
	if (attr.primary_key >= 0) {
		if (isConflict(tuples, v, attr.primary_key))
			//������ͻ�쳣
			throw minisql_exception("Primary key conflict!");
	}
	for (int i = 0; i < attr.amount; i++) {
		if (attr.is_unique[i] == true) {
			if (isConflict(tuples, v, i))
				//����unqiue��ͻ�쳣
				throw minisql_exception("Unique attribute conflict!");
		}
	}


	//��ȡ����ռ�Ŀ������
	int blockAccount = getBlockNum(file_path);

	if (blockAccount == 0)
	{
		blockAccount = 1;
	}
	
	char* p = buffer_manager.getPage(file_path, blockAccount - 1);
	int i,j;

	for (i = 0; p[i] != '\0' && i < _PAGESIZE; i++);

	int len = 0;
	//��������tuple�ĳ���
	for (j = 0; j < v.size(); j++) {
		if (v[j].type == INT)
		{
			int l = getDataLength(v[j].idata);
			len += l;
		}
		else if (v[j].type == FLOAT)
		{
			int l = getDataLength(v[j].fdata);
			len += l;
		}
		else
		{
			len += v[j].sdata.length();
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
		PID = buffer_manager.getPageId(file_path, blockAccount - 1);
		buffer_manager.modifyPage(PID);
	}
	//���ʣ��Ŀռ䲻��
	else {
		block_offset = blockAccount;
		//����һ����
		char* p = buffer_manager.getPage(file_path, blockAccount);
		//����Ԫ��
		DoInsertOnRecord(p, 0, len, v);
		//д�ر��ļ�
		PID = buffer_manager.getPageId(file_path, blockAccount);
		buffer_manager.modifyPage(PID);
	}

	auto indexs = catalog_manager.GetTableIndex(table_name);
	//��������
	for (int i = 0; i < indexs.amount; i++)
	{
		std::vector<Data> tmp_data = tuple.getData();
		index_manager.insert_index(table_name, indexs.name[i], tmp_data[indexs.whose[i]], block_offset);
	}
}

int RecordManager::deleteRecord(const std::string &table_name) {
	auto file_path = "./database/data/" + table_name;
	// API������Ƿ����

	int blockAccount = getBlockNum(file_path);
	//���ļ���СΪ0ʱֱ�ӷ���
	if (blockAccount == 0)
		return 0;
	Attribute attr = catalog_manager.GetTableAttribute(table_name);
	Table table = selectRecord(table_name);
	auto index = catalog_manager.GetTableIndex(table_name);
	int count = 0;
	//�������п�
	for (int i = 0; i < blockAccount; i++) {
		//��ȡ��ǰ��ľ��
		char* p = buffer_manager.getPage(file_path, i);
		char* t = p;
		//�����е�ÿһ��Ԫ���¼����Ϊ��ɾ��
		while (*p != '\0' && p < t + _PAGESIZE) {
			//��������
			Tuple tuple = readTuple(p, attr);
			for (int j = 0; j < index.amount; j++)
			{
				std::vector<Data> tmp_data = tuple.getData();
				// index_manager.delete_index(table_name, index.name[j]);
				index_manager.delete_index(table_name, index.name[j], tmp_data[index.whose[j]]);
			}
			
			//ɾ����¼
			p = SetDeleteOnRecord(p);
			count++;
		}
		//����д�ر��ļ�
		int PID = buffer_manager.getPageId(file_path, i);
		buffer_manager.modifyPage(PID);
	}
	return count;
}

int RecordManager::deleteRecord(const std::string &table_name, SelectCondition scondition) {
	auto file_path = "./database/data/" + table_name;
	// API�����Ƿ����
	// ���Ŀ�������Ƿ�����Լ��Ƿ�ƥ��
	Attribute attr = catalog_manager.GetTableAttribute(table_name);
	std::vector<int> attr_positon;
	std::map<int, std::string> indexs;
	for(auto i = 0; i < scondition.amount; i++)
	{
		auto attr_pos = catalog_manager.isAttributeExist(table_name, scondition.attr[i]);
		if(attr_pos == -1)
		{
			throw minisql_exception("Attribute " + scondition.attr[i] + " not exists!");
		}
		if(attr.attr_type[attr_pos] != scondition.key[i].type)
		{
			throw minisql_exception("Attribute type not match!");
		}
		attr_positon.push_back(attr_pos);
	}
	auto index = catalog_manager.GetTableIndex(table_name);
	std::vector<int> block_ids;
	auto flag = false;
	for(auto k = 0; k < attr_positon.size(); k++)
	{
		// ��������Ƿ����
		for (int i = 0; i < index.amount; i++)
		{
			if (index.whose[i] == attr_positon[k])
			{
				if(op_table[scondition.operationtype[k]] != NOT_EQUAL)
				{
					flag = true;
					if(indexs.count(k) == 0)
					{
						indexs[k] = index.name[i];
					}
					break;
				}
			}
		}
	}
	int count = 0;
	//���Ŀ��������������
	if (flag) 
	{
		for(auto it : indexs)
		{
			auto where = Where{.data = scondition.key[it.first], .relation_character = op_table[scondition.operationtype[it.first]]};
			//ͨ��������ȡ���������ļ�¼���ڵĿ��
			std::vector<int> tmp;
			searchWithIndex(table_name, it.second, where, tmp);
			combine(tmp, block_ids);
			removeDuplicate(block_ids);
		}
		for (int i = 0; i < block_ids.size(); i++) {
			count += queryDeleteInBlock(table_name, block_ids[i], attr, attr_positon, scondition);
		}
	}
	else 
	{
		int blockAccount = getBlockNum(file_path);
		//�ļ���СΪ0ֱ�ӷ���
		if (blockAccount == 0)
			return 0;
		//�������еĿ�
		for (int i = 0; i < blockAccount; i++) {
			count += queryDeleteInBlock(table_name, i, attr, attr_positon, scondition);
		}
	}
	return count;
}

Table RecordManager::selectRecord(const std::string &table_name, std::string result_table_name) {
	auto file_path = "./database/data/" + table_name;
	// ��������Ƿ����

	// ��ȡ�ļ���ռ�Ŀ������
	int blockAccount = getBlockNum(file_path);
	// �����ļ���СΪ0���������
	// ����Ҫ����
	// ��ȡ�������
	Attribute attr = catalog_manager.GetTableAttribute(table_name);
	//����table���ʵ��
	Table table(result_table_name, attr);
	std::vector<Tuple> &v = table.GetTuples();
	//�������п�
	for (int i = 0; i < blockAccount; i++) {
		//��ȡ��ǰ��ľ��
		char* p = buffer_manager.getPage(file_path, i);
		char* t = p;
		//�����������м�¼
		while (*p != '\0' && p < t + _PAGESIZE) {
			//��ȡ��¼
			Tuple tuple = readTuple(p, attr);
			//�����¼û�б�ɾ����������ӵ�table��
			if (!tuple.isDeleted())
			{
				v.push_back(tuple);
			}
			int len = getTupleLength(p);
			p += len;
		}
	}
	return table;
}

Table RecordManager::selectRecord(const std::string &table_name, SelectCondition scondition, std::string result_table_name) {
	auto file_path = "./database/data/" + table_name;
	// API������Ƿ����
	// Ҳ���������Ƿ����
	Attribute attr = catalog_manager.GetTableAttribute(table_name);
	std::vector<int> attr_positon;
	std::map<int, std::string> indexs;
	for(auto i = 0; i < scondition.amount; i++)
	{
		auto attr_pos = catalog_manager.isAttributeExist(table_name, scondition.attr[i]);
		if(attr.attr_type[attr_pos] != scondition.key[i].type)
		{
			throw minisql_exception("Attribute type not match!");
		}
		attr_positon.push_back(attr_pos);
	}
	auto index = catalog_manager.GetTableIndex(table_name);
	std::vector<int> block_ids;
	auto flag = false;
	for(auto k = 0; k < attr_positon.size(); k++)
	{
		// ��������Ƿ����
		for (int i = 0; i < index.amount; i++)
		{
			if (index.whose[i] == attr_positon[k])
			{
				if(op_table[scondition.operationtype[k]] != NOT_EQUAL)
				{
					flag = true;
					if(indexs.count(k) == 0)
					{
						indexs[k] = index.name[i];
					}
					break;
				}
			}
		}
	}
	//����table
	Table new_table(result_table_name, attr);
	std::vector<Tuple>& v = new_table.GetTuples();
	if (flag == true) {
		for(auto it : indexs)
		{
			auto where = Where{.data = scondition.key[it.first], .relation_character = op_table[scondition.operationtype[it.first]]};
			//ͨ��������ȡ���������ļ�¼���ڵĿ��
			std::vector<int> tmp;
			searchWithIndex(table_name, it.second, where, tmp);
			combine(tmp, block_ids);
			removeDuplicate(block_ids);
		}
		for (int i = 0; i < block_ids.size(); i++) {
			querySelectInBlock(table_name, block_ids[i], attr, attr_positon, scondition, v);
		}
	}
	else {
		//��ȡ�ļ���ռ�������
		int blockAccount = getBlockNum(file_path);
		//�������п�
		for (int i = 0; i < blockAccount; i++) {
			querySelectInBlock(table_name, i, attr, attr_positon, scondition, v);
		}
	}
	return new_table;
}

void RecordManager::createIndex(const std::string &table_name, const std::string &index_name, const std::string &attr) {
	auto file_path = "./database/data/" + table_name;
	Attribute attr_t = catalog_manager.GetTableAttribute(table_name);
	// Ŀ�����Ա��
	int index = -1;
	// ��ȡĿ�����Եı��
	for (int i = 0; i < attr_t.amount; i++) {
		if (attr_t.attr_name[i] == attr) {
			index = i;
			break;
		}
	}
	// ��ȡ�ļ���ռ�Ŀ������
	int blockAccount = getBlockNum(file_path);
	// �ļ���СΪ0��������� ����Ҫ����
	// �������п�
	int j = 0;
	for (int i = 0; i < blockAccount; i++) {
		//��ȡ��ǰ��ľ��
		char* p = buffer_manager.getPage(file_path, i);
		char* t = p;
		//�����������м�¼
		while (*p != '\0' && p < t + _PAGESIZE) {
			//��ȡ��¼
			Tuple tuple = readTuple(p, attr_t);
			if (!tuple.isDeleted()) {
				std::vector<Data> v = tuple.getData();
				index_manager.insert_index(table_name, index_name, v[index], i);
			}
			p = p + getTupleLength(p);
		}
	}
}

//��ȡ�ļ���С
int RecordManager::getBlockNum(const std::string &table_name) {
	char* p;
	int blockAccount = -1;
	do {
		p = buffer_manager.getPage(table_name, blockAccount + 1);
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
	p += getTupleLength(p);
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
		char tmp[256];
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
		// �����ܳ���ɾ������Ԫ��
		if(tuples[i].isDeleted())
		{
			continue;
		}
		auto d = tuples[i].getData();
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
void RecordManager::searchWithIndex(const std::string &table_name, const std::string &index_name, const Where &where, std::vector<int> &block_ids) 
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
int RecordManager::queryDeleteInBlock(const std::string &table_name, int block_id, const Attribute &attr, std::vector<int> &indexs, const SelectCondition &cond) {
	//��ȡ��ǰ��ľ��
	auto file_path = "./database/data/" + table_name;
	char* p = buffer_manager.getPage(file_path, block_id);
	char* t = p;
	int count = 0;
	//�����������м�¼
	while (*p != '\0' && p < t + _PAGESIZE) {
		//��ȡ��¼
		Tuple tuple = readTuple(p, attr);
		if(tuple.isDeleted())
		{
			p += getTupleLength(p);
			continue;
		}
		std::vector<Data> d = tuple.getData();
		//������������ִ�в���
		auto flag = true;
		for(auto i = 0; i < indexs.size(); i++)
		{
			if (attr.attr_type[indexs[i]] == INT)
			{
				if (!QueryJudge(d[indexs[i]].idata, cond.key[i].idata, op_table[cond.operationtype[i]]))
				{
					flag = false;
					break;
				}
			}
			else if (attr.attr_type[indexs[i]] == FLOAT)
			{
				if (!QueryJudge(d[indexs[i]].fdata, cond.key[i].fdata, op_table[cond.operationtype[i]]))
				 {
					flag = false;
					break;
				}
			}
			else
			{
				if (!QueryJudge(d[indexs[i]].sdata, cond.key[i].sdata, op_table[cond.operationtype[i]])) 
				{
					flag = false;
					break;
				}
			}
		}
		if(flag)
		{
			p = SetDeleteOnRecord(p);
			count++;
		}
		else
		{
			p += getTupleLength(p);
		}
	}
	int PID = buffer_manager.getPageId(file_path, block_id);
	buffer_manager.modifyPage(PID);
	return count;
}

//�ڿ��н���������ѯ
void RecordManager::querySelectInBlock(const std::string &table_name, int block_id, const Attribute &attr, std::vector<int> &indexs, const SelectCondition &cond, std::vector<Tuple> & v) {
	//��ȡ��ľ��
	auto file_path = "./database/data/" + table_name;
	char* p = buffer_manager.getPage(file_path, block_id);
	char* t = p;
	//�������м�¼
	while (*p != '\0' && p < t + _PAGESIZE) {
		//��ȡ��¼
		Tuple tuple = readTuple(p, attr);
		//�����¼�ѱ�ɾ���������ü�¼
		if (tuple.isDeleted()) 
		{
			p = p + getTupleLength(p);
			continue;
		}
		std::vector<Data> d = tuple.getData();
		auto flag = true;
		for(auto i = 0; i < indexs.size(); i++)
		{
			if (attr.attr_type[indexs[i]] == INT)
			{
				if (!QueryJudge(d[indexs[i]].idata, cond.key[i].idata, op_table[cond.operationtype[i]]))
				{
					flag = false;
					break;
				}
			}
			else if (attr.attr_type[indexs[i]] == FLOAT)
			{
				if (!QueryJudge(d[indexs[i]].fdata, cond.key[i].fdata, op_table[cond.operationtype[i]]))
				 {
					flag = false;
					break;
				}
			}
			else
			{
				if (!QueryJudge(d[indexs[i]].sdata, cond.key[i].sdata, op_table[cond.operationtype[i]])) 
				{
					flag = false;
					break;
				}
			}
		}
		if(flag)
		{
			v.push_back(tuple);
		}
		p += getTupleLength(p);
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

void RecordManager::removeDuplicate(std::vector<int> &block_ids)
{
	std::sort(block_ids.begin(), block_ids.end());
	auto it = block_ids.begin();
	while(it != block_ids.end() && it+1 != block_ids.end())
	{
		if(*it == *(it+1))
		{
			it = block_ids.erase(it);
		}
		else
		{
			it++;
		}
	}
}

void RecordManager::combine(std::vector<int> &tmp, std::vector<int> &block_ids)
{
	for(auto i = 0; i < tmp.size(); i++)
	{
		block_ids.push_back(tmp[i]);
	}
}

