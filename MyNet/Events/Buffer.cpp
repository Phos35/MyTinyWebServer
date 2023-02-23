#include "Buffer.h"
#include "AsyncLog.h"
#include <sys/uio.h>

uint32_t Buffer::DEFAULT_SIZE = 1024;

/**
 * @brief: 从文件描述符中读取内容
 * @return {*}
 * @param {int} fd 待读取的文件描述符
 * @param {Trigger} trigger epoll触发模式
 */    
int Buffer::readfd(int fd, EPoller::Trigger trigger)
{
    // 额外缓冲区
    char tmp[65536] = {0};

    // 使用聚集IO
    iovec readBuf[2];
    readBuf[0].iov_base = (void*)wrPtr();
    readBuf[0].iov_len = writeable();
    readBuf[1].iov_base = tmp;
    readBuf[1].iov_len = sizeof(tmp);

    LOG_TRACE << "Buffer Writable: " << writeable();

    // 读取文件描述符
    ssize_t readed = readv(fd, readBuf, 2);
    if(readed == -1)
    {
        // ET模式下读取到-1是正常行为, 无需报错；而LT需要报错
        if(trigger == EPoller::Trigger::LT)
            LOG_TRACE << "Buffer Read Failed" << strerror(errno);
    }
    // 若读取内容小于等于缓冲区可写空间，则直接改变指针即可
    else if(readed <= writeable())
    {
        wrIndex_ += readed;
    }
    // 读取内容长度大于可写空间大小，则需要扩展缓冲区并写入
    else
    {
        int wrSpace = writeable();
        wrIndex_ = size();
        append(tmp, readed - wrSpace);
    }

    return readed;
}

/**
 * @brief: 获取缓冲区内容
 * @return {*}
 * @param {int} size 读取的长度
 */    
std::string Buffer::readAsString(int size)
{
    if(size >= readable())
    {
        return readAll();
    }
    else
    {
        std::string ret(rdPtr(), size);
        rdIndex_ += size;
        return ret;
    }
}

/**
 * @brief: 读取缓冲区中的全部内容
 * @return {*}
 */    
std::string Buffer::readAll()
{
    std::string ret(rdPtr(), readable());
    release(readable());
    return ret;
}

/**
 * @brief: 向缓冲区中写入内容
 * @return {*}
 * @param {char*} content 待写入的内容
 * @param {size_t} size 待写入内容的长度
 */    
void Buffer::append(const char* content, size_t size)
{
    // 确保可写空间足够
    ensureWriteable(size);

    // 写入缓冲区
    std::copy(content, content + size, wrPtr());
    wrIndex_ += size;
}

/**
 * @brief: 确保缓冲区中有足够的空间可写(即可能扩展空间)
 * @param {int} len 需要写的长度
 * @return {*}
 */    
void Buffer::ensureWriteable(int len)
{
    // 若可写空间小于需要写入内容的长度，则需要创建足够的空间
    if(writeable() < len)
    {
        makeSpace(len);
    }
}

/**
 * @brief: 扩展缓冲区的可写空间 -- 可能需要申请更大的空间
 * @return {*}
 * @param {int} len 待写入内容的长度
 */    
void Buffer::makeSpace(int len)
{
    // 若 writable + prespace >= len, 则只需要移动缓冲区的指针即可
    if(writeable() + prespace() >= len)
    {
        uint32_t rdSpaceSize = readable();
        std::copy(rdPtr(), rdPtr() + rdSpaceSize, content_.data());

        rdIndex_ -= rdSpaceSize;
        wrIndex_ = rdIndex_ + rdSpaceSize;
    }
    // 否则直接在可写空间后申请足够的空间
    else
    {
        content_.resize(wrIndex_ + len);
    }
}

/**
 * @brief: 释放可读区域的前size个字节
 * @return {*}
 * @param {int} size 
 */    
void Buffer::release(int size)
{
    if(size < readable())
    {
        rdIndex_ += size;
    }
    else
    {
        rdIndex_ = wrIndex_ = 0;
    }
} 