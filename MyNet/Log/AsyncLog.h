#ifndef ASYNC_LOG_H
#define ASYNC_LOG_H
#include "FixedBuffer.h"
#include "LogFile.h"
#include "Logger.h"
#include <vector>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <thread>

class AsyncLog{
public:
    AsyncLog(std::string &processName,
             int rollSize = 1024 * 1024 * 512,
             int dateCheckCount = 1024,
             int interval = 3);

    /**
     * @brief: 启用异步日志系统
     * @return {*}
     */    
    void turnOn();

    /**
     * @brief: 写入buffer的数据
     * @return {*}
     * @param {char} *buff 待写入的数据
     * @param {int} len 待写入数据的长度
     */    
    void append(const char *buff, int len);

    /**
     * @brief: 启动后台线程
     * @return {*}
     */    
    void start();

private:
    typedef std::mutex Mutex;
    typedef std::condition_variable Condition;
    typedef FixedBuffer<LARGE_BUFFER_SIZE> Buffer;
    typedef std::unique_ptr<Buffer> BufferPtr;
    typedef std::vector<BufferPtr> BufferVec;

    Mutex mutex;                // 保护各个buffer 和 cond
    Condition cond;             // 在缓冲区满的时候唤醒后台线程
    BufferPtr input1;           // 正在输入的buffer
    BufferPtr input2;           // 预备输入的buffer
    BufferVec inputBuffers;     // 存储准备写出的buffer

    std::string processName;    // 进程名
    int rollSize;               // 日志文件大小限制
    int dateCheckCount;         // 日志文件更新检查的append次数
    int interval;               // 强制刷新的时间间隔

    std::atomic<bool> running;  // 后端线程运行标志


    /**
     * @brief: 后端线程，输出信息至文件中 
     * @return {*}
     */    
    void backFunc();
};

#endif