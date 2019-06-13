#include "util.h"

// 判断文件是否存在
bool is_file_exist(const std::string &file_name)
{
	std::fstream file;
	file.open(file_name, std::ios::in);
	if(!file)
	{
		return false;
	}
	else
	{
		return true;
	}
}