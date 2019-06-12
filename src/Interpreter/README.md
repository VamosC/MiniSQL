#相关语法示例
##创建表语句

	create table student (
			sno char(8),
			sname char(16) unique,
			sage int,
			sgender char (1),
			primary key (sno)
	);

##删除表语句

  drop table student;

##创建索引语句

	create index stunameidx on student (sname);

##删除索引语句

	drop index stunameidx;

##选择语句

  select * from student;
  select * from student where sno = ‘88888888’;
  select * from student where sage > 20 and sgender = ‘F’;
    
##插入记录语句
  <b>括号内的值，和‘之间一定要用空格隔开<\b>
	insert into student values (‘12345678’, ’wy’, 22, ’M’);

##删除记录语句

	delete from student;
  delete from student where sno = ‘88888888’;
