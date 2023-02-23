#ifndef CONFIG_H
#define CONFIG_H
#include <stdint.h>
#include <string>
#include "EPoller.h"
#include "HTTPConnection.h"

class Config{
public:
    typedef std::function<void(const HTTPConnection::SPtr &)> CallBack;
    typedef EPoller::Trigger Trigger;

    // 日志写出方式
    enum LogType
    {
        FILE,
        STD
    };

    static uint16_t DEFAULT_PORT;           // 默认端口号
    static uint16_t DEFAULT_DBPOOL_SZ;      // 默认数据库连接池容量
    static uint16_t DEFAULT_ELPOOL_SZ;      // 默认事件循环池容量
    static uint16_t DEFAULT_WORKERPOOL_SZ;  // 默认工作线程池(请求报文解析线程)容量
    static LogType DEFAULT_LOG_TYPE;        // 默认日志写出的方式
    static Trigger DEFAULT_TRIGGER;         // 默认epoll触发模式

    Config(uint16_t port = DEFAULT_PORT, uint16_t dbConnNr = DEFAULT_DBPOOL_SZ,
           uint16_t eventLoopNr = DEFAULT_ELPOOL_SZ, uint16_t workerNr = DEFAULT_WORKERPOOL_SZ,
           LogType logType = DEFAULT_LOG_TYPE, Trigger trigger = DEFAULT_TRIGGER)
    :port_(port), dbConnNr_(dbConnNr), eventLoopNr_(eventLoopNr), workerNr_(workerNr),
     logType_(logType), trigger_(trigger) 
    {
    }

    /**
     * @brief: 载入配置参数
     * @return {*}
     * @param {int} argc 参数数量
     * @param {char} **argv 参数字符串
     */    
    void loadConfig(int argc, char **argv);

    /*获取HTTP针对不同请求的响应报文*/
    void setOnGET(const CallBack &cb) { onGET_ = cb; }
    void setOnPOST(const CallBack &cb) { onPOST_ = cb; }

    /*成员get*/
    std::string processName() { return processName_; }
    uint16_t port() { return port_; }
    uint16_t dbConnNr() { return dbConnNr_; }
    uint16_t eventLoopNr() { return eventLoopNr_; }
    uint16_t workerNr() { return workerNr_; }
    LogType logType() { return logType_; }
    Trigger trigger() { return trigger_; }
    CallBack &onGET() { return onGET_; }
    CallBack &onPOST() { return onPOST_; }

private:
    std::string processName_;
    uint16_t port_;
    uint16_t dbConnNr_;
    uint16_t eventLoopNr_;
    uint16_t workerNr_;
    LogType logType_;
    Trigger trigger_;

    CallBack onGET_;
    CallBack onPOST_;

    /**
     * @brief: 输出帮助信息 
     * @return {*}
     */    
    void help();

    /**
     * @brief: 判断日志输出方式
     * @return {*}
     * @param {string} &val
     */    
    LogType checkLogType(const std::string &val);

    /**
     * @brief: 判断触发模式
     * @return {*}
     * @param {string} &val
     */    
    Trigger checkTrigger(const std::string &val);
};

#endif