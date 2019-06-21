



 



![1561124102892](C:\Users\apple\AppData\Roaming\Typora\typora-user-images\1561124102892.png)

 









**<center>数据库系统实验报告</center>**

 

| 作业名称： | MiniSQL 程序设计 |
| :--------: | :--------------: |
| 姓    名： |      何墨琪      |
| 学    号： |    3170101799    |
| 其他组员： |    梁超、冯伟    |
| 电子邮箱： | 939210190@qq.com |
| 联系电话： |   15356252701    |
| 指导老师： |      孙建伶      |

 

 

 

 

 







<center>2019年	6月	19日</center>

​	















# MiniSQL程序设计

## 一、 实验目的

通过对于数据库Interpreter、API、Catalog Manager、Record Manager、Index Manager和Buffer Manager的了解和学习，结合课上所学相关数据库知识，设计并实现一个精简型单用户SQL引擎(DBMS)MiniSQL，允许用户通过字符界面输入SQL语句实现表的建立/删除；索引的建立/删除以及表记录的插入/删除/查找。

 

## 二、 系统需求

### 1.1 需求概述

| 要求类型         | 要求的细节                                                   |
| ---------------- | ------------------------------------------------------------ |
| 数据类型         | 只要求支持三种基本数据类型：int，char(n)，float，其中char(n)满足 1 <= n <= 255 。 |
| 表定义           | 一个表最多可以定义32个属性，各属性可以指定是否为unique；支持单属性的主键定义。 |
| 索引的建立和删除 | 对于表的主属性自动建立B+树索引，对于声明为unique的属性可以通过SQL语句由用户指定建立/删除B+树索引（因此，所有的B+树索引都是单属性单值的）。 |
| 查找记录         | 可以通过指定用and连接的多个条件进行查询，支持等值查询和区间查询。 |
| 插入和删除记录   | 支持每次一条记录的插入操作；支持每次一条或多条记录的删除操作。 |



### 1.2 语法说明

MiniSQL支持标准的SQL语句格式，每一条SQL语句以分号结尾，一条SQL语句可写在一行或多行（多行针对create table这一语句）。为简化编程，要求所有的关键字都为小写。

我们组基本参考了MySQL的SQL语句格式，实现了以下语句：

#### 创建表语句

```sql
create table 表名 (
	列名 类型 ,
	列名 类型 ,
	primary key ( 列名 )
);
```

若该语句执行成功，则输出执行成功信息；若失败，必须告诉用户失败的原因。

常见的语法错误原因有单词拼写错误，'('及')'位置摆放不对，primary key 未选中已定义的列名等。

#### 删除表语句

```sql
drop table 表名 ;
```

若该语句执行成功，则输出执行成功信息；若失败，必须告诉用户失败的原因。

常见的语法错误原因有单词拼写错误等。

#### 创建索引语句

```sql
create index 索引名 on 表名 ( 列名 );
```

若该语句执行成功，则输出执行成功信息；若失败，必须告诉用户失败的原因。

常见的语法错误原因有单词拼写错误，'('及')'位置摆放不对，缺失关键词index\on等。

#### 删除索引语句

```sql
drop index 索引名 on 表名;
```

若该语句执行成功，则输出执行成功信息；若失败，必须告诉用户失败的原因。

常见的语法错误原因有单词拼写错误，'('及')'位置摆放不对，缺失关键词index\on等。

#### 选择语句

```sql
select * from 表名 ;

select * from 表名 where 条件 and 条件;
```

其中“条件”具有以下格式：列 op 值 and 列 op 值 … and 列 op 值。

op是算术比较符：=	<>	<	>	<=	>=

若该语句执行成功且查询结果不为空，则按行输出查询结果，第一行为属性名，其余每一行表示一条记录；若查询结果为空，则输出信息告诉用户查询结果为空；若失败，必须告诉用户失败的原因。

常见的语法错误原因有单词拼写错误，缺失关键词from\where\and，算数比较符输入错误等。

#### 插入记录语句

