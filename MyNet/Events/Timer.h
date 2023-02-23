#ifndef TIMER_H
#define TIMER_H
#include <stdint.h>
#include <functional>
#include <Timestamp.h>
#include <atomic>

class Timer{
public:
    typedef std::function<void()> CallBack;

    Timer(Timestamp expiration, uint64_t interval, CallBack&& expiredCallBack);

    /**
     * @brief: 调用超时处理回调函数，完成定时任务
     * @return {*}
     */    
    void handle();

    /**
     * @brief: 对于需要重复启动的定时器，重新启动定时
     * @return {*}
     * @param {Timestamp} timestamp
     */    
    void restart(Timestamp timestamp);

    /*成员get函数*/
    uint32_t id() { return id_; }
    Timestamp expiration() { return expiration_; }
    uint64_t interval() { return interval_; }
    bool repeat() { return interval_ > 0; }

private:
    typedef std::atomic<uint32_t> AtomicUInt;
    uint32_t id_;                   // timerID
    Timestamp expiration_;          // 超时时间点
    uint64_t interval_;             // 重复启动的间隔
 
    CallBack expiredCallBack_;      // 超时处理回调函数

    static AtomicUInt globalID;     // 全局定时器可用的最小ID

    /**
     * @brief: 获取可用ID，全局ID回滚式自增1
     * @return {uint32_t} 可用ID
     */    
    uint32_t getNewID();
};

#endif