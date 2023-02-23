#ifndef ACCEPTOR_H
#define ACCEPTOR_H
#include "EventLoop.h"
#include "Socket.h"
#include "Channel.h"
#include <functional>

class Acceptor{
public:
    typedef std::function<void(Socket &)> NewConnCallBack;

    Acceptor(EventLoop *loop, const Socket &socket);
    ~Acceptor();

    /**
     * @brief: 设置新建连接回调函数
     * @return {*}
     * @param {NewConnCallBack&} cb 新建连接回调函数
     */    
    void setNewConnCallBack(const NewConnCallBack& cb)
    {
        newConnCallBack_ = cb;
    }

    /**
     * @brief: 进入监听状态
     * @return {*}成功返回0，失败返回-1
     */    
    int listen();

    /*成员get函数*/
    const Socket &socket() { return sock_; }

private:
    EventLoop* loop_;                   // Acceptor所对应的事件循环
    Socket sock_;                       // Acceptor所负责的socket
    Channel listenChannel_;             // 监听用Channel

    NewConnCallBack newConnCallBack_;   // 新建连接的回调函数

    /**
     * @brief: socket 可读时创建新连接
     * @return {*}
     * @param {Timestamp} time fd可读的时刻
     */    
    void handleNewConn(Timestamp time);
};

#endif