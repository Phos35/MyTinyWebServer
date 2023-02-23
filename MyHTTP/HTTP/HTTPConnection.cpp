#include "HTTPConnection.h"

HTTPConnection::HTTPConnection(const TCPConnection::SPtr &conn, WorkerPool* workerPool)
:tcpConn_(conn), workerPool_(workerPool), request_(conn->inputPtr())
{
    // 设置读事件回调
    tcpConn_->setRDCallBack(std::bind(&HTTPConnection::handleRead, this, std::placeholders::_1));
}

// 优雅关闭连接
void HTTPConnection::shutdown()
{
    tcpConn_->shutdown();
}

/**
 * @brief: 发送数据
 * @return {*}
 * @param {sring} &content 待发送的数据
 */    
void HTTPConnection::respond(const std::string &content)
{
    tcpConn_->send(content);
}

/**
 * @brief: 读取数据后的回调函数
 * @return {*}
 * @param {SPtr} &conn TCP连接
 */    
void HTTPConnection::handleRead(const TCPConnection::SPtr &conn)
{
    // 设置正在解析的标志
    setParsing(true);

    // HTTP请求放入任务队列中，等待工作线程解析
    workerPool_->push(shared_from_this());
}