```sql
insert into 表名 values ( 值1 , 值2 , … , 值n );
```

若该语句执行成功，则输出执行成功信息；若失败，必须告诉用户失败的原因。

常见的语法错误原因有单词拼写错误，缺失关键词into\values，'('及')'位置摆放不对等。

#### 删除记录语句

该语句的语法如下：

```sql
delete from 表名 ;

delete from 表名 where 条件 and 条件;
```

若该语句执行成功，则输出执行成功信息，其中包括删除的记录数；若失败，必须告诉用户失败的原因。

常见的语法错误原因有单词拼写错误，缺失关键词from\where\and，算数比较符输入错误等。

#### 退出MiniSQL系统语句

```
quit;
```

#### 执行SQL脚本文件语句

```
execfile 文件名 ;
```

SQL脚本文件中可以包含任意多条上述8种SQL语句，MiniSQL系统读入该文件，然后按序依次逐条执行脚本中的SQL语句。

 





## 三、 实验环境

VS2017，使用C++编程语言

 





## 四、 模块设计

由于本次实验室小组合作的形式，所以**我在小组中主要负责了Interpreter、API的撰写预测试，以及Catalog Manager初稿的撰写**。

**整体类图及类间关系如下：**

![未命名文件 (5)](C:\Users\apple\Downloads\未命名文件 (6).jpg)

### 4.1 总体数据结构（不含相应的类内函数）

#### 基本数据

```c++
class Data{
public:
	int type;				//-1 - int; 0 - float; 正数 - string（varchar长度）
	int idata；
	float fdata;
	std::string sdata;
}
```

#### 条件判断

```c++
//operation here: 0- = 1- <> 2- < 3- > 4- <= 5- >=

struct SelectCondition{
	int amount;
	std::string attr[30];
	int operationtype[30];
	Data key[30];
};
```

 

#### 属性存储

```C++
//数量、属性名、类型、是否唯一、是否存在索引、主码
//最多存放32个属性 

struct Attribute{
	int amount;
	std::string attr_name[32];
	int attr_type[32];		//1 - int; 0 - float; 2 - string
	bool is_unique[32];
	int primary_key;		//主码的序号 
};
```

 

#### 索引存储

```C++
//一张表最多有10个索引 

struct Index{
	int amount;
	std::string name[10]; 
	int whose[10];			//索引及对应的属性 
}; 
```

 

#### 表的存储

```C++
class Table{
private:
	std::vector<Tuple> tuples;
	std::string table_name;
	struct Attribute attr;
}
```



### 4.2 Interpreter

1. **主要数据结构**

   Interpreter类主要用来一个类来储存interpreter会使用到的API的引用，Catalog Manager的引用。

2. **类图**

   ![1561000169867](C:\Users\apple\AppData\Roaming\Typora\typora-user-images\1561000169867.png)

3. **如何检测SQL是否有错误**

   在本程序中，我采用了一行一行读取输入的方式一个一个单词拆解，预计可能出现的错误，比如 create 后面只能出现 table 和 index， 如果出现了其他单词，就说明出现了语法错误。

   所以在每执行一项任务时，先通过前一个单词/两个单词去判断执行什么语句，之后对不同与句之间分类讨论。对于每种语句的检测，主要考虑需要读入的信息极可能存在的错误，在可能存在错误或可能结尾的地方设置if-else语句去判断是否满足语法要求，如果不满足，则输出相应的语法错误；如果满足，就继续读入。当所有的单词都分析过一遍，发现不存在错误，就调用API的接口来执行语句。除非有特殊需要，检测表、属性是否存在都交给底层来执行。

 

### 4.3 API

