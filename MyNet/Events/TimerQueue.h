#ifndef TIMER_QUEUE_H
#define TIMER_QUEUE_H
#include "Channel.h"
#include "Timestamp.h"
#include "Timer.h"
#include "TimerID.h"
#include <set>

class EventLoop;

class TimerQueue
{
public:
    typedef std::pair<Timestamp, Timer*> Entry;
    typedef std::set<Entry> TQueue;
    typedef std::set<Timer *> CQueue;

    TimerQueue(EventLoop *loop = nullptr);

    /**
     * @brief: 添加定时器
     * @return {*}
     * @param {Timestamp} expiration 定时器超时时间点
     * @param {uint64_t} interval 定时器重复触发间隔。=0表示只触发一次
     * @param {CallBack&&} callback 定时器到期后的处理回调函数
     */    
    TimerID addTimer(Timestamp expiration, uint64_t interval, Timer::CallBack&& callback);

    /**
     * @brief: 取消指定定时器
     * @return {*}
     * @param {TimerID&} id 带取消定时器的id
     */    
    void cancelTimer(TimerID id);

private:
    EventLoop *loop_;        // TimerQueue 所属的事件循环

    int timerfd_;            // 用于定时唤醒的fd
    Channel timerChannel_;   // 负责timerfd的Channel

    TQueue timers_;          // 存储定时器的队列
    CQueue cancelled_;       // 标记重复取消的定时器

    bool handlingExpire_;    // 正在处理定时器事件的标志

    /**
     * @brief: 将添加定时器的任务转移到定时器队列所属的线程中执行
     * @return {*}
     * @param {Timer*} timer 需要添加的定时器
     */    
    void addTimerInLoop(Timer* timer);

    /**
     * @brief: 将取消定时器的任务转移到定时器队列所属的线程中执行   
     * @return {*}
     * @param {TimerID&} id
     */    
    void cancelTimerInLoop(TimerID id);

    /**
     * @brief: 超时处理函数
     * @return {*}
     * @param {Timestamp} happendTime 发生超时的时刻
     */    
    void handleExpiration(Timestamp happendTime);

    /**
     * @brief: 将定时器添加到队列中
     * @return {*} 若待添加定时器早于队列中最早的定时器，则返回true；否则返回false
     * @param {Timer} *timer 待添加的队列
     */    
    bool addIntoQueue(Timer *timer);

    /**
     * @brief: 重新设置TimerQueue的超时时间
     * @return {*}
     * @param {Timestamp} expiration 新的超时时间
     */    
    void resetExpiration(Timestamp expiration);

    /**
     * @brief: 获取超时定时器
     * @return {*}
     * @param {Timestamp&} now 当前时间点的时间戳
     */    
    std::vector<Entry> getExpired(Timestamp& now);

    /**
     * @brief: 重启需要重复启动的定时器
     * @return {*}
     * @param {Timestamp&} now 当前时间点时间戳
     */    
    void restartTimers(std::vector<Entry>& expired, Timestamp& now);

    /**
     * @brief: 读timerfd，避免重复触发
     * 
     * @return {*}
     */    
    void readTimerfd();
};

#endif