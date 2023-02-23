#ifndef LOG_FILE_H
#define LOG_FILE_H
#include "Timestamp.h"
#include "AppendFile.h"
#include <memory>
#include <unistd.h>

class LogFile{
public:
    LogFile(std::string& processName,
            int rollSize = 1024 * 1024 * 512, // 默认512MB
            int dateCheckCount = 1024         // 每1024次调用检查日期是否变更
            );

    /**
     * @brief: 向文件中写入buff
     * @return {*}
     * @param {char} *buff 待写入的数据
     * @param {int} len 待写入数据的长度
     */    
    void append(const char *buff, int len);

    /**
     * @brief: 刷新缓冲区
     * @return {*}
     */    
    void flush();

private:
    std::string processName;            // 进程名称

    std::unique_ptr<AppendFile> file;
    int rollSize;                       // 单个日志文件大小
    int dateCheckCount;                 // 检查日期变更的append次数
    int count;                          // append次数

    Timestamp startTime;                     // 日志文件创建的时间
    Timestamp lastRollTime;                  // 前一次滚动的时间


    /**
     * @brief: 滚动日志文件
     * @return {*}
     */    
    void roll();

    /**
     * @brief: 生成日志文件的名称
     * @return {*}
     */    
    std::string genFileName();
};

#endif