#include "RecordManager.h"

void RecordManager::insertRecord(std::string tablename, Tuple& tuple) {
	std::string tmp_tablename = tablename;
	tablename = "./database/data/" + tablename;
	Catalog catalog_manager;
	

	//异常检测
	if (!catalog_manager.isTableExist(tmp_tablename)) {
		throw TABLE_NOT_EXISTED();
		//表不存在异常
	}
	Attribute attr = catalog_manager.GetTableAttribute(tmp_tablename);
	std::vector<Data> v = tuple.getData();
	
	for (int i = 0; i < v.size(); i++) {
		if (v[i].attr_type != attr.attr_type[i])
		{
			throw TUPLE_ATTR_NOT_MATCH();
			//属性不匹配异常
		}
			
	}
	Table table = selectRecord(tmp_tablename);
	std::vector<Tuple>& tuples = table.getTuple();
	if (attr.primary_key >= 0) {
		if (isConflict(tuples, v, attr.primary_key) == true)
			//主键冲突异常
			throw PRIM_KEY_CONFLICT();
	}
	for (int i = 0; i < attr.amount; i++) {
		if (attr.is_unique[i] == true) {
			if (isConflict(tuples, v, i) == true)
				//存在unqiue冲突异常
				trow UNIQUE_CONFLICT();
		}
	}


	//获取表所占的块的数量
	int blockAccount = getBlockNum(tablename);

	if (blockAccount == 0)
		blockAccount = 1;
	
	char* p = buffer_manager.getPage(tablename, blockAccount - 1);
	int i,j;

	for (i = 0; p[i] != '\0' && i < _PAGESIZE; i++);

	int len = 0;
	//计算插入的tuple的长度
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
		//插入该元组
		DoInsertOnRecord(p, i, len, v);
		//写回表文件
		PID = buffer_manager.getPageId(tablename, blockAccount - 1);
		buffer_manager.modifyPage(PID);
	}
	//如果剩余的空间不够
	else {
		block_offset = blockAccount;
		//新增一个块
		char* p = buffer_manager.getPage(tablename, blockAccount);
		//插入元组
		DoInsertOnRecord(p, 0, len, v);
		//写回表文件
		PID = buffer_manager.getPageId(tablename, blockAccount);
		buffer_manager.modifyPage(PID);
	}

	//更新索引
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
	//检测表是否存在
	if (!catalog_manager.isTableExist(tmp_name)) {
		throw TABLE_NOT_EXISTED();
	}
	int blockAccount = getBlockNum(tablename);
	//表文件大小为0时直接返回
	if (blockAccount == 0)
		return 0;
	Attribute attr = catalog_manager.GetTableAttribute(tmp_name);
	IndexManager index_manager(tmp_name);
	int count = 0;
	//遍历所有块
	for (int i = 0; i < blockAccount; i++) {
		//获取当前块的句柄
		char* p = buffer_manager.getPage(tablename, i);
		char* t = p;
		//将块中的每一个元组记录设置为已删除
		while (*p != '\0' && p < t + _PAGESIZE) {
			//更新索引
			Tuple tuple = readTuple(p, attr);
			for (int j = 0; j < attr.amount; j++) {
				if (attr.has_index[j] == true) {
					std::string attr_name = attr.attr_name[i];
					std::string FilePath = "INDEX_FILE_" + attr_name + "_" + tmp_name;
					std::vector<Data> d = tuple.getData();
					

					//在索引上删除，此处没写
				}
			}
			//删除记录
			p = SetDeleteOnRecord(p);
			count++;
		}
		//将块写回表文件
		int PID = buffer_manager.getPageId(tablename, i);
		buffer_manager.modifyPage(PID);
	}
	return count;
}

