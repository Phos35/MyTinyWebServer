#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H
#include "TCPServer.h"
#include "HTTPRequest.h"
#include "HTTPConnection.h"
#include "ThreadPool.hpp"
#include "Config.h"
#include <unordered_map>

class HTTPServer : public TCPServer
{
public:
    typedef std::function<void(const HTTPConnection::SPtr &)> CallBack;
    typedef EPoller::Trigger Trigger;

    HTTPServer(EventLoop* loop, const Socket& host, Trigger trigger = Trigger::LT,
               int connNr = ELThreadPool::DEFAULT_SIZE, 
               int workerNr = 5, int wokerQueSize = 10000);

    /**
     * @brief: 创建一个HTTPServer并运行（函数运行loop，阻塞）
     * @return {*}
     * @param {Config} &config
     */    
    static void createAndStart(Config config);

    /**
     * @brief: 启动HTTP服务器
     * @return {*}
     */    
    void startServer();

    /*设置处理回调函数*/
    void setOnGET(const CallBack &cb) { onGET_ = cb; }
    void setOnPOST(const CallBack &cb) { onPOST_ = cb; }
    void setOnErr(const CallBack &cb) { onErr_ = cb; }

private:
    typedef std::unordered_map<uint32_t, HTTPConnection::SPtr> ConnMap;
    typedef ThreadPool<HTTPConnection::SPtr> WorkerPool;

    ConnMap conns_;             // HTTPConnection 表
    int connNr_;                // TCP连接池的大小
    WorkerPool workerPool_;     // 解析报文的工作线程池
    Trigger trigger_;           // epoll 触发模式

    CallBack onGET_;            // GET请求的处理回调函数
    CallBack onPOST_;           // POST请求的处理回调函数
    CallBack onErr_;            // 请求处理出错的处理回调函数

    /**
     * @brief: 新连接到达
     * @return {*}
     * @param {SPtr&} conn 新的TCP连接
     */    
    void handleNewConn(const TCPConnection::SPtr& conn);
    
    /**
     * @brief: 连接关闭的回调函数
     * @return {*}
     * @param {SPtr&} conn 待关闭的连接
     */    
    void handleClose(const TCPConnection::SPtr& conn);

    /**
     * @brief: 解析错误处理函数
     * @return {*}
     * @param {HTTPConnection::SPtr&} conn 解析出错的HTTP连接
     */    
    void handleError(const HTTPConnection::SPtr& conn);

    /**
     * @brief: 解析HTTP请求报文的工作函数
     * @return {*}
     * @param {SPtr&} conn HTTP连接
     */    
    void worker(HTTPConnection::SPtr conn);
};

#endif