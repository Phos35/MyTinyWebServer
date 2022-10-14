#ifndef BUFFER_H
#define BUFFER_H
#include <vector>
#include <string>
#include <atomic>
#include "EPoller.h"

/*Bufeer 模型
+-------------+------------------+-------------------+
|  pre_space  |  readable_space  |  writeable_space  |
+-------------+------------------+-------------------+
|             |                  |                   |
0          rdIndex            wrIndex               size
*/

class Buffer{
public:
    Buffer(int size = Buffer::DEFAULT_SIZE)
    : content_(size, 0), rdIndex_(0), wrIndex_(0), handling_(false)
    {}

    /**
     * @brief: 从文件描述符中读取内容
     * @return {*}
     * @param {int} fd 待读取的文件描述符
     * @param {Trigger} trigger epoll触发模式
     */    
    int readfd(int fd, EPoller::Trigger trigger);

    /**
     * @brief: 获取缓冲区内容
     * @return {*}
     * @param {int} size 读取的长度
     */    
    std::string readAsString(int size);

    /**
     * @brief: 读取缓冲区中的全部内容
     * @return {*}
     */    
    std::string readAll();

    /**
     * @brief: 向缓冲区中写入内容
     * @return {*}
     * @param {char*} content 待写入的内容
     * @param {size_t} size 待写入内容的长度
     */    
    void append(const char* content, size_t size);
    void append(const std::string &content) { append(content.c_str(), content.size()); }

    /*成员/属性 get 函数*/
    uint32_t prespace() { return rdIndex_; }
    uint32_t readable() { return wrIndex_ - rdIndex_; }
    uint32_t writeable() { return content_.size() - wrIndex_; }
    uint32_t size() { return content_.size(); }
    const char *data() { return content_.data(); }
    const char *rdPtr() { return content_.data() + rdIndex_; }
    char *wrPtr() { return content_.data() + wrIndex_; }
    bool handling() { return handling_.load(); }

    /*set函数*/
    void setHandling(bool val) { handling_.store(val); }

private:    
    static uint32_t DEFAULT_SIZE;       // 缓冲区最小/初始大小

    std::vector<char> content_;     // 缓冲区存储的内容
    uint32_t rdIndex_;              // 可读区域的起始下标
    uint32_t wrIndex_;              // 可写区域的起始下标

    std::atomic<bool> handling_;    // 正在处理缓冲区中的数据的标志

    /**
     * @brief: 确保缓冲区中有足够的空间可写(即可能扩展空间)
     * @param {int} len 需要写的长度
     * @return {*}
     */    
    void ensureWriteable(int len);

    /**
     * @brief: 扩展缓冲区的可写空间 -- 可能需要申请更大的空间
     * @return {*}
     * @param {int} len 待写入内容的长度
     */    
    void makeSpace(int len);

    /**
     * @brief: 释放可读区域的前size个字节
     * @return {*}
     * @param {int} size 
     */    
    void release(int size);
};

#endif