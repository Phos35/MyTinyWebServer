#include "Timer.h"
#include "AsyncLog.h"

Timer::AtomicUInt Timer::globalID(0);

Timer::Timer(Timestamp expiration, uint64_t interval, 
             CallBack&& expiredCallBack)
    :expiration_(expiration), interval_(interval), expiredCallBack_(std::move(expiredCallBack))
{
    id_ = getNewID();
}

/**
 * @brief: 调用超时处理回调函数，完成定时任务
 * @return {*}
 */    
void Timer::handle()
{
    if(expiredCallBack_)
        expiredCallBack_();
    else
        LOG_ERROR << "Timer " << (int)id_ << "ExpireCallBack Not Set";
}

/**
 * @brief: 对于需要重复启动的定时器，重新启动定时
 * @return {*}
 * @param {Timestamp} timestamp
 */    
void Timer::restart(Timestamp timestamp)
{
    timestamp.addMicrosec(interval_);
    expiration_ = timestamp;
}

/**
 * @brief: 获取可用ID，全局ID回滚式自增1
 * @return {uint32_t} 可用ID
 */    
uint32_t Timer::getNewID()
{
    return globalID.fetch_add(1);
}