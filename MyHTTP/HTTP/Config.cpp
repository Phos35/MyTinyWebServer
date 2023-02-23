#include "Config.h"
#include "ELThreadPool.h"
#include <getopt.h>


uint16_t Config::DEFAULT_PORT = 8000;
uint16_t Config::DEFAULT_DBPOOL_SZ = 30;
uint16_t Config::DEFAULT_ELPOOL_SZ = ELThreadPool::DEFAULT_SIZE;
uint16_t Config::DEFAULT_WORKERPOOL_SZ = 5;
Config::LogType Config::DEFAULT_LOG_TYPE = Config::LogType::FILE;
Config::Trigger Config::DEFAULT_TRIGGER = Config::Trigger::LT;

/**
 * @brief: 载入配置参数
 * @return {*}
 * @param {int} argc 参数数量
 * @param {char} **argv 参数字符串
 */    
void Config::loadConfig(int argc, char **argv)
{
    processName_ = argv[0];

    option longOpts[] = {
        {"port", required_argument, NULL, 'p'},
        {"dbpoll", required_argument, NULL, 'd'},
        {"eventloop", required_argument, NULL, 'e'},
        {"workerpool", required_argument, NULL, 'w'},
        {"logtype", required_argument, NULL, 'l'},
        {"trigger", required_argument, NULL, 't'},
        {"help", no_argument, NULL, 'h'},
        {0,0,0,0}
    };
    opterr = 1;
    while (1)
    {
        int index = 0;
        int ret = getopt_long(argc, argv, "p:d:e:w:l:t:", longOpts, &index);
        if (ret == -1)
        {
            break;
        }

        switch(ret)
        {
            case 'p': port_ = std::stoi(optarg); break;
            case 'd': dbConnNr_ = std::stoi(optarg); break;
            case 'e': eventLoopNr_ = std::stoi(optarg); break;
            case 'w': workerNr_ = std::stoi(optarg); break;
            case 'l': logType_ = checkLogType(optarg); break;
            case 't': trigger_ = checkTrigger(optarg); break;
            default: help(); break;
        }
    }

    printf("Port: %d\nDBCONN_POOL_SIZE: %d\nEVENTLOOP_POOL_SIZE: %d\nWORKER_POOL_SZ: %d\nLogType: %s\nTrigger: %s\n",
           port_, dbConnNr_, eventLoopNr_, workerNr_, logType_ == LogType::FILE ? "FILE" : "STDOUT", 
           trigger_ == Trigger::LT ? "LT" : "ET");
}

/**
 * @brief: 输出帮助信息 
 * @return {*}
 */    
void Config::help()
{
    printf("Usage:\n");
    printf("-p, --port=PORT_NUM            服务器端口号\n");
    printf("-d, --dbpool=POOL_SIZE         数据库连接池的容量\n");
    printf("-e, --eventloop=POOL_SIZE      TCP事件循环线程池的容量\n");
    printf("-w, --workerpool=POOL_SIZE     HTTP请求解析工作线程池的容量\n");
    printf("-l, --logtype=LOG_TYPE         日志输出的方式. F-输出到文件；S-标准输出\n");
    printf("-t, --trigger=TRIGGER_MODE     epoll针对TCP连接的触发模式. L-LT；E-ET\n");
    exit(-1);
}

/**
 * @brief: 判断日志输出方式
 * @return {*}
 * @param {string} &val
 */    
Config::LogType Config::checkLogType(const std::string &val)
{
    if(val == "F")
        return LogType::FILE;
    else if(val == "S")
        return LogType::STD;
    else
    {
        printf("Wrong LogType\n");
        help();
        return logType_;
    }
}

/**
 * @brief: 判断触发模式
 * @return {*}
 * @param {string} &val
 */    
Config::Trigger Config::checkTrigger(const std::string &val)
{
    if(val == "L")
        return Trigger::LT;
    else if(val == "E")
        return Trigger::ET;
    else
    {
        printf("Wrong Trigger Type\n");
        help();
        return trigger_;
    }
}