1. **API接口设计, 输入参数说明，返回参数说明**

   由于API是沟通整个程序所有部分的桥梁，所以API中储存了Catalog Manager、Record Manager、Index Manager的引用，方便统一协调所有的执行函数。

   API接受的内容都是Interpreter处理后传输下来的数据，因此输入的参数一般为string及Interpreter处理过后的Attribute，Data数组， Selectcondition，供下层模块调用的时候方便地使用。

   由于7条语句有不同的执行方式，所以API设计了7种供Interpreter调用的接口，一旦执行成功均返回空值（select返回table类），如果中如出现错误就及时抛出错误，停止执行。另外还有一条Combine语句用于多值select时多不同的查询结果进行合并，供API中的Select函数调用。

   

2. **功能描述**

   API模块是**整个系统的核心**，其主要功能为提供执行SQL语句的接口，供Interpreter层调用。API连接了所有的模块，提供应用程序与开发人员基于某软件或硬件得以访问一组例程的能力，而又无需访问源码，或理解内部工作机制的细节。

   API将接受有Interpreter处理后的指令内容，调用Catalog Manager、Record Manager、Index Manager中相应的功能实现函数，传递相应的参数，让指令能够得到执行。

   

3. **主要数据结构**

   API类中主要是用到了Catalog Manager、Record Manager、Index Manager的引用。在Combine中还会使用相应的数组（vector）辅助。

4. 给出类图

 ![未命名文件 (5)](C:\Users\apple\Downloads\未命名文件 (5).jpg)

### 4.4 Catalog Manager

1. 功能描述

   Catalog Manager是一个文件管理系统，它储存了所有表的基本信息，包括表名，表中属性个数，表内的各个属性及属性名，属性是否唯一，主码等等。

   另外它还需要储存关于箱管表的索引信息，包括索引个数，索引名称，索引对应的属性等等。

   在本程序中，表的信息将以以下方式存储：

   > @ tablename attribute_number attrbute_name type is_unique(按顺序)  primary_key_number index_number index_name towhatattribute#

   

2. 主要数据结构

   Catalog Manager 类中主要储存了表信息的出巡路径 TABLE_PATH，以及Buffer Manager的引用，用来调取Buffer Manager中的函数存储表的相关信息。

3. 给出类图

![1561000370826](C:\Users\apple\AppData\Roaming\Typora\typora-user-images\1561000370826.png)

## 五、 模块实现

### Interpreter

1. 阐述该模块所使用的数据结构，说明实现各个功能的核心代码(给出简化后的代码截图)，包括重要的函数等。

   ##### 数据结构

   ​	Interpreter类主要用来一个类来储存interpreter会使用到的API的引用，Catalog Manager的引用。

   ##### 功能核心代码

   **JudgeAndExec();**

   ![i1](C:\Users\apple\Desktop\数据库\报告所需图片\i1.JPG) 

   

   ![i2](C:\Users\apple\Desktop\数据库\报告所需图片\i2.JPG) 

   

   **ExecCreateTable();**

   ![i3](C:\Users\apple\Desktop\数据库\报告所需图片\i3.JPG) 

   

   ![i4](C:\Users\apple\Desktop\数据库\报告所需图片\i4.JPG) 

   

   **ExecDropTable();**

   ![i60](C:\Users\apple\Desktop\数据库\报告所需图片\i60.JPG) 

   

   **ExecCreateIndex();**

   ![ i70](C:\Users\apple\Desktop\数据库\报告所需图片\i70.JPG)  

   

   **ExecDropIndex();**

   ![i80](C:\Users\apple\Desktop\数据库\报告所需图片\i80.JPG)  

   

   **ExecSelect();**

   ![i9](C:\Users\apple\Desktop\数据库\报告所需图片\i9.JPG) 

   

   ![i10](C:\Users\apple\Desktop\数据库\报告所需图片\i10.JPG) 

   

   ![i11](C:\Users\apple\Desktop\数据库\报告所需图片\i11.JPG) 

   

   **ExecInsert();**

   ![i12-1](C:\Users\apple\Desktop\数据库\报告所需图片\i12-1.JPG) 

   

   ![i12-2](C:\Users\apple\Desktop\数据库\报告所需图片\i12-2.JPG) 

   

   **ExecDelete();**

   ![i13-1](C:\Users\apple\Desktop\数据库\报告所需图片\i13-1.JPG) 

   

   ![i13-2](C:\Users\apple\Desktop\数据库\报告所需图片\i13-2.JPG) 

    

   **ExecFile();**

   ![i14](C:\Users\apple\Desktop\数据库\报告所需图片\i14.JPG) 

   

   ![i50](C:\Users\apple\Desktop\数据库\报告所需图片\i50.JPG) 

   

