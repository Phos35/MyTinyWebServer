#ifndef FIXED_BUFFER_H
#define FIXED_BUFFER_H
#include <string.h>

#define SMALL_BUFFER_SIZE 4096                      // 小缓冲区大小 4K
#define LARGE_BUFFER_SIZE 1024 * SMALL_BUFFER_SIZE  // 大缓冲区大小 4M

template <int SIZE>
class FixedBuffer {
public:
    FixedBuffer()
    :begin(dataArr), end(dataArr + sizeof(dataArr)), cur(begin)
    {
        clear();
    }

    /**
     * @brief: 获取已写入内容的长度
     * @return {*}
     */    
    int length();

    /**
     * @brief: 获取可写入长度的内容
     * @return {*}
     */    
    int available();

    /**
     * @brief: 向缓冲区中写入buf中的数据
     * @return {*}
     * @param {char} *buf 待写入的数据
     * @param {int} len 待写入数据的长度
     */    
    void append(const char *buf, int len);

    /**
     * @brief: 获取缓冲区数组
     * @return {*}
     */    
    char *data();

    /**
     * @brief: 缓冲区数据置0
     * @return {*}
     */    
    void clear();

private:
    char dataArr[SIZE];     // 缓冲区数组
    char* begin;            // 缓冲区数组起始指针
    char* end;              // 缓冲区数组末尾指针
    char* cur;              // 缓冲区数组当前写入位置指针
};

template <int SIZE>
int FixedBuffer<SIZE>::length()
{
    return cur - begin;
}

template <int SIZE>
int FixedBuffer<SIZE>::available()
{
    return end - cur;
}

template <int SIZE>
void FixedBuffer<SIZE>::append(const char* buf, int len)
{
    // 缓冲区存在足够空间则写入
    if(available() > len)
    {
        memcpy(cur, buf, len);
        cur += len;
    }
}

template <int SIZE>
char* FixedBuffer<SIZE>::data()
{
    return dataArr;
}

template <int SIZE>
void FixedBuffer<SIZE>::clear()
{
    bzero(dataArr, sizeof(dataArr));
    cur = dataArr;
}

#endif