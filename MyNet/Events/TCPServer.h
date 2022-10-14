#ifndef TCPSERVER_H
#define TCPSERVER_H
#include "ELThreadPool.h"
#include "Acceptor.h"
#include "TCPConnection.h"
#include <unordered_map>
#include <functional>

class TCPServer{
public:
    typedef std::function<void(const TCPConnection::SPtr &)> ConnCallBack;
    typedef TCPConnection::RDCallBack RDCallBack;

    TCPServer(EventLoop* loop, const Socket& socket, int poolSize = ELThreadPool::DEFAULT_SIZE);

    /*设置相关回调函数*/
    // 设置新建连接回调函数
    void setNewConnCallBack(const ConnCallBack& cb)
    {
        newConnCallBack_ = cb;
    }

    // 设置连接关闭回调函数
    void setCloseConnCallBack(const ConnCallBack& cb)
    {
        closeCallBack_ = cb;
    }

    // 设置读事件回调函数
    void setReadCallBack(const RDCallBack& cb)
    {
        readCallBack_ = cb;
    }

    /**
     * @brief: 启动TCP服务器
     * @return {*}
     */    
    void start();

    /*set系列函数*/
    void setLoop(EventLoop *loop) { loop_ = loop; }

private:
    typedef std::unordered_map<uint32_t, TCPConnection::SPtr> ConnMap;

    EventLoop *loop_;       // TCPServer 所属的事件循环
    Acceptor acceptor_;     // TCPServer 的 Acceptor
    ELThreadPool pool_;     // 事件循环线程池，用于分配给客户端连接（轮询式）

    ConnMap conns_;         // 已建立的连接
    uint32_t nextID_;       // 下一个TCPConnection的ID

    ConnCallBack newConnCallBack_;      // 新建连接回调函数
    ConnCallBack closeCallBack_;        // 连接关闭回调函数
    RDCallBack readCallBack_;           // 读事件回调函数

    /**
     * @brief: 处理新建连接
     * @param {Socket} client 表示客户端的Socket
     * @return {*}
     */    
    void handleNewConn(Socket& client);

    /**
     * @brief: 断开连接
     * @return {*}
     * @param {uint32_t} connID 连接ID
     */    
    void removeConn(uint32_t connID);

    /**
     * @brief: 在TCPServer所属的事件循环中断开连接 
     *         -- removeConn可能由IO所属的事件循环调用，
     *            因此需要转移
     * @return {*}
     * @param {uint32_t} connID 连接ID
     */    
    void removeConnInLoop(uint32_t connID);
};

#endif