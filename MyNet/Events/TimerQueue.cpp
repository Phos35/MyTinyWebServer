#include "TimerQueue.h"
#include "AsyncLog.h"
#include "EventLoop.h"
#include <sys/timerfd.h>
#include <assert.h>
#include <unistd.h>
#include <algorithm>
#include <iterator>

TimerQueue::TimerQueue(EventLoop *loop)
:loop_(loop), timerfd_(timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC)),
 timerChannel_(timerfd_, loop_),
 handlingExpire_(false)
{
    if(timerfd_ == -1)
    {
        perror("TimerQueue timerfd Create Failed");
        exit(-1);
    }

    // 启动timer的读事件，并绑定超时处理函数
    timerChannel_.setRDCallBack(std::bind(&TimerQueue::handleExpiration, this, std::placeholders::_1));
    timerChannel_.focusRead();
}

/**
 * @brief: 添加定时器
 * @return {*}
 * @param {Timestamp} expiration 定时器超时时间点
 * @param {uint64_t} interval 定时器重复触发间隔。=0表示只触发一次
 * @param {CallBack&&} callback 定时器到期后的处理回调函数
 */    
TimerID TimerQueue::addTimer(Timestamp expiration, uint64_t interval,
                             Timer::CallBack&& callback)
{
    Timer* newTimer = new Timer(expiration, interval, std::move(callback));

    if(loop_->isInLoopThread())
    {
        addTimerInLoop(newTimer);
    }
    // 转移至TimerQueue所属的线程添加
    else
    {
        loop_->runInLoop(std::bind(&TimerQueue::addTimerInLoop, this, newTimer));
    }

    return TimerID(newTimer->id(), newTimer);
}

/**
 * @brief: 取消指定定时器
 * @return {*}
 * @param {TimerID&} id 带取消定时器的id
 */    
void TimerQueue::cancelTimer(TimerID id)
{
    if(loop_->isInLoopThread())
    {
        cancelTimerInLoop(id);
    }
    else
    {
        loop_->runInLoop(std::bind(&TimerQueue::cancelTimerInLoop, this, id));
    }
}

/**
 * @brief: 将添加定时器的任务转移到定时器队列所属的线程中执行
 * @return {*}
 * @param {Timer*} timer 待添加的定时器
 */    
void TimerQueue::addTimerInLoop(Timer* timer)
{
    LOG_TRACE << "ADD Timer: ( " << static_cast<int>(timer->id()) << ", " 
              << timer->expiration().format("%y-%m-%d %H-%M-%S.%s") << ")";

    // 断言必然在TimerQueue所属的事件循环中
    loop_->assertInLoopThread();

    // 将定时器添加到队列中
    bool isEarliest = addIntoQueue(timer);

    // 若新添加的定时器比原先的超时时间更早，则更新超时时间
    if(isEarliest)
    {
        resetExpiration(timer->expiration());
    }
}

/**
 * @brief: 将取消定时器的任务转移到定时器队列所属的线程中执行   
 * @return {*}
 * @param {TimerID&} id
 */    
void TimerQueue::cancelTimerInLoop(TimerID id)
{
    // 断言必然在TimerQueue所属的事件循环中
    loop_->assertInLoopThread();

    Timer *timer = id.timer();
    auto itr = timers_.find({timer->expiration(), timer});
    // 若定时器在队列中，则进行删除
    if(itr != timers_.end())
    {
        LOG_TRACE << "Cancel Timer: " << id.id();
        int nr = timers_.erase({timer->expiration(), timer});
        assert(nr == 1);

        // 释放资源 TODO 使用智能指针管理Timer可能会更好
        delete timer;
    }
    // 若定时器不在队列中，则
    // 若正在进行超时事件的处理，则为超时回调中重复取消定时器，
    // 加入到cancelled队列中做标记
    else if(handlingExpire_)
    {
        cancelled_.insert(timer);
    }
    // 若未在进行超时事件处理，则为主动的重复取消定时器，不处理即可
}

/**
 * @brief: 将定时器添加到队列中
 * @return {*} 若待添加定时器早于队列中最早的定时器，则返回true；否则返回false
 * @param {Timer} *timer 待添加的队列
 */    
