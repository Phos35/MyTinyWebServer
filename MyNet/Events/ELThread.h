#ifndef EVENT_LOOP_THREAD_H
#define EVENT_LOOP_THREAD_H
#include "EventLoop.h"
#include <mutex>
#include <condition_variable>

class ELThread{
public:
    typedef std::function<void(EventLoop*)> InitCallBack;
    ELThread():loop_(nullptr){}
    ~ELThread();

    /**
     * @brief: 创建一个线程，启动事件循环
     * @return {*} 启动了的事件循环
     */    
    EventLoop *startLoop();

    /**
     * @brief: 设置初始化回调函数
     * @return {*}
     * @param {InitCallBack} &cb
     */
    void setInitCallBack(const InitCallBack &cb) { init_ = cb; }

private:
    typedef std::mutex Mutex;
    typedef std::condition_variable Cond;

    EventLoop *loop_;   // 线程所拥有的事件循环

    Mutex mutex_;       // condition使用的互斥锁
    Cond condition_;    // 跨线程获取loop的条件变量

    InitCallBack init_;  // 用于初始化的回调函数

    /**
     * @brief: 新线程的线程函数，创建并运行事件循环
     * @return {*}
     */    
    void threadFunc();

};

#endif