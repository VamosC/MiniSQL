#include "RecordManager.h"

void RecordManager::insertRecord(std::string tablename, Tuple& tuple) {
	std::string tmp_tablename = tablename;
	tablename = "./database/data/" + tablename;
	Catalog catalog_manager;
	

	//�쳣���
	if (!catalog_manager.isTableExist(tmp_tablename)) {
		throw TABLE_NOT_EXISTED();
		//�������쳣
	}
	Attribute attr = catalog_manager.GetTableAttribute(tmp_tablename);
	std::vector<Data> v = tuple.getData();
	
	for (int i = 0; i < v.size(); i++) {
		if (v[i].attr_type != attr.attr_type[i])
		{
			throw TUPLE_ATTR_NOT_MATCH();
			//���Բ�ƥ���쳣
		}
			
	}
	Table table = selectRecord(tmp_tablename);
	std::vector<Tuple>& tuples = table.getTuple();
	if (attr.primary_key >= 0) {
		if (isConflict(tuples, v, attr.primary_key) == true)
			//������ͻ�쳣
			throw PRIM_KEY_CONFLICT();
	}
	for (int i = 0; i < attr.amount; i++) {
		if (attr.is_unique[i] == true) {
			if (isConflict(tuples, v, i) == true)
				//����unqiue��ͻ�쳣
				trow UNIQUE_CONFLICT();
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
			int t = getDataLength(tmp_data.fdata);
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
	IndexManager index_manager(tmp_tablename);
	for (int i = 0; i < attr.amount; i++) {
		if (attr.has_index[i] == true) {
			std::string attr_name = attr.attr_name[i];
			std::string FilePath = "INDEX_FILE_" + attr_name + "_" + tmp_tablename;
			std::vector<Data> tmp_data = tuple.getData();
			index_manager.insertIndex(FilePath, tmp_data[i], block_offset);
		}
	}
}

int RecordManager::deleteRecord(std::string tablename) {
	std::string tmp_name = tablename;
	tablename = "./database/data/" + tablename;
	Catalog catalog_manager;
	//�����Ƿ����
	if (!catalog_manager.isTableExist(tmp_name)) {
		throw TABLE_NOT_EXISTED();
	}
	int blockAccount = getBlockNum(tablename);
	//���ļ���СΪ0ʱֱ�ӷ���
	if (blockAccount == 0)
		return 0;
	Attribute attr = catalog_manager.GetTableAttribute(tmp_name);
	IndexManager index_manager(tmp_name);
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
			for (int j = 0; j < attr.amount; j++) {
				if (attr.has_index[j] == true) {
					std::string attr_name = attr.attr_name[i];
					std::string FilePath = "INDEX_FILE_" + attr_name + "_" + tmp_name;
					std::vector<Data> d = tuple.getData();
					

					//��������ɾ�����˴�ûд
				}
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
	Catalog catalog_manager;
	//�����Ƿ����
	if (!catalog_manager.isTableExist(tmp_name)) {
		throw TABLE_NOT_EXISTED();
	}
	Attribute attr = catalog_manager.GetTableAttribute(tmp_name);
	int index = -1;
	bool flag = false;
	//��ȡĿ�����Զ�Ӧ�ı��
	for (int i = 0; i < attr.amount; i++) {
		if (attr.attr_name[i] == attr) {
			index = i;
			if (attr.has_index[i] == true)
				flag = true;
			break;
		}
	}
	//Ŀ�����Բ����ڣ��׳��쳣
	if (index == -1) {
		//Ŀ�����Բ������쳣
		throw ATTR_NOT_EXIST()
	}
	else if (attr.attr_type[index] != where.data.attr_type) {
		//where�����е��������ݵ����Ͳ�ƥ���쳣
		throw WHERE_TYPE_NOT_MATCH();
	}

	//�쳣�������

	int count = 0;
	//���Ŀ��������������
	if (flag == true && where.relation_character != NOT_EQUAL) {
		std::vector<int> block_ids;
		//ͨ��������ȡ���������ļ�¼���ڵĿ��
		searchWithIndex(tmp_name, attr, where, block_ids);
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
	Catalog catalog_manager;
	//�����Ƿ����
	if (!catalog_manager.isTableExist(tmp_name)) {
		throw TABLE_NOT_EXISTED();
	}
	//��ȡ�ļ���ռ�Ŀ������
	// int blockAccount = getFileSize(tablename) / _PAGESIZE;
	// ��Ϊ
	int blockAccount = getBlockNum(tablename);
	//�����ļ���СΪ0���������
	if (blockAccount <= 0)
		blockAccount = 1;
	//��ȡ�������
	Attribute attr = catalog_manager.GetTableAttribute(tmp_name);
	//����table���ʵ��
	Table table(result_table_name, attr);
	std::vector<Tuple> & v = table.getTuple();
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
	Catalog catalog_manager;

	if (!catalog_manager.isTableExist(tmp_name)) {
		throw TABLE_NOT_EXISTED();
	}
	Attribute attr = catalog_manager.GetTableAttribute(tmp_name);
	int index = -1;
	bool flag = false;
	//��ȡĿ�����Եı��
	for (int i = 0; i < attr.amount; i++) {
		if (attr.attr_name[i] == to_attr) {
			index = i;
			if (attr.has_index[i] == true)
				flag = true;
			break;
		}
	}

	if (index == -1) {
		//Ŀ�����Բ������쳣
		throw ATTR_NOT_EXIST();
	}
	else if (attr.attr_type[index] != where.data.attr_type) {
		// where�����е��������ݵ����Ͳ�ƥ���쳣
		throw WHERE_TYPE_NOT_MATCH();
	}



	//����table
	Table table(result_table_name, attr);
	std::vector<Tuple>& v = table.getTuple();
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
	return table;
}

void RecordManager::createIndex(IndexManager & index_manager, std::string tablename, std::string to_attr) {
	std::string tmp_name = tablename;
	tablename = "./database/data/" + tablename;
	Catalog catalog_manager;
	if (!catalog_manager.isTableExist(tmp_name)) {
		//�������쳣
		throw TABLE_NOT_EXISTED();
	}
	Attribute attr = catalog_manager.GetTableAttribute(tmp_name);
	int index = -1;
	//��ȡĿ�����Եı��
	for (int i = 0; i < attr.amount; i++) {
		if (attr.attr_name[i] == to_attr) {
			index = i;
			break;
		}
	}
	if (index == -1) {
		//Ŀ�����Բ������쳣
		throw ATTR_NOT_EXIST();

	}

	//��ȡ�ļ���ռ�Ŀ������
	int blockAccount = getBlockNum(tablename);
	//�����ļ���СΪ0���������
	if (blockAccount == 0)
		blockAccount = 1;
	//��ȡ�������
	std::string FilePath = "INDEX_FILE_" + to_attr + "_" + tmp_name;
	//�������п�
	for (int i = 0; i < blockAccount; i++) {
		//��ȡ��ǰ��ľ��
		char* p = buffer_manager.getPage(tablename, i);
		char* t = p;
		//�����������м�¼
		while (*p != '\0' && p < t + _PAGESIZE) {
			//��ȡ��¼
			Tuple tuple = readTuple(p, attr);
			if (tuple.isDeleted() == false) {
				std::vector<Data> v = tuple.getData();
				index_manager.insertIndex(FilePath, v[index], i);
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
		if (d.attr_type == INT)
		{
			CopyFunc(p, offset, d.datai);
		}
		else if (d.attr_type == FLOAT)
		{
			CopyFunc(p, offset, d.dataf);
		}
		else
		{
			CopyFunc(p, offset, d.datas);
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
		data.attr_type = attr.attr_type[i];
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
			stream >> data.datai;
		}
		else if (data.type == FLOAT)
		{
			std::stringstream stream(s);
			stream >> data.dataf;
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
			if (v[index].sdata == f[index].sdata)
				return true;
		}
	}
	return false;
}

//����������
void RecordManager::searchWithIndex(std::string tablename, std::string to_attr, Where where, std::vector<int> & block_ids) {
	
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
			if (QueryJudge(d[index].datai, where.data.datai, where.relation_character))
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
			if (QueryJudge(d[index].dataf, where.data.dataf, where.relation_character)) {
				p = SetDeleteOnRecord(p);
				count++;
			}
			else {
				p = p + getTupleLength(p);
			}
		}
		else
		{
			if (QueryJudge(d[index].datas, where.data.datas, where.relation_character)) {
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

		if (attr.attr_type[indxe] == INT)
		{
			if (QueryJudge(d[index].datai, where.data.datai, where.relation_character)) 
			{
				v.push_back(tuple);
			}
		}
		else if (attr.attr_type[indxe] == FLOAT)
		{
			if (QueryJudge(d[index].dataf, where.data.dataf, where.relation_character)) {
				v.push_back(tuple);
			}
		}
		else
		{
			if (QueryJudge(d[index].datas, where.data.datas, where.relation_character)) {
				v.push_back(tuple);
			}
		}
		p = p + getTupleLength(p);
	}
}

void RecordManager::createTableFile(std::string tablename) {
	tablename = "./database/data/" + tablename;
	FILE* f = fopen(tablename.c_str(), "w");
	if (f == NULL)
	{
		cout << "Can not creat the file." << endl;
	}
	fclose(f);
}

void RecordManager::dropTableFile(std::string tablename) {
	tablename = "./database/data/" + tablename;
	remove(tablename.c_str());
}