bool TimerQueue::addIntoQueue(Timer *timer)
{
    // 获取最早超时的定时器
    auto firstTimer = timers_.begin();

    // 添加新的定时器
    timers_.insert({timer->expiration(), timer});

    // 若队列中原先无定时器 或 新定时器超时时间早于原先最早超时的定时器，则需要更新超时时间
    if(firstTimer == timers_.end() || timer->expiration() < firstTimer->first)
    {
        return true;
    }
    else
    {
        return false;
    }
}

/**
 * @brief: 重新设置TimerQueue的超时时间
 * @return {*}
 * @param {Timestamp} expiration 新的超时时间
 */    
void TimerQueue::resetExpiration(Timestamp expiration)
{
    uint64_t microsecs = expiration.microseconds() - Timestamp::now().microseconds();
    time_t seconds = microsecs / Timestamp::MICRO_PER_SECOND;
    time_t nanosecs = microsecs % Timestamp::MICRO_PER_SECOND * 1000;
    itimerspec ex = {
        .it_interval = {.tv_sec = 0, .tv_nsec = 0},
        .it_value = {.tv_sec = seconds, .tv_nsec = nanosecs}
    };

    // TODO 这里采用绝对时间，而muduo采用相对时间，可能出错
    int ret = timerfd_settime(timerfd_, 0, &ex, nullptr);
    if(ret == -1)
    {
        perror("TimerReset Failed\n");
        exit(-1);
    }

    LOG_TRACE << "Update ExpireTime: " << expiration.format("%y-%m-%d %H-%M-%S.%s");
}

/**
 * @brief: 超时处理函数
 * @return {*}
 * @param {Timestamp} happendTime 发生超时的时刻
 */    
void TimerQueue::handleExpiration(Timestamp happendTime)
{   
    // 读取timerfd，避免重复触发
    readTimerfd();

    // 读取已超时的定时器
    Timestamp now = Timestamp::now();
    std::vector<Entry> expired = getExpired(now);

    // 开始处理定时器超时事件
    handlingExpire_ = true;

    // 清空重复取消定时器标记
    cancelled_.clear();

    // 调用回调函数进行处理
    for(auto& e : expired)
    {
        LOG_TRACE << "Timer " << e.second->id() <<" TimeOut at " << happendTime.format("%y-%m-%d %H:%M:%S.%s");
        e.second->handle();
    }

    handlingExpire_ = false;

    // 重启需要重复启动的定时器
    restartTimers(expired, now);
}

/**
 * @brief: 获取超时定时器
 * @return {*}
 * @param {Timestamp&} now 当前时间点的时间戳
 */    
std::vector<TimerQueue::Entry> TimerQueue::getExpired(Timestamp& now)
{
    std::vector<Entry> expired;

    // 使用UINTPTR_MAX，保证能获取到expiration == now的所有定时器
    Entry boundE(now, (Timer*)UINTPTR_MAX);

    // end 为第一个未超时的Entry的迭代器
    auto end = timers_.lower_bound(boundE);
    // 断言要么不存在超时的定时器，一旦存在，则end的时间点一定比现在靠后
    assert(end == timers_.end() || now < end->first);

    // 拷贝超时定时器到结果集中，并从timers_中移除
    std::copy(timers_.begin(), end, back_inserter(expired));
    timers_.erase(timers_.begin(), end);

    return expired;
}

/**
 * @brief: 重启需要重复启动的定时器
 * @return {*}
 * @param {Timestamp&} now 当前时间点时间戳
 */    
void TimerQueue::restartTimers(std::vector<Entry>& expired, Timestamp& now)
{
    for(auto& e : expired)
    {
        // 若定时器设置为重复启动，且未被取消，则重启
        if(e.second->repeat() && cancelled_.find(e.second) == cancelled_.end())
        {
            e.second->restart(now);
            timers_.insert(e);
        }
        else
        {
            // TODO 使用智能指针管理Timer
            delete e.second;
        }
    }

    // 若timers_不为空，则设置新的超时时间
    if(timers_.empty() == false)
    {
        resetExpiration(timers_.begin()->second->expiration());
    }
}

/**
 * @brief: 读timerfd，避免重复触发
 * 
 * @return {*}
 */    
void TimerQueue::readTimerfd()
{
    uint64_t num;
    int size = read(timerfd_, &num, sizeof(num));
    if(size != sizeof(num))
    {
        printf("TimerQueue timerfd Reads %d bytes\n", size);
        perror("READ TIEMRFD");
    }
}