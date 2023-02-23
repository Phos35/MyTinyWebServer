#ifndef EVENT_LOOP_H
#define EVENT_LOOP_H
#include <pthread.h>
#include <mutex>
#include <functional>
#include <vector>
#include <atomic>
#include "EPoller.h"
#include "TimerQueue.h"

class EventLoop{
public:
    typedef std::function<void()> Task;
    typedef std::vector<Task> TaskList;
    typedef std::vector<Channel *> ChannelList;

    EventLoop();
    ~EventLoop();

    /**
     * @brief: 判断当前线程是否为EventLoop所属的线程
     * @return {*}
     */    
    bool isInLoopThread();

    /**
     * @brief: 断言当前线程为EventLoop所属的线程
     * @return {*}
     */    
    void assertInLoopThread();

    /**
     * @brief: 启动事件循环
     * @return {*}
     */    
    void run();

    /**
     * @brief: 在EPoller中更新Channel
     * @return {*}
     * @param {Channel} *channel 待更新的Channel
     */    
    void updateChannel(Channel *channel);

    /**
     * @brief: 在EPoller中移除Channel
     * @return {*}
     * @param {Channel} *channel 待移除的Channel
     */    
    void removeChannel(Channel *channel);

    /**
     * @brief: 在本线程执行任务
     * @return {*}
     * @param {Task} & 待处理的任务
     */    
    void runInLoop(Task &&task);

    /**
     * @brief: 退出事件循环
     * @return {*}
     */    
    void quit();

    /*定时器操作函数*/
    TimerID runAt(Timer::CallBack &&cb, Timestamp expiration);
    TimerID runEvery(Timer::CallBack &&cb, Timestamp expiration, uint64_t interval);
    void cancelTimer(TimerID id);

    /*成员/状态 get函数*/
    pthread_t threadID() { return threadID_; }
    bool taskHandling() { return tasksHandling_; }
    bool quited() { return quit_; }
    int wakefd() { return wakefd_; }

private:
    pthread_t threadID_;        // 事件循环所属线程的ID
    bool running_;              // 循环运行标志
    std::atomic<bool> quit_;    // 事件循环退出标志 为什么需要添加atomic--ELThread 析构时需要检测是否已退出
    bool eventHandling_;        // 正在处理事件的标志
    bool tasksHandling_;        // 正在处理Task的标志

    int wakefd_;                // 唤醒用文件描述符
    Channel wakeChannel_;       // 负责wakefd的Channel
    std::mutex mutex_;          // 函数vector的互斥锁
    TaskList tasks_;            // 其他线程转移至该事件循环/线程的任务

    EPoller epoller_;           // 事件发生检测器
    ChannelList activeChannels_;// 发生事件的Channel列表

    Timestamp eventsHappenTime_;// 事件发生的时刻

    TimerQueue timerQue_;       // 定时器队列

    /**
     * @brief: wakefd唤醒后的读回调函数
     * @return {*}
     */    
    void readWakefd(Timestamp time);

    /**
     * @brief: 将任务添加到tasks中并唤醒事件循环
     * @return {*}
     * @param {Task} & 待添加的task
     */    
    void addTask(Task &&task);

    /**
     * @brief: 处理其它线程转移过来的任务
     * @return {*}
     */    
    void handleTask();

    /**
     * @brief: 唤醒当前事件循环
     * @return {*}
     */    
    void wakeup();

    /**
     * @brief: 设置quit_ == true，使事件循环退出
     * @return {*}
     */    
    void quitInLoop();
};

#endif