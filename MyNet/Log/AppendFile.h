#ifndef APPEND_FILE_H
#define APPEND_FILE_H
#include <stdio.h>
#include <string>
#include <string.h>

class AppendFile{
public:
    AppendFile(std::string fileName);
    ~AppendFile();

    /**
     * @brief: 向缓冲区中添加数据
     * @return {*}
     * @param {char} *buff 待写入的数据
     * @param {int} len 待写入数据的长度
     */    
    void append(const char *buff, int len);

    /**
     * @brief: 刷新缓冲区，强制写入文件
     * @return {*}
     */    
    void fflush();

    /**
     * @brief: 获取已写入的字符数量
     * @return {*}
     */    
    int written();

private:
    FILE *file;                 // 目标文件的文件指针
    char buffer[64 * 1024];     // 流缓冲区
    int writtenBytes;           // 已写入的字节数

    /**
     * @brief: 向缓冲区中写入数据
     * @return {*}
     * @param {char} *buff 待写入的数据
     * @param {int} len 待写入数据的长度
     */    
    int write(const char *buff, int len);
};
#endif