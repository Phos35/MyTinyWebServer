#ifndef HTTP_CONNECTION_H
#define HTTP_CONNECTION_H
#include <memory>
#include "TCPConnection.h"
#include "HTTPRequest.h"
#include "ThreadPool.hpp"

class HTTPConnection : public std::enable_shared_from_this<HTTPConnection>
{
public:
    typedef std::unique_ptr<HTTPConnection> UPtr;
    typedef std::shared_ptr<HTTPConnection> SPtr;
    typedef ThreadPool<SPtr> WorkerPool;

    HTTPConnection(const TCPConnection::SPtr &conn, WorkerPool* workerPool);

    // 优雅关闭连接
    void shutdown();

    /**
     * @brief: 发送数据
     * @return {*}
     * @param {sring} &content 待发送的数据
     */    
    void respond(const std::string &content);

    /*成员/属性 get 函数*/
    HTTPRequest &requestRef() { return request_; }
    bool keepAlive() { return request_.keepAlive(); }
    bool parsing() { return request_.parsing(); }
    int id() { return tcpConn_->id(); }

    /*set函数*/
    void setParsing(bool val) { request_.setParsing(val); }

private:
    TCPConnection::SPtr tcpConn_;    // TCP连接
    HTTPRequest request_;            // HTTP请求
    WorkerPool* workerPool_;         // 执行解析任务的线程池

    /**
     * @brief: 读取数据后的回调函数
     * @return {*}
     * @param {SPtr} &conn TCP连接
     */    
    void handleRead(const TCPConnection::SPtr &conn);
};

#endif