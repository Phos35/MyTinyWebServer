#ifndef CHANNEL_H
#define CHANNEL_H
#include <vector>
#include <stdint.h>
#include <functional>
#include <sys/epoll.h>
#include "Timestamp.h"

// FIXME 前向声明可能是不必要的
class EventLoop;

class Channel
{
public:
    typedef std::vector<Channel *> List;
    typedef std::function<void(Timestamp)> RDCallBack;
    typedef std::function<void()> CallBack;

    // Channel在EPoller中的状态
    enum State
    {
        NOT_ADD,    // 未添加至EPoller中
        ADDED       // 已添加至EPoller中
    };

    // 构造/析构函数函数
    Channel(int fd = -1, EventLoop* loop = nullptr)
    :fd_(fd), loop_(loop), 
     state_(NOT_ADD), handling_(false),
     activeEvents_(0), focusedEvents_(0)
    {}
    ~Channel();

    /**
     * @brief: 处理Channel上发生的事件
     * @param {Timestamp&} 事件发生的时刻
     * @return {*}
     */    
    void handleEvents(Timestamp& time);

    /**
     * @brief: 从EPoller中移除Channel
     * @return {*}
     */    
    void remove();

    /*设置回调函数*/
    void setRDCallBack(RDCallBack &&cb) { readCallBack_ = std::move(cb); }
    void setWRCallBack(CallBack &&cb) { writeCallBack_ = std::move(cb); }
    void setCloseCallback(CallBack &&cb) { closeCallBack_ = std::move(cb); }
    void setErrCallBack(CallBack &&cb) { errorCallBack_ = std::move(cb); }

    /*设置关注事件*/
    void focusRead() { focusedEvents_ |= (EPOLLIN | EPOLLPRI); update(); }
    void focusWrite() { focusedEvents_ |= EPOLLOUT; update(); writing_ = true; }
    void focusOneShot() { focusedEvents_ |= EPOLLONESHOT; update(); }
    void focusET() { focusedEvents_ |= EPOLLET; update(); }

    /*取消关注事件*/
    void unfocusRead() { focusedEvents_ &= ~(EPOLLIN | EPOLLPRI); update(); }
    void unfocusWrite() { focusedEvents_ &= ~EPOLLOUT; update(); writing_ = false; }
    void unfocusOneShot() { focusedEvents_ &= ~EPOLLONESHOT; update(); }
    void unfocusET() { focusedEvents_ &= ~EPOLLET; update(); }
    void unfocusAll() { focusedEvents_ &= 0; update(); }

    /*成员get函数*/
    int fd() { return fd_; }
    State state() { return state_; }
    bool handling() { return handling_; }
    uint32_t focusedEvents() { return focusedEvents_; }
    uint32_t activeEvents() { return activeEvents_; }
    bool writing() { return writing_; }

    /*成员set函数*/
    void setState(State s) { state_ = s; }
    void setActiveEvents(uint32_t e) { activeEvents_ = e; }

private:
    EventLoop *loop_;            // Channel 所属的事件循环
    int fd_;                     // Channel 监视的文件描述符
    State state_;                // Channel 在EPoller中的状态
    bool handling_;              // 正在处理事件的标志
    bool writing_;               // 正在写的标志

    uint32_t focusedEvents_;     // Channel 关注的事件
    uint32_t activeEvents_;      // Channel 发生的事件

    // 各事件回调函数
    RDCallBack readCallBack_;
    CallBack writeCallBack_;
    CallBack closeCallBack_;
    CallBack errorCallBack_;

    /**
     * @brief: 将Channel的状态更新到EPoller中
     * @return {*}
     */    
    void update();


};

#endif