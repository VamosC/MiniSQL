#include "RecordManager.h"
#include <cassert>

void RecordManager::insertRecord(const std::string &table_name, Tuple& tuple) {
	auto file_path = "./database/data/" + table_name;

	// 表一定存在
	// Interpreter检查过

	Attribute attr = catalog_manager.GetTableAttribute(table_name);
	std::vector<Data> v = tuple.getData();

	if(v.size() != attr.amount)
	{
		throw minisql_exception("Attribute number not match!");
	}

	for (int i = 0; i < v.size(); i++) {
		if (v[i].type != attr.attr_type[i])
		{
			//属性不匹配异常
			throw minisql_exception("Attribute type not match!");
		}
			
	}
	Table table = selectRecord(table_name);
	std::vector<Tuple>& tuples = table.GetTuples();
	if (attr.primary_key >= 0) {
		if (isConflict(tuples, v, attr.primary_key))
			//主键冲突异常
			throw minisql_exception("Primary key conflict!");
	}
	for (int i = 0; i < attr.amount; i++) {
		if (attr.is_unique[i] == true) {
			if (isConflict(tuples, v, i))
				//存在unqiue冲突异常
				throw minisql_exception("Unique attribute conflict!");
		}
	}


	//获取表所占的块的数量
	int blockAccount = getBlockNum(file_path);

	if (blockAccount == 0)
	{
		blockAccount = 1;
	}
	
	char* p = buffer_manager.getPage(file_path, blockAccount - 1);
	int i,j;

	for (i = 0; p[i] != '\0' && i < _PAGESIZE; i++);

	int len = 0;
	//计算插入的tuple的长度
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
		//插入该元组
		DoInsertOnRecord(p, i, len, v);
		//写回表文件
		PID = buffer_manager.getPageId(file_path, blockAccount - 1);
		buffer_manager.modifyPage(PID);
	}
	//如果剩余的空间不够
	else {
		block_offset = blockAccount;
		//新增一个块
		char* p = buffer_manager.getPage(file_path, blockAccount);
		//插入元组
		DoInsertOnRecord(p, 0, len, v);
		//写回表文件
		PID = buffer_manager.getPageId(file_path, blockAccount);
		buffer_manager.modifyPage(PID);
	}

	auto indexs = catalog_manager.GetTableIndex(table_name);
	//更新索引
	for (int i = 0; i < indexs.amount; i++)
	{
		std::vector<Data> tmp_data = tuple.getData();
		index_manager.insert_index(table_name, indexs.name[i], tmp_data[indexs.whose[i]], block_offset);
	}
}

int RecordManager::deleteRecord(const std::string &table_name) {
	auto file_path = "./database/data/" + table_name;
	// API会检测表是否存在

	int blockAccount = getBlockNum(file_path);
	//表文件大小为0时直接返回
	if (blockAccount == 0)
		return 0;
	Attribute attr = catalog_manager.GetTableAttribute(table_name);
	Table table = selectRecord(table_name);
	auto index = catalog_manager.GetTableIndex(table_name);
	int count = 0;
	//遍历所有块
	for (int i = 0; i < blockAccount; i++) {
		//获取当前块的句柄
		char* p = buffer_manager.getPage(file_path, i);
		char* t = p;
		//将块中的每一个元组记录设置为已删除
		while (*p != '\0' && p < t + _PAGESIZE) {
			//更新索引
			Tuple tuple = readTuple(p, attr);
			for (int j = 0; j < index.amount; j++)
			{
				std::vector<Data> tmp_data = tuple.getData();
				// index_manager.delete_index(table_name, index.name[j]);
				index_manager.delete_index(table_name, index.name[j], tmp_data[index.whose[j]]);
			}
			
			//删除记录
			p = SetDeleteOnRecord(p);
			count++;
		}
		//将块写回表文件
		int PID = buffer_manager.getPageId(file_path, i);
		buffer_manager.modifyPage(PID);
	}
	return count;
}

