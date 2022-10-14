#include "ELThread.h"
#include <thread>

/**
 * @brief: 创建一个线程，启动事件循环
 * @return {*} 启动了的事件循环
 */    
EventLoop *ELThread::startLoop()
{
    // 创建一个运行事件循环的线程
    std::thread th(&ELThread::threadFunc, this);
    th.detach();

    // 等待事件循环创建完毕
    {
        std::unique_lock<Mutex> lck(mutex_);
        while(loop_ == nullptr)
        {
            condition_.wait(lck);
        }
    }

    // 返回事件循环的指针
    return loop_;
}

    /**
 * @brief: 新线程的线程函数，创建并运行事件循环
 * @return {*}
 */    
void ELThread::threadFunc()
{
    // 创建事件循环
    EventLoop loop;

    // 调用初始化函数进行处理
    if(init_)
    {
        init_(&loop);
    }

    // 赋值事件循环指针，并通知事件循环创建完毕
    {
        std::unique_lock<Mutex> lck(mutex_);
        loop_ = &loop;
        condition_.notify_one();
    }

    // 运行事件循环
    loop.run();
}

ELThread::~ELThread()
{
    // 若事件循环退出，则退出循环
    if(loop_->quited() == false)
    {
        loop_->quit();
    }
}