#include "RecordManager.h"

int main(int argc, char const* argv[])
{
	BufferManager bm;
	IndexManager im(bm);
	CatalogManager cm(bm);
	RecordManager record(bm, cm, im);
	std::vector<Data> v;
	// auto data = Data{.type = 10, .sdata = std::string("456")};
	// v.push_back(data);
	auto data = Data{.type = -1, .idata = 3};
	// v.push_back(data);
	// data = Data{.type = 20, .sdata = std::string("xiaoming")};
	// v.push_back(data);
	// Tuple tuple(v);
	// auto cond = Where{.data = data, .relation_character = EQUAL};
	// std::cout << record.deleteRecord("student", "age", cond) << '\n';
	// record.insertRecord("student", tuple);
	// std::cout << "??" << '\n';
	auto table = record.selectRecord("student");
	table.PrintTable();
	return 0;
}