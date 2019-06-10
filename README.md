# MiniSQL

DataBase 期末大程 MiniSQL

## 支持语法

- 创建表

  ```sql
  create table student (
    sid char(8) unique ,                     //char(n)       1 <= n <= 255
    sname char(20) ,
    sage int ,
    scome float ,
    primary key (sid)
  );
  // 最多32个属性
  ```

  

- 删除表

  ```sql
  drop table student ;
  ```

  

- 创建索引

  ```sql
  create index stuname on student(sname) ;
  ```

  

- 删除索引

  ```sql
  drop index stuname ;
  ```

  

- 选择语句

  ```sql
  select * from	student ;
  select * from student where sid = '88888888' ;
  select * from student where sage > 20 and scome < 2000 ;
  
  // < > <= >= <> = 
  ```

  

- 插入语句

  ```sql
  insert into student values ( '88888888', 'xiaoming', 20, 2000.00 ) ;
  ```

  

- 删除语句

  ```sql
  delete from student ;
  delete from student where sid = '88888888' ;
  ```

  

- 退出语句

  ```sql
  quit;
  ```

  

- 执行脚本

  ```sql
  execfile "test.txt" ;
  ```

  

## 要求

- 需要返回查询时间、执行返回或影响的行数、错误信息

## 文件格式

- 定长记录 or 不定长记录？
- 块内存放信息？
- ………………...

## 备忘/提醒
