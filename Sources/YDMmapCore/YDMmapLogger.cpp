//
//  YDMmapLogger.cpp
//
//  Created by wangyuandong on 2021/9/22.
//  Cross-platform mmap logger core — shared by iOS and Android.
//

#include "YDMmapLogger.h"
#include <fcntl.h>
#include <sys/errno.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <string.h>

#ifdef __APPLE__
#include <sys/mount.h>
#elif defined(__ANDROID__) || defined(__linux__)
#include <sys/vfs.h>
#endif

#ifndef MAP_FILE
#define MAP_FILE 0
#endif

/**
 获取磁盘信息需要的文件目录

 iOS 真机: /var
 iOS 模拟器 / macOS: /
 Android: /data
 */
#if defined(__ANDROID__)
#   define STATFS_DIR "/data"
#elif __x86_64__  ||  __i386__
#   define STATFS_DIR "/"
#else
#   define STATFS_DIR "/var"
#endif


static int returnErr (int err, int fd);

int yd_logger::mmapFile (off_t offset)
{
    if (fpath == NULL)
        return YD_NOPATH;
    
    struct stat statInfo;
    
    bool rw = readWrite();
    uint32_t pc_min = pageCountMin();
    uint32_t pc_max = pageCountMax();
    uint32_t rbc = reservedBC();
    uint32_t bs = blockSize();
    int ps = getpagesize();
    
    if (fd) close(fd);
    fd = open(fpath, rw ? O_RDWR : O_RDONLY);
    if (fd < 0) return errno;
    
    if (fstat(fd, &statInfo) != 0)
        return returnErr(errno, fd);
    
    uint32_t length = pc_max * ps;
    if (statInfo.st_size > length)
        return returnErr(YD_EXCESSIVE, fd);
    msize = (uint32_t)statInfo.st_size;
    
    uint32_t msize_min = pc_min * ps;
    if (msize)
        length = ((msize + msize_min - 1) / msize_min) * msize_min;
    else
        length = pc_min * ps;
    
    if (rw) {
        struct statfs diskInfo;
        if (statfs(STATFS_DIR, &diskInfo) < 0)
            return returnErr(errno, fd);
        
        int64_t freeBlk = diskInfo.f_bavail - (rbc / (diskInfo.f_bsize / bs));
        if (freeBlk < (length + diskInfo.f_bsize - 1) / diskInfo.f_bsize)
            return returnErr(YD_NOSPACE, fd);
        
        if (msize < length) {
            if (ftruncate(fd, length) != 0)
                return returnErr(errno, fd);
        }
        
        start_p = mmap(NULL, length, PROT_READ | PROT_WRITE, MAP_FILE | MAP_SHARED, fd, offset);
    }
    else {
        if (msize < 1) return YD_NOFILE;
        start_p = mmap(NULL, msize, PROT_READ, MAP_SHARED, fd, offset);
    }
    
    if (start_p == MAP_FAILED) {
        msize = 0;
        start_p = NULL;
        return returnErr(errno, fd);
    }
    
    current_p = rw ? ((uint8_t *)start_p + msize) : start_p;
    msize_file = length;
    msize_max = pc_max * ps;
    bits.setFlags(YD_OPENED_MASK);
    
    return 0;
}

int yd_logger::munmapFile(void *start, const uint32_t fsize)
{
    if (!hasOpened()) return YD_UNOPENED;
    if (start == NULL) return YD_NOSTART;
    
    int errCode = 0;
    errCode = munmap(start, fsize);
    bits.clearFlags(YD_OPENED_MASK);
    
    uint32_t length_max = msize_file;
    msize = 0;
    msize_file = 0;
    msize_max = 0;
    
    if (readWrite() && fpath != NULL && fsize < length_max) {
        
        start_p = NULL;
        current_p = NULL;
        fpath = NULL;
        
        if (!fd) return YD_INVALIDFD;
        
        if (ftruncate(fd, fsize) != 0)
            return returnErr(errno, fd);
        
        close(fd);
        fd = 0;
    }
    else {
        start_p = NULL;
        current_p = NULL;
        fpath = NULL;
        if (fd) close(fd);
        fd = 0;
    }
    
    return errCode;
}

int yd_logger::increaseFileSize (uint32_t increasedSize)
{
    if (!fd) return YD_INVALIDFD;
    if (!hasOpened()) return YD_UNOPENED;
    
    uint32_t rbc = reservedBC();
    uint32_t bs = blockSize();
    uint32_t msize_min = pageCountMin() * getpagesize();
    
    uint32_t length = ((increasedSize + msize_min - 1) / msize_min) * msize_min;
    
    if (length > msize_max) return YD_EXCESSIVE;
    
    struct statfs diskInfo;
    if (statfs(STATFS_DIR, &diskInfo) < 0)
        return errno;
    
    int64_t freeBlk = diskInfo.f_bavail - (rbc / (diskInfo.f_bsize / bs));
    if (freeBlk < (length + diskInfo.f_bsize - 1) / diskInfo.f_bsize)
        return YD_NOSPACE;
    
    if (ftruncate(fd, length) != 0)
        return errno;
    
    munmap(start_p, msize_file);
    bits.clearFlags(YD_OPENED_MASK);
    
    void *new_sp, *old_sp;
    void *new_cp, *old_cp;
    new_sp = mmap(NULL, length, PROT_READ | PROT_WRITE, MAP_FILE | MAP_SHARED, fd, 0);
    if (new_sp == MAP_FAILED)
        return errno;
    
    do {
        old_sp = start_p;
    } while (YD_LIKELY(!std::atomic_compare_exchange_strong(
                                                            reinterpret_cast<std::atomic<void*>*>(&start_p), &old_sp, new_sp)));
    
    do {
        old_cp = current_p;
        new_cp = reinterpret_cast<uint8_t*>(start_p) + msize;
    } while (YD_LIKELY(!std::atomic_compare_exchange_strong(
                                                            reinterpret_cast<std::atomic<void*>*>(&current_p), &old_cp, new_cp)));
    
    msize_file = length;
    bits.setFlags(YD_OPENED_MASK);
    return 0;
}