2. 阐述模块的测试代码

   本部分测试代码主要是对原本的代码作了修改，把执行API和调用Catalog Manager的地方注释掉，再调用main函数测试。

   测试部分结果如下：

   由于图片过多，本部分只展示了create table 的部分测试结果，其他的会放在同报告一起的文件夹中。

   ![1561120883174](C:\Users\apple\AppData\Roaming\Typora\typora-user-images\1561120883174.png)



### API

1. 阐述该模块所使用的数据结构，说明实现各个功能的核心代码(给出简化后的代码截图)，包括重要的函数。

   **CreateTable**

   ![a1](C:\Users\apple\Desktop\数据库\报告所需图片\a1.JPG)

   

   **DropTable**

   ![a2](C:\Users\apple\Desktop\数据库\报告所需图片\a2.JPG) 

   

   **CreateIndex**

   ![a3](C:\Users\apple\Desktop\数据库\报告所需图片\a3.JPG) 

   

   **DropIndex**

   ![a4](C:\Users\apple\Desktop\数据库\报告所需图片\a4.JPG) 

   

   **Insert**

   ![a5](C:\Users\apple\Desktop\数据库\报告所需图片\a5.JPG)  

   

   **Delete**

   ![a6](C:\Users\apple\Desktop\数据库\报告所需图片\a6.JPG) 

   

   **Select**

   ![a7](C:\Users\apple\Desktop\数据库\报告所需图片\a7.JPG) 

    

2. 阐述模块的测试代码

   API的测试主要是测试某些功能是否能实现，测试伪代码如下（其中api中调用Catalog Manager和Record Manager的地方注释掉，用输出某一特定字符串代替）：

   ```pseudocode
   void APItest()
   {
       Create test attribute data;
       Create test tuple datas;//at least 10; specify a special tuple
       
       Create Table table1;
       Insert all the tuples into table1;
       Delete some tuples from table1 using condition;
       Select the special tuple from table1;
       Create Index i1 on table1;
       Select the special tuple from table1;
       Drop Index i1;
       Drop Table table1;
   }
   ```

   



### Catalog Manager

1. 阐述该模块所使用的数据结构，说明实现各个功能的核心代码(给出简化后的代码截图)，包括重要的函数、提供给API模块的接口(如果有)等。

   **接口：**

   ```C++
   // 创建表格
   // 输入：表格名称、表格属性、索引对象、主码 
   void CreateTable(const std::string &table_name, Attribute attr, Index indices, int primary_key);
   
   // 删除表格
   // 输入：表格名称 
   void DropTable(const std::string &table_name);
   
   // 通过表名查看表是否存在	
   // 输入：表格名称	
   bool isTableExist(const std::string &table_name);
   
   // 某一属性是否存在
   // 输入：表格名称、属性名称
   // 输出：非负数-位置； -1-不存在		 
   int isAttributeExist(const std::string &table_name, const std::string &attr);
   
   // 得到某表的全部属性,必须在表存在时才可以用 
   // 输入：表格名称
   // 输出：Attribute结构数据
   Attribute GetTableAttribute(const std::string &table_name);
   
   // 在指定属性上建立索引
   // 输入：表格名称、属性名称、索引名称
   void CreateIndex(const std::string &table_name, const std::string &attr, const std::string &index_name);
   
   // 删除索引
   // 输入：表格名称、索引名称
   // 输出：1-成功； 0-失败,包含异常
   void DropIndex(const std::string &table_name, const std::string &index_name);
   
   // 索引是否存在
   // 输入：表格名称、索引名称
   // 输出：正整数 - index的序号，0-不存在		 
   int isIndexExist(const std::string &table_name, const std::string &index_name);
   
   // 得到某表的全部索引,必须在表存在时才可以用 
   // 输入：表格名称
   // 输出：Index结构数据
   Index GetTableIndex(const std::string &table_name);
   ```

   **create table**

   ![c1](C:\Users\apple\Desktop\数据库\报告所需图片\c1.JPG) 

   

   ![c2](C:\Users\apple\Desktop\数据库\报告所需图片\c2.JPG) 

   

   **drop table**

   ![c3](C:\Users\apple\Desktop\数据库\报告所需图片\c3.JPG)  

   

   **create index**

   ![c4](C:\Users\apple\Desktop\数据库\报告所需图片\c4.JPG)  

   

   **drop index**

   ![c5](C:\Users\apple\Desktop\数据库\报告所需图片\c5.JPG)  

    

   

