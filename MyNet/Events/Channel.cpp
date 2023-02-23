#include "Channel.h"
#include "EventLoop.h"
#include "AsyncLog.h"
#include <assert.h>

/**
 * @brief: 处理Channel上发生的事件
 * @param {Timestamp&} time 事件发生的时刻
 * @return {*}
 */    
void Channel::handleEvents(Timestamp& time)
{
    handling_ = true;
    LOG_TRACE << "fd " << fd_ << ", happend events: " << EPoller::eventStr(activeEvents_);

    // 发生错误事件
    if((activeEvents_ & EPOLLERR) && errorCallBack_)
    {
        errorCallBack_();
    }
    // 发生读事件
    if(activeEvents_ & (EPOLLIN | EPOLLPRI))
    {
        if(readCallBack_)
        {
            readCallBack_(time);
        }
        else
        {
            printf("ReadCallBack Not Set!\n");
        }
    }
    // 发生写事件
    if(activeEvents_ & EPOLLOUT)
    {
        if(writeCallBack_)
        {
            writeCallBack_();
        }
        else
        {
            printf("WriteCallBack Not Set!\n");
        }
    }

    handling_ = false;
}

/**
 * @brief: 从EPoller中移除Channel
 * @return {*}
 */    
void Channel::remove()
{
    loop_->assertInLoopThread();
    loop_->removeChannel(this);
}

/**
 * @brief: 将Channel的状态通过事件循环更新到EPoller中
 * @return {*}
 */    
void Channel::update()
{
    loop_->assertInLoopThread();
    loop_->updateChannel(this);
}

Channel::~Channel()
{
    assert(handling_ == false);
}