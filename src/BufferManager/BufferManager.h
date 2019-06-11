#ifndef _BUFFER_MANAGER_H_
#define _BUFFER_MANAGER_H_ 

#include <cstdio>
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include "../ERROR.h"
#include "../base.h"
#include "../util.h"

/*
    创建一个BufferManager的对象
    
    getPage:    获取file对应的block在内存中的handle

    getPageId:  获取块在内存中的页号。
    
    modifyPage: 标记页已被修改，防止修改丢失。

    pinPage:    钉住某页，可以重复对该页进行操作

    unpinPage:  pinPage的反操作。

    flushPage:  将内存中的一页写入文件中的一块

*/
//  Page类,磁盘的每个一块对应内存中的一个page
class Page {
    public:
        // 构造函数
        Page();

        //初始化
        void initialize();

        //命名
        void setFileName(std::string file_name);

        //获取文件名
        std::string getFileName();

        //设置块ID
        void setBlockId(int block_id);

        //获得块ID
        int getBlockId();

        //
        void setPinCount(int pin_count);

        //
        int getPinCount();

        //设置脏页面
        void setDirty(bool dirty);

        //
        bool getDirty();

        //
        void setRef(bool ref);

        //
        bool getRef();

        //
        void setAvaliable(bool avaliable);

        //
        bool getAvaliable();

        //
        char* getBuffer();

    private:
        //一个page是大小为 PAGESIZE 4096 的数组
        char buffer[_PAGESIZE];

        //页所对应的文件名
        std::string FileName;

        //页在所在文件中的块号(磁盘中通常叫块)
        int BlockID;

        //记录被钉住的次数
        int PinCount;

        //记录脏页
        bool IsDirty;

        //用于时钟替换策略
        bool IsRef;

        //avaliable标示页是否被load到磁盘块
        bool IsAvaliable;
};

// BufferManager类，对外提供操作缓冲区的接口。
class BufferManager {
    public: 
        //构造函数
        BufferManager();

        //
        BufferManager(int frame_size);

        // 析构函数
        ~BufferManager();


        // 通过页号得到页的handle，即页的首地址
        char* getPage(std::string file_name , int block_id);


        // 标记page_id所对应的页已经被修改
        void modifyPage(int page_id);


        // 钉住一个页
        void pinPage(int page_id);


        // 解除一个页的一次钉住状态
        int unpinPage(int page_id);


        // 将对应内存页写入对应文件的对应块
        int flushPage(int page_id , std::string file_name , int block_id);
       
        // 获取文件的对应块在内存中的页号
        int getPageId(std::string file_name , int block_id);

    private:
        Page* Frames;//缓冲池，实际上就是一个元素为Page的数组，实际内存空间将分配在堆上
        
        int frame_size_;//记录总页数
        
        int current_position_;//时钟替换策略需要用到的变量
       
        void initialize(int frame_size);//实际初始化函数
        // 获取一个闲置的页的页号(内部封装了时钟替换策略，但使用者不需要知道这些)
        
        int getEmptyPageId();
        // 讲对应文件的对应块载入对应内存页，对于文件不存在返回-1，否则返回0
        
        int loadDiskBlock(int page_id , const std::string &file_name , int block_id);
};

#endif