//
//  YDMmapLogger.h
//
//  Created by wangyuandong on 2021/9/22.
//  Cross-platform mmap logger core — shared by iOS and Android.
//

#ifndef YDMmapLogger_hpp
#define YDMmapLogger_hpp

#include <stdio.h>
#ifdef __APPLE__
#include <libkern/OSAtomic.h>
#endif
#include <unistd.h>
#include <string>
#include <atomic>

typedef enum : int {
    /****** 文件映射/关闭映射相关错误 ******/
    YD_NOPATH      =     1001,      // 文件路径为nil
    YD_NOSPACE     =     1002,      // 磁盘空间不足
    YD_EXCESSIVE   =     1003,      // 文件过大
    YD_NOFILE      =     1004,      // 只读模式，且待读取的文件大小为0
    YD_UNOPENED    =     1005,      // 没有被打开的文件
    YD_INVALIDFD   =     1006,      // 无效的文件描述符
    
    /****** 日志写入相关错误 ******/
    YD_NOSTART     =     2001,      // 写入文件的起始指针为NULL
    YD_NORAM       =     2002,      // 文件没有足够空间保存即将写入的数据
} YDERROR;

#if __GNUC__
#define YD_LIKELY(x) __builtin_expect(!!(x),1)
#define YD_UNLIKELY(x) __builtin_expect(!!(x),0)
#else
#define YD_LIKELY(x) (!!(x))
#define YD_UNLIKELY(x) (!!(x))
#endif

/**
 union yd_flags 的位域
 
 opened 默认为0 文件是否打开
 read_write 默认为1 文件打开模式是否为可读写
 block_size 默认为1024 自定义的文件运输块的大小，方便不同的系统计算磁盘剩余空间
 fsize_min 默认为256*1024 文件大小的最小值，但不是绝对最小值，因为mmap映射的虚拟内存大小是内存页大小的整倍数
 fsize_max 默认为256*1024 文件大小的最大值，但不是绝对最大值，因为mmap映射的虚拟内存大小是内存页大小的整倍数
 reserved_size 默认为32*1024*1024 剩余的磁盘空间需不小于reserved_size，才可以写入数据
 */
#define YDFLAGS_BITFIELD                       \
uint32_t opened         : 1;               \
uint32_t read_write     : 1;               \
uint32_t block_size     : 11;              \
uint32_t fsize_min      : 11;              \
uint32_t fsize_max      : 1;               \
uint32_t reserved_size  : 7


#define YD_FLAGS_VALUE      0x03040402

#define YD_OPENED_MASK      1
#define YD_RW_MASK          (1<<1)
#define YD_BS_MASK          0x00001ffc
#define YD_FSMIN_MASK       0x00ffe000
#define YD_FSMAX_MASK       0x01000000
#define YD_RS_MASK          0xfe000000


typedef union yd_flags {
    yd_flags ():datas(YD_FLAGS_VALUE) {}
    yd_flags (uint32_t datas):datas(datas) {}
    yd_flags(const yd_flags& other) : datas(other.datas.load()) {}
    yd_flags& operator=(const yd_flags& other) {
        if (this != &other) {
            datas.store(other.datas.load());
            
        }
        return *this;
    }
    
private:
    std::atomic<uint32_t> datas;
    
    struct {
        YDFLAGS_BITFIELD;
    };
    
public:
    uint32_t getFlags (uint32_t flag)
    {
        return datas.load() & flag;
    }
    
    void setFlags (uint32_t set)
    {
        datas.fetch_or(set);
    }

    void clearFlags (uint32_t clear)
    {
        datas.fetch_xor(clear);
    }
    
    void changeFlags (uint32_t set, uint32_t clear)
    {
        if ((set & clear) != 0) return;
        
        uint32_t oldf, newf;
        do {
            oldf = datas;
            newf = (oldf | set) & ~clear;
        } while (!std::atomic_compare_exchange_weak(&datas, &oldf, newf));
    }
    
}dataBits;

struct yd_logger {
    yd_logger ():bits(yd_flags()), fpath(NULL), fd(0), start_p(NULL), current_p(NULL), msize(0), msize_file(0), msize_max(0) {}
    yd_logger (char *fpath):bits(yd_flags()), fpath(fpath), fd(0), start_p(NULL), current_p(NULL), msize(0), msize_file(0), msize_max(0) {}
    yd_logger (char *fpath, bool readOnly):bits(yd_flags()), fpath(fpath), fd(0), start_p(NULL), current_p(NULL), msize(0), msize_file(0), msize_max(0)
    {
        if (readOnly == false) bits.clearFlags(YD_RW_MASK);
    }
    
private:
    dataBits bits;
    char *fpath;
    int fd;
    
    void *start_p;
    void *current_p;
    uint32_t msize;
    uint32_t msize_file;
    uint32_t msize_max;
    
public:
    int mmapFile (off_t offset);
    int mmapFile ()
    {
        return mmapFile(0);
    }
    
    int munmapFile (void *start, const uint32_t fsize);
    int munmapFile ()
    {
        return munmapFile(start_p, msize);
    }
    
    int increaseFileSize (uint32_t increasedSize);
    int increaseFileSize ()
    {
        uint32_t size = pageCountMin() * getpagesize() + msize;
        return increaseFileSize(size);
    }
    
    int mRecorde (void *start, const void *data, const size_t length);
    int mRecordeNext (const void *data, const size_t length);
    
    int syncData (void *start, const size_t length);
    int syncData (const uint32_t off, const size_t length)
    {
        return syncData((uint8_t *)start_p + off, length);
    }
    int syncAll ()
    {
        return syncData(start_p, msize);
    }
    
    std::string errorDescription (const int err);
    
    void setFilePath (const char *path)
    {
        if (hasOpened()) return ;
        fpath = const_cast<char *>(path);
    }
    
    void setReadWrite (bool rw)
    {
        if ((rw ^ (readWrite())) == 0)
            return;
        if (rw)
            bits.setFlags(YD_RW_MASK);
        else
            bits.clearFlags(YD_RW_MASK);
    }
    
    const uint32_t totalSize ()
    {
        return msize;
    }
    
    const uint32_t fileSizeMin ()
    {
        return pageCountMin() * getpagesize();
    }
    
    const uint32_t fileSizeMax ()
    {
        return msize_file;
    }
    
    bool hasOpened ()
    {
        return bits.getFlags(YD_OPENED_MASK);
    }
    
    bool readWrite ()
    {
        return bits.getFlags(YD_RW_MASK);
    }
    
    uint32_t blockSize ()
    {
        return bits.getFlags(YD_BS_MASK);
    }
    
    uint32_t pageCountMin ()
    {
        int ps = getpagesize();
        return (bits.getFlags(YD_FSMIN_MASK) + ps - 1) / ps;
    }
    
    uint32_t pageCountMax ()
    {
        int ps = getpagesize();
        return (bits.getFlags(YD_FSMAX_MASK) + ps - 1) / ps;
    }
    
    uint32_t reservedBC ()
    {
        return bits.getFlags(YD_RS_MASK) / blockSize();
    }
};


#endif /* YDMmapLogger_hpp */
