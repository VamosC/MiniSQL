# 相关源代码

```c++
// IndexManager的使用需要BufferManager
// 每一次启动程序 IndexManager应当只有一份
// 如何使用:
// 假定已有 BufferManager bm;

// 构造
IndexManager index_manager(bm);

// 创建索引 例如student表的name索引 类型是 -1
// 类型的具体定义见base.h
// -1 - int; 0 - float; 正数 - string（varchar长度）
// 注意索引只能创建一次, 如果创建时已经存在了索引文件
// 那么该函数会抛出错误, 这是编程的错误, 因此需要解决
// 也就是说创建索引的时候要从维护的信息中判断是否已经创建
index_manager.create("student", "name", -1); 

// 假定已经有Data data 包含int类型的索引 2 并且索引的记录在第100个block中 block_id为99
// 插入索引
// 注意我们实现的是单值索引 因此插入已有的key是需要提醒用户的
// 当插入相同索引的时候 会抛出错误
// 在这里可以不用去捕获, 抛到最上层的时候捕获输出
index_manager.insert_index("student", "name", data, 99);

// 删除索引项
index_manager.delete_index("student", "name", data);

// 查找索引
// 首先要有一个容纳结果的vector
// 注意是单值查找(=)
// 可以通过返回值判断是否找到
std::vector<int> block_id;
if(index_manager.find_key("student", "name", data, block_id))
{
  // 找到
  // 结果在
  block_id[0] // 对应记录在student表的哪个block上 
}
else
{
  // 没找到
}

// 范围查找
// 同样通过vector来存储结果
// 注意需要区分 < <=
// 范围查找需要把 > >= 转换成 < <=
// 注意多个条件的结合比如:
// age > 18 and age < 22 转化为 18 < age < 22
std::vector<int> block_ids;
condition cond;
cond.l_op = 2; // <
cond.start = ..... // 18 
cond.r_op = 2; // <
cond.end = .... // 22
//  ||
//  \/
// cond 也可以这么定义
// auto 是自动类型推导
// condition是对应的结构体
auto cond = condition{.l_op = 2, .start = ... , .r_op = 2, .end = ...};

if(index_manager.find_range_key("student", "name", cond, block_ids))
{
  // 找到
  // 结果在
  block_ids // 对应记录在student表的哪个block上 
}
else
{
  // 没找到
}


// 删除索引
index_manager.drop_index("student", "name", -1);


```

