#include "TCPServer.h"
#include "AsyncLog.h"
#include <assert.h>

TCPServer::TCPServer(EventLoop* loop, const Socket& socket, int poolSize)
:loop_(loop), acceptor_(loop_, socket), 
 pool_(loop_, poolSize), nextID_(0)
{
    
}

/**
 * @brief: 启动TCP服务器
 * @return {*}
 */    
void TCPServer::start()
{
    // 检查Loop是否为空
    if(loop_ == nullptr)
    {
        LOG_ERROR << "TCPServer: EventLoop Is Nullptr";
        return;
    }

    // 设置新连接到来时的回调函数
    acceptor_.setNewConnCallBack(std::bind(&TCPServer::handleNewConn, this, std::placeholders::_1));

    // 启动连接用的线程池
    pool_.start();

    // 启动Acceptor，即进入监听状态
    if(acceptor_.listen() == -1)
    {
        perror("TCPServer Start Failed");
        abort();
    }
}

/**
 * @brief: 处理新建连接
 * @param {Socket} client 表示客户端的Socket
 * @return {*}
 */     
void TCPServer::handleNewConn(Socket& client)
{
    // 获取分配给新连接的事件循环
    EventLoop *dispatched = pool_.next();

    // 创建新的连接并绑定对应的回调函数
    TCPConnection::SPtr newConn(
        new TCPConnection(dispatched, nextID_++, acceptor_.socket(), client));
    newConn->setRDCallBack(readCallBack_);
    newConn->setCloseCallBack(std::bind(&TCPServer::removeConn, this, std::placeholders::_1));

    // 添加到TCPServer的连接表中
    conns_[newConn->id()] = newConn;

    // 调用上层的新建连接回调函数
    if(newConnCallBack_)
    {
        newConnCallBack_(newConn);
    }

    // 在连接所属的事件循环中启动TCPConnection
    dispatched->runInLoop(std::bind(&TCPConnection::establish, newConn));
}

/**
 * @brief: 移除连接
 * @return {*}
 * @param {uint32_t} connID 连接对应的ID
 */    
void TCPServer::removeConn(uint32_t connID)
{
    // 若当前在Server事件循环中，则直接移除连接
    if(loop_->isInLoopThread())
    {
        removeConnInLoop(connID);
    }
    // 否则转移到 TCPServer 的事件循环中执行
    else
    {
        loop_->runInLoop(std::bind(&TCPServer::removeConnInLoop, this, connID));
    }
}

/**
 * @brief: 在TCPServer所属的事件循环中断开连接 
 *         -- removeConn可能由IO所属的事件循环调用，
 *            因此需要转移
 * @return {*}
 * @param {uint32_t} connID 连接ID
 */    
void TCPServer::removeConnInLoop(uint32_t connID)
{
    TCPConnection::SPtr conn = conns_[connID];
    // 调用关闭连接回调函数，关闭上层连接
    if(closeCallBack_)
    {
        closeCallBack_(conn);
    }

    // 从连接表中移除对应的连接
    int nr = conns_.erase(connID);
    assert(nr == 1);

    // 在连接所属的事件循环中调用destroy
    EventLoop *ioLoop = conn->loop();
    ioLoop->runInLoop(std::bind(&TCPConnection::destroy, conn));
}