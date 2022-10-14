#ifndef TCP_CONNECTION_H
#define TCP_CONNECTION_H
#include <memory>
#include <functional>
#include "Socket.h"
#include "EventLoop.h"
#include "Channel.h"
#include "Buffer.h"

class TCPConnection : public std::enable_shared_from_this<TCPConnection>
{
public:
    typedef std::shared_ptr<TCPConnection> SPtr;
    typedef std::function<void(const SPtr&)> RDCallBack;
    typedef std::function<void(uint32_t)> CLSCallBack;
    typedef EPoller::Trigger Trigger;

    // TCP连接状态
    enum State
    {
        CONNECTING,     // 连接中，对应从accept到调用establish之前的阶段
        CONNECTED,      // 已连接，对应调用establish之后
        DISCONNECTING,  // 对应read == 0 或者主动close之后
        SHUTDOWN,       // 优雅关闭连接中
        DISCONNECTED    // 对应调用destroy之后
    };  

    TCPConnection(EventLoop* loop, uint32_t id, const Socket &host, const Socket& peer);
    ~TCPConnection();

    /*设置相关的回调函数*/
    // 设置读事件回调函数
    void setRDCallBack(const RDCallBack& cb) { readCallBack_ = cb; }

    // 设置连接关闭回调函数
    void setCloseCallBack(const CLSCallBack& cb) { closeCallBack_ = cb; }

    /**
     * @brief: 启动TCPConnection
     * @return {*}
     */    
    void establish();

    /**
     * @brief: 连接关闭，销毁相应资源
     * @return {*}
     */    
    void destroy();

    /**
     * @brief: 发送数据
     * @return {*}
     * @param {string} &content 待发送的内容
     */    
    void send(const std::string &content);

    /**
     * @brief: 优雅关闭连接，即只关闭读端，写端待数据写完后再关闭
     * @return {*}
     */    
    void shutdown();

    /*成员/属性get函数*/
    EventLoop *loop() { return loop_; }
    uint32_t id() { return id_; }
    Socket& peer() { return peer_; }
    Socket& host() { return host_; }
    Buffer &input() { return input_; }
    Buffer *inputPtr() { return &input_; }

    /*set函数*/
    void setTrigger(Trigger val) { trigger_ = val; }

private:
    EventLoop* loop_;           // TCPConnection 所属的事件循环
    uint32_t id_;               // TCPConnection 在TCPServer中的id
    Socket host_;               // 本端对应的 socket
    Socket peer_;               // 对端对应的 socket
    Channel channel_;           // 本次连接对应的 Channel
    State state_;               // TCPConnection 的状态
    Trigger trigger_;           // 触发模式
    TimerID timer_;            // 定时器id

    RDCallBack readCallBack_;   // 读回调函数
    CLSCallBack closeCallBack_; // 连接关闭回调函数

    Buffer input_;              // 读缓冲区
    Buffer output_;             // 写缓冲区

    /*处理各类事件*/
    void handleRead(Timestamp time);
    void readLT(Timestamp time);
    void readET(Timestamp time);
    void handleWrite();
    int writeLT();
    int writeET();
    int firstWriteLT(const std::string &content);
    int firstWriteET(const std::string &content);
    void handleClose();

    /**
     * @brief: 在 TCPConnection 所属的事件循环中执行发送数据的任务
     * @return {*}
     * @param {string} content
     */    
    void sendInLoop(std::string content);

    /**
     * @brief: 将优雅关闭连接的任务转移到IO事件循环中执行
     * @return {*}
     */    
    void shutdownInLoop();

    /**
     * @brief: 状态字符串 
     * @return {*}
     * @param {State} s 状态
     */    
    std::string stateStr(State s);
};

#endif