#include "BufferManager.h"

// Page类的实现
Page::Page() 
{
    initialize();
}
// 初始化
void Page::initialize() 
{
    FileName = "";
    BlockID = -1;
    PinCount = -1;
    IsDirty = false;
    IsRef = false;
    IsAvaliable = true;
    for (int i = 0;i < _PAGESIZE;i++)
    {
        buffer[i] = '\0';
    }      
}

inline void Page::setFileName(std::string FileName) 
{
    FileName = FileName;
}

inline std::string Page::getFileName() 
{
    return FileName;
}

inline void Page::setBlockId(int BlockID) 
{
    BlockID = BlockID;
}

inline int Page::getBlockId() 
{
    return BlockID;
}

inline void Page::setPinCount(int PIN_Count) 
{
    PinCount = PIN_Count;
}

inline int Page::getPinCount() 
{
    return PinCount;
}

inline void Page::setDirty(bool Is_Dirty) 
{
    IsDirty = Is_Dirty;
}

inline bool Page::getDirty() 
{
    return IsDirty;
}

inline void Page::setRef(bool ref) 
{
    IsRef = ref;
}

inline bool Page::getRef() 
{
    return IsRef;
}

inline void Page::setAvaliable(bool Is_Avaliable) 
{
    IsAvaliable = Is_Avaliable;
}

inline bool Page::getAvaliable() 
{
    return IsAvaliable;
}

inline char* Page::getBuffer() 
{
    return buffer;
}

// BufferManager类的实现

//构造函数，默认大小
BufferManager::BufferManager() 
{
    initialize(_MAXFRAMESIZE);
}

//构造函数，制定大小
BufferManager::BufferManager(int FrameSize) 
{
    initialize(FrameSize);
}

// 实际初始化
void BufferManager::initialize(int FrameSize) 
{
    Frames = new Page[FrameSize];
    frame_size_ = FrameSize;
    current_position_ = 0;
}

// 析构函数。
// 在程序结束时需要将buffer里的所有页写回磁盘。
BufferManager::~BufferManager() 
{
    for (int i = 0;i < frame_size_;i++) 
    {
        int BlockID;
        std::string FileName;
        FileName = Frames[i].getFileName();
        BlockID = Frames[i].getBlockId();
        if(flushPage(i , FileName , BlockID)==0)
        {
            //报错机制，此处省略
        }
    }
    delete[] Frames;
}

char* BufferManager::getPage(std::string FileName , int BlockID) 
{
    int PageID = getPageId(FileName , BlockID);
    if (PageID == -1) {
        PageID = getEmptyPageId();
        loadDiskBlock(PageID , FileName , BlockID);
    }
    Frames[PageID].setRef(true);
    return Frames[PageID].getBuffer();
}

void BufferManager::modifyPage(int PageID) 
{
    Frames[PageID].setDirty(true);
}

void BufferManager::pinPage(int PageID) 
{
    int PIN_Count = Frames[PageID].getPinCount();
    Frames[PageID].setPinCount(PIN_Count + 1);
}

int BufferManager::unpinPage(int PageID) 
{
    int PIN_Count = Frames[PageID].getPinCount();
    if (PIN_Count <= 0)
        return -1;
    else
        Frames[PageID].setPinCount(PIN_Count - 1);
    return 0;
}

// 遍历获取页号
int BufferManager::getPageId(std::string FileName , int BlockID) 
{
    for (int i = 0;i < frame_size_;i++) {
        std::string tmp_file_name = Frames[i].getFileName();
        int tmp_block_id = Frames[i].getBlockId();
        if (tmp_file_name == FileName && tmp_block_id == BlockID)
            return i;
    }
    return -1;
}

// 写入一页
int BufferManager::flushPage(int PageID , std::string FileName , int BlockID) 
{
    FILE* f = fopen(FileName.c_str() , "r+");
    // 将文件指针定位到对应位置
    fseek(f , _PAGESIZE * BlockID , SEEK_SET);
    // 获取页的句柄
    char* buffer = Frames[PageID].getBuffer();
    // 将内存页的内容写入磁盘块
    fwrite(buffer , _PAGESIZE , 1 , f);
    // 关闭文件
    fclose(f);
    return 0;
}

// 寻找一个闲置页
int BufferManager::getEmptyPageId() {
    // 遍历一遍
    for (int i = 0;i < frame_size_;i++) {
        if (Frames[i].getAvaliable() == true)
            return i;
    }

    // 如果所有页都已经被使用，那就采用时钟替换策略找到一个页并删除。
    while (1) {
        if (Frames[current_position_].getRef() == true) {
            Frames[current_position_].setRef(false);
        }
        else if (Frames[current_position_].getPinCount() == 0) {
            if (Frames[current_position_].getDirty() == true) {
                std::string FileName = Frames[current_position_].getFileName();
                int BlockID = Frames[current_position_].getBlockId();
                flushPage(current_position_ , FileName , BlockID);
            }
            Frames[current_position_].initialize();
            return current_position_;
        }
        current_position_ = (current_position_ + 1) % frame_size_;
    }
}

//内存和磁盘交互的接口。
int BufferManager::loadDiskBlock(int PageID , std::string FileName , int BlockID) {
    // 初始化一个页
    Frames[PageID].initialize();
    // 打开磁盘文件
    FILE* f = fopen(FileName.c_str() , "r");
    // 打开失败返回-1
    if (f == NULL)
        return -1;
    // 将文件指针定位到对应位置
    fseek(f , _PAGESIZE * BlockID , SEEK_SET);
    // 获取页的句柄
    char* buffer = Frames[PageID].getBuffer();
    // 读取对应磁盘块到内存页
    fread(buffer , _PAGESIZE , 1 , f);
    // 关闭文件
    fclose(f);
    // 对新载入的页进行相应设置
    Frames[PageID].setFileName(FileName);
    Frames[PageID].setBlockId(BlockID);
    Frames[PageID].setPinCount(1);
    Frames[PageID].setDirty(false);
    Frames[PageID].setRef(true);
    Frames[PageID].setAvaliable(false);
    return 0;
}