int yd_logger::mRecorde(void *start, const void *data, const size_t length)
{
    if (YD_UNLIKELY(start == NULL)) return YD_NOSTART;
    
    if (YD_UNLIKELY((uint8_t *)start_p + msize_file < (uint8_t *)start + length))
        return YD_NORAM;
    
    memcpy(start, data, length);
    
    return 0;
}

int yd_logger::mRecordeNext(const void *data, const size_t length)
{
    if (YD_UNLIKELY(current_p == NULL)) return YD_NOSTART;
    if (YD_UNLIKELY(msize_file < msize + length)) return YD_NORAM;
    
    memcpy(current_p, data, length);
    
    void *oldf, *newf;
    uint32_t oldi, newi;
    
    do {
        oldi = msize;
        newi = msize + static_cast<uint32_t>(length);
    } while (YD_LIKELY(!std::atomic_compare_exchange_strong(
                                                            reinterpret_cast<std::atomic<uint32_t>*>(&msize), &oldi, newi)));
    
    do {
        oldf = current_p;
        newf = reinterpret_cast<uint8_t*>(current_p) + length;
    } while (YD_LIKELY(!std::atomic_compare_exchange_strong(
                                                            reinterpret_cast<std::atomic<void*>*>(&current_p), &oldf, newf)));

    return 0;
}

int yd_logger::syncData(void *start, const size_t length)
{
    if (start == NULL) return YD_NOSTART;
    if (length == 0) return 0;
    
    if (msync(start, length, MS_SYNC | MS_ASYNC) == 0) {
        return 0;
    }
    else {
        return errno;
    }
}

std::string yd_logger::errorDescription(const int err)
{
    std::string errDes;
    
    switch (err) {
        case EEXIST:
            errDes = "文件已存在,却使用了O_CREAT和O_EXCL旗标";
            break;
        case EACCES:
            errDes = "文件不符合所要求的权限";
            break;
        case EROFS:
            errDes = "欲测试写入权限的文件存在于只读文件系统内";
            break;
        case EINVAL:
            errDes = "一个或者多个参数无效";
            break;
        case EIO:
            errDes = "I/O存取错误";
            break;
        case EBADF:
            errDes = "文件描述词无效";
            break;
        case ENOENT:
            errDes = "路径名的部分组件不存在，或路径名是空字串";
            break;
        case ENOMEM:
            errDes = "核心内存不足";
            break;
        case EFAULT:
            errDes = "地址空间不可访问";
            break;
        case ELOOP:
            errDes = "遍历路径时遇到太多的符号连接";
            break;
        case ENAMETOOLONG:
            errDes = "文件路径名太长";
            break;
        case ENOTDIR:
            errDes = "路径名的部分组件不是目录";
            break;
        case EAGAIN:
            errDes = "文件已被锁定，或者太多的内存已被锁定";
            break;
        case ENFILE:
            errDes = "已达到系统对打开文件的限制";
            break;
        case ENODEV:
            errDes = "指定文件所在的文件系统不支持内存映射";
            break;
        case EPERM:
            errDes = "权能不足，操作不允许";
            break;
        case ETXTBSY:
            errDes = "已写的方式打开文件，同时指定MAP_DENYWRITE标志";
            break;
        case EBUSY:
            errDes = "已写的方式打开文件，同时指定MAP_DENYWRITE标志";
            break;
            
            /* 自定义errCode */
        case YD_NOPATH:
            errDes = "文件路径为NULL";
            break;
        case YD_NOSPACE:
            errDes = "磁盘空间不足，请先清理磁盘";
            break;
        case YD_EXCESSIVE:
            errDes = "文件过大";
            break;
        case YD_NOFILE:
            errDes = "文件无内容，无法打开";
            break;
        case YD_UNOPENED:
            errDes = "没有文件被映射";
            break;
        case YD_INVALIDFD:
            errDes = "无效的文件描述符";
            break;
        case YD_NOSTART:
            errDes = "无起始地址，写入数据失败";
            break;
        case YD_NORAM:
            errDes = "数据过大，无法写入";
            break;
        default:
            errDes = "未找到错误描述，请自行查阅文档";
            break;
    }
    
    return errDes;
}


static int returnErr (int err, int fd)
{
    close(fd);
    return err;
}
