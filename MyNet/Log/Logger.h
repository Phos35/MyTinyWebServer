#ifndef LOGGER_H
#define LOGGER_H
#include <string>
#include <unordered_map>
#include <functional>
#include "Timestamp.h"
#include "LogStream.h"

/* 日志一般格式
+------+-----------+---------+------+-----+
| date | thread_id | content | file | line|
+------+-----------+---------+------+-----+
*/

class Logger
{
public:
    typedef std::string String;
    typedef std::function<void(const char*, int)> CallBack;

    enum Level
    {
        DEBUG,      // 调试信息
        TRACE,      // 追踪信息
        INFO,       // 一般信息
        WARN,       // 警告信息
        ERROR,      // 错误信息
    };

    // 调试级别的字符串
    static std::unordered_map<Level, std::string> LevelStr;

    // 输出回调函数
    static CallBack outputCallBack;

    /**
     * @brief: 初始化数据，填入date, thread_id, level部分
     * @return {*}
     * @param {String} file 文件名称
     * @param {int} line 行号
     * @param {Level} level 调试级别
     */    
    Logger(String file = String(), int line = 0, Level level = Level::DEBUG);

    /**
     * @brief: 析构，填入file 和 line
     * @return {*}
     */    
    ~Logger();

    /**
     * @brief: 获取缓冲区
     * @return {*}
     */
    LogStream &stream();

    /**
     * @brief: 设置输出的回调函数
     * @return {*}
     * @param {CallBack} cb 回调函数
     */    
    static void setOutput(CallBack cb);

    /**
     * @brief: 默认的输出函数
     * @return {*}
     */    
    static void defaulOutput(const char* buff, int len);

private:
    Level level;            // 调试级别
    Timestamp time;              // 时间
    String sourceFile;      // 源文件
    int line;               // 行号

    LogStream logStream;    // 缓冲区
};

// 定义便于输出的hong
#define LOG_DEBUG Logger(__FILE__, __LINE__, Logger::Level::DEBUG).stream()
#define LOG_TRACE Logger(__FILE__, __LINE__, Logger::Level::TRACE).stream()
#define LOG_INFO Logger(__FILE__, __LINE__, Logger::Level::INFO).stream()
#define LOG_WARN Logger(__FILE__, __LINE__, Logger::Level::WARN).stream()
#define LOG_ERROR Logger(__FILE__, __LINE__, Logger::Level::ERROR).stream()

#endif