int RecordManager::deleteRecord(std::string tablename, std::string to_attr, Where where) {
	std::string tmp_name = tablename;
	tablename = "./database/data/" + tablename;
	Catalog catalog_manager;
	//检测表是否存在
	if (!catalog_manager.isTableExist(tmp_name)) {
		throw TABLE_NOT_EXISTED();
	}
	Attribute attr = catalog_manager.GetTableAttribute(tmp_name);
	int index = -1;
	bool flag = false;
	//获取目标属性对应的编号
	for (int i = 0; i < attr.amount; i++) {
		if (attr.attr_name[i] == attr) {
			index = i;
			if (attr.has_index[i] == true)
				flag = true;
			break;
		}
	}
	//目标属性不存在，抛出异常
	if (index == -1) {
		//目标属性不存在异常
		throw ATTR_NOT_EXIST()
	}
	else if (attr.attr_type[index] != where.data.attr_type) {
		//where条件中的两个数据的类型不匹配异常
		throw WHERE_TYPE_NOT_MATCH();
	}

	//异常处理完成

	int count = 0;
	//如果目标属性上有索引
	if (flag == true && where.relation_character != NOT_EQUAL) {
		std::vector<int> block_ids;
		//通过索引获取满足条件的记录所在的块号
		searchWithIndex(tmp_name, attr, where, block_ids);
		for (int i = 0; i < block_ids.size(); i++) {
			count += queryDeleteInBlock(tmp_name, block_ids[i], attr, index, where);
		}
	}
	else {
		int blockAccount = getBlockNum(tablename);
		//文件大小为0直接返回
		if (blockAccount == 0)
			return 0;
		//遍历所有的块
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
	//检测表是否存在
	if (!catalog_manager.isTableExist(tmp_name)) {
		throw TABLE_NOT_EXISTED();
	}
	//获取文件所占的块的数量
	// int blockAccount = getFileSize(tablename) / _PAGESIZE;
	// 改为
	int blockAccount = getBlockNum(tablename);
	//处理文件大小为0的特殊情况
	if (blockAccount <= 0)
		blockAccount = 1;
	//获取表的属性
	Attribute attr = catalog_manager.GetTableAttribute(tmp_name);
	//构建table类的实例
	Table table(result_table_name, attr);
	std::vector<Tuple> & v = table.getTuple();
	//遍历所有块
	for (int i = 0; i < blockAccount; i++) {
		//获取当前块的句柄
		char* p = buffer_manager.getPage(tablename, i);
		char* t = p;
		//遍历块中所有记录
		while (*p != '\0' && p < t + _PAGESIZE) {
			//读取记录
			Tuple tuple = readTuple(p, attr);
			//如果记录没有被删除，将其添加到table中
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
	//获取目标属性的编号
	for (int i = 0; i < attr.amount; i++) {
		if (attr.attr_name[i] == to_attr) {
			index = i;
			if (attr.has_index[i] == true)
				flag = true;
			break;
		}
	}

	if (index == -1) {
		//目标属性不存在异常
		throw ATTR_NOT_EXIST();
	}
	else if (attr.attr_type[index] != where.data.attr_type) {
		// where条件中的两个数据的类型不匹配异常
		throw WHERE_TYPE_NOT_MATCH();
	}



	//构建table
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
		//获取文件所占块的数量
		int blockAccount = getBlockNum(tablename);
		if (blockAccount == 0)
			blockAccount = 1;
		//遍历所有块
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
		//表不存在异常
		throw TABLE_NOT_EXISTED();
	}
	Attribute attr = catalog_manager.GetTableAttribute(tmp_name);
	int index = -1;
	//获取目标属性的编号
	for (int i = 0; i < attr.amount; i++) {
		if (attr.attr_name[i] == to_attr) {
			index = i;
			break;
		}
	}
	if (index == -1) {
		//目标属性不存在异常
		throw ATTR_NOT_EXIST();

	}

	//获取文件所占的块的数量
	int blockAccount = getBlockNum(tablename);
	//处理文件大小为0的特殊情况
	if (blockAccount == 0)
		blockAccount = 1;
	//获取表的属性
	std::string FilePath = "INDEX_FILE_" + to_attr + "_" + tmp_name;
	//遍历所有块
	for (int i = 0; i < blockAccount; i++) {
		//获取当前块的句柄
		char* p = buffer_manager.getPage(tablename, i);
		char* t = p;
		//遍历块中所有记录
		while (*p != '\0' && p < t + _PAGESIZE) {
			//读取记录
			Tuple tuple = readTuple(p, attr);
			if (tuple.isDeleted() == false) {
				std::vector<Data> v = tuple.getData();
				index_manager.insertIndex(FilePath, v[index], i);
			}
			p = p + getTupleLength(p);
		}
	}
}

//获取文件大小
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

//从内存中读取一个tuple
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

//获取一个tuple的长度
int RecordManager::getTupleLength(char* p) {
	int i;
	char tmp[100];
	for (i = 0; p[i] != ' '; i++)
		tmp[i] = p[i];
	tmp[i] = '\0';
	std::string s=tmp;
	return (int)stoi(s);
}

//判断插入的记录是否冲突
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

//带索引查找
void RecordManager::searchWithIndex(std::string tablename, std::string to_attr, Where where, std::vector<int> & block_ids) {
	
}

//在块中进行条件删除
int RecordManager::queryDeleteInBlock(std::string tablename, int block_id, Attribute attr, int index, Where where) {
	//获取当前块的句柄
	tablename = "./database/data/" + tablename;//新增
	char* p = buffer_manager.getPage(tablename, block_id);
	char* t = p;
	int count = 0;
	//遍历块中所有记录
	while (*p != '\0' && p < t + _PAGESIZE) {
		//读取记录
		Tuple tuple = readTuple(p, attr);
		std::vector<Data> d = tuple.getData();
		//根据属性类型执行操作
		if (attr.attr_type[index] == 0)
		{
			if (QueryJudge(d[index].datai, where.data.datai, where.relation_character))
			{
				//将记录删除
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

//在块中进行条件查询
void RecordManager::querySelectInBlock(std::string tablename, int block_id, Attribute attr, int index, Where where, std::vector<Tuple> & v) {
	//获取块的句柄
	tablename = "./database/data/" + tablename;
	char* p = buffer_manager.getPage(tablename, block_id);
	char* t = p;
	//遍历所有记录
	while (*p != '\0' && p < t + _PAGESIZE) {
		//读取记录
		Tuple tuple = readTuple(p, attr);
		//如果记录已被删除，跳过该记录
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