2. 阐述模块的测试代码

   Catalog Manager的测试主要是测试对于表的信息的存储功能是否能实现，测试伪代码如下：

   ```pseudocode
   void Catalogtest()
   {
       Create test attribute data;
       
       Create Table table1;
       Create Table table2;
       Create Table table3;
       //to see if the file is what we expected
       Create Index i1 on table1;
       Create Index i2 on table2;
       Create Index i3 on table1;
       //to see if the file is what we expected
       Drop Index i1;
       Drop Index i2;
       Drop Index i3;
       //to see if the file is what we expected
       Drop Table table1;
       Drop Table table2;
       Drop Table table3;
   }
   ```

   

## 六、 遇到的问题及解决方法

自刚刚开始的时候，分到Interpreter、API、Catalog Manager的撰写，挺令人慌张的，因为当时并不清楚它们具体的功能以及最终的详细实现形式。之后慢慢地查找资料，去了解它们的功能和具体的形式，也就慢慢地将代码写了出来。

在写代码的过程中，因为我分到的部分属于较顶层的部分，需要调用很多下面那些模块的代码，但是由于组员的进度不同，有些需要调用的函数不能及时拿到，就经常进度停滞。最终经过商讨，由我将需要调用底层函数的部分通过注释说明需要做些什么，最终由写底层函数的同学们来进行补充，所以Catalog Manager这一部分更多的是大家一起合作的结果，由我先写一个大框架，再由其他两位同学进行补充和细节修正。

另外为了让我们的交互部分更贴近MySQL，或者更好地贴近用户的习惯，我们对Interpreter做了许多修改和补充。由于我采用了每次读一行，一个一个单词分析的方式，就比大多数人选择的一下将完整的语句读进一个数组再进行字符分析要困难的多，需要更多的细节讨论和对于空格的细节处理。对于每一点点错误，我逐个慢慢分析修改，而没有想要一下子就想出一种方式解决这些问题，就有效率的多，最终完成的Interpreter部分就比较贴近平时的SQL习惯，尽管还是存在一些硬性要求。

API主要是调用Record Manager和Catalog Manager中的函数进行数据传输，其实也不是很复杂，没有遇上什么大问题。



## 七、 总结

MiniSQL通过Interpreter、API、Catalog Manager、Record Manager、Index Manager、Buffer Manager的协调合作，最终实现了有条理的，有一定效率的数据管理系统。

我在本次实现中主要实现了Interpreter、API、Catalog Manager三个部分，通过Interpreter进行语法错误检测，并提炼出有效的命令信息，API主要通过调用Catalog Manager、Record Manager的接口来进行统筹，Catalog Manager则将表的各种信息组织成字符串，实现表的基本信息的管理。

本次MiniSQL的小组合作让我深刻的理解了数据库系统的基础实现形式，尽管目前实现的并不是什么非常先进或者贴近现代的技术，但是仍然为我们之后的学习打下了基础。在小组的合作中，我们还更深刻地理解了小组分工合作以及成员间交流的重要性，通过小组协作，完成一个较困难的程序实现，更有益于自身实力的提升和自身视野的拓展。