int RecordManager::deleteRecord(const std::string &table_name, SelectCondition scondition) {
	auto file_path = "./database/data/" + table_name;
	// API检测表是否存在
	// 检查目标属性是否存在以及是否匹配
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
		// 检查索引是否存在
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
	//如果目标属性上有索引
	if (flag) 
	{
		for(auto it : indexs)
		{
			auto where = Where{.data = scondition.key[it.first], .relation_character = op_table[scondition.operationtype[it.first]]};
			//通过索引获取满足条件的记录所在的块号
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
		//文件大小为0直接返回
		if (blockAccount == 0)
			return 0;
		//遍历所有的块
		for (int i = 0; i < blockAccount; i++) {
			count += queryDeleteInBlock(table_name, i, attr, attr_positon, scondition);
		}
	}
	return count;
}

Table RecordManager::selectRecord(const std::string &table_name, std::string result_table_name) {
	auto file_path = "./database/data/" + table_name;
	// 无需检测表是否存在

	// 获取文件所占的块的数量
	int blockAccount = getBlockNum(file_path);
	// 处理文件大小为0的特殊情况
	// 不需要处理
	// 获取表的属性
	Attribute attr = catalog_manager.GetTableAttribute(table_name);
	//构建table类的实例
	Table table(result_table_name, attr);
	std::vector<Tuple> &v = table.GetTuples();
	//遍历所有块
	for (int i = 0; i < blockAccount; i++) {
		//获取当前块的句柄
		char* p = buffer_manager.getPage(file_path, i);
		char* t = p;
		//遍历块中所有记录
		while (*p != '\0' && p < t + _PAGESIZE) {
			//读取记录
			Tuple tuple = readTuple(p, attr);
			//如果记录没有被删除，将其添加到table中
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
	// API会检查表是否存在
	// 也会检查属性是否存在
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
		// 检查索引是否存在
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
	//构建table
	Table new_table(result_table_name, attr);
	std::vector<Tuple>& v = new_table.GetTuples();
	if (flag == true) {
		for(auto it : indexs)
		{
			auto where = Where{.data = scondition.key[it.first], .relation_character = op_table[scondition.operationtype[it.first]]};
			//通过索引获取满足条件的记录所在的块号
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
		//获取文件所占块的数量
		int blockAccount = getBlockNum(file_path);
		//遍历所有块
		for (int i = 0; i < blockAccount; i++) {
			querySelectInBlock(table_name, i, attr, attr_positon, scondition, v);
		}
	}
	return new_table;
}

void RecordManager::createIndex(const std::string &table_name, const std::string &index_name, const std::string &attr) {
	auto file_path = "./database/data/" + table_name;
	Attribute attr_t = catalog_manager.GetTableAttribute(table_name);
	// 目标属性编号
	int index = -1;
	// 获取目标属性的编号
	for (int i = 0; i < attr_t.amount; i++) {
		if (attr_t.attr_name[i] == attr) {
			index = i;
			break;
		}
	}
	// 获取文件所占的块的数量
	int blockAccount = getBlockNum(file_path);
	// 文件大小为0的特殊情况 不需要处理
	// 遍历所有块
	int j = 0;
	for (int i = 0; i < blockAccount; i++) {
		//获取当前块的句柄
		char* p = buffer_manager.getPage(file_path, i);
		char* t = p;
		//遍历块中所有记录
		while (*p != '\0' && p < t + _PAGESIZE) {
			//读取记录
			Tuple tuple = readTuple(p, attr_t);
			if (!tuple.isDeleted()) {
				std::vector<Data> v = tuple.getData();
				index_manager.insert_index(table_name, index_name, v[index], i);
			}
			p = p + getTupleLength(p);
		}
	}
}

//获取文件大小
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

//从内存中读取一个tuple
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
		// 不可能出现删除掉的元素
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

//带索引查找
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
		// 不可能出现这种情况
		// 若出现一定是编程出错了
		assert(false);
	}
}

//在块中进行条件删除
int RecordManager::queryDeleteInBlock(const std::string &table_name, int block_id, const Attribute &attr, std::vector<int> &indexs, const SelectCondition &cond) {
	//获取当前块的句柄
	auto file_path = "./database/data/" + table_name;
	char* p = buffer_manager.getPage(file_path, block_id);
	char* t = p;
	int count = 0;
	//遍历块中所有记录
	while (*p != '\0' && p < t + _PAGESIZE) {
		//读取记录
		Tuple tuple = readTuple(p, attr);
		if(tuple.isDeleted())
		{
			p += getTupleLength(p);
			continue;
		}
		std::vector<Data> d = tuple.getData();
		//根据属性类型执行操作
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

//在块中进行条件查询
void RecordManager::querySelectInBlock(const std::string &table_name, int block_id, const Attribute &attr, std::vector<int> &indexs, const SelectCondition &cond, std::vector<Tuple> & v) {
	//获取块的句柄
	auto file_path = "./database/data/" + table_name;
	char* p = buffer_manager.getPage(file_path, block_id);
	char* t = p;
	//遍历所有记录
	while (*p != '\0' && p < t + _PAGESIZE) {
		//读取记录
		Tuple tuple = readTuple(p, attr);
		//如果记录已被删除，跳过该记录
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

