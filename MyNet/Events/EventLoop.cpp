#include "EventLoop.h"
#include "AsyncLog.h"
#include <sys/eventfd.h>
#include <unistd.h>
#include <assert.h>

__thread EventLoop *loopOfThisThread = nullptr;

EventLoop::EventLoop()
:threadID_(pthread_self()),
 running_(false), quit_(false), eventHandling_(false), tasksHandling_(false),
 epoller_(this), timerQue_(this),
 wakefd_(eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK)),
 wakeChannel_(wakefd_, this)
{
    // 事件循环已为其它线程所有，不能赋予当前线程
    if(loopOfThisThread != nullptr)
    {
        printf("Thread %ld has its loop!", pthread_self());
        exit(-1);
    }
    // 当前线程不拥有事件循环，则分配当前事件循环
    else
    {
        loopOfThisThread = this;
    }

    // wakefd启动读事件，便于唤醒事件循环
    wakeChannel_.setRDCallBack(std::bind(&EventLoop::readWakefd, this, std::placeholders::_1));
    wakeChannel_.focusRead();
}

/**
 * @brief: 判断当前线程是否为EventLoop所属的线程
 * @return {*}
 */    
bool EventLoop::isInLoopThread()
{
    return loopOfThisThread == this;
}

/**
 * @brief: 判断当前线程是否为EvenLoop所属的线程
 * @return {*}
 */    
void EventLoop::assertInLoopThread()
{
    pthread_t currentID = pthread_self();
    if (currentID != threadID_)
    {
        printf("AssertInLoopThread. Current Thread %ld, Loop Thread %ld\n", pthread_self(), threadID_);
        abort();
    }
}

/**
 * @brief: 启动事件循环
 * @return {*}
 */    
void EventLoop::run()
{
    assert(running_ == false);
    running_ = true;

    while(quit_ == false)
    {
        activeChannels_.clear();
        int nr = epoller_.poll(activeChannels_, -1, eventsHappenTime_);

        // 事件发生，处理事件
        eventHandling_ = true;
        if (activeChannels_.empty() == false)
        {
            for (int i = 0; i < nr; i++)
            {
                activeChannels_[i]->handleEvents(eventsHappenTime_);
            }
        }
        eventHandling_ = false;

        // 处理其他线程转移过来的任务
        handleTask();
    }
}

/**
 * @brief: 在EPoller中更新Channel
 * @return {*}
 * @param {Channel} *channel 待更新的Channel
 */    
void EventLoop::updateChannel(Channel *channel)
{
    epoller_.updateChannel(channel);
}

/**
 * @brief: 在EPoller中移除Channel
 * @return {*}
 * @param {Channel} *channel 待移除的Channel
 */    
void EventLoop::removeChannel(Channel *channel)
{
    epoller_.removeChannel(channel);
}

/**
 * @brief: 在本线程执行任务
 * @return {*}
 * @param {Task} & 待处理的任务
 */    
void EventLoop::runInLoop(Task &&task)
{
    // 若当前处于事件循环所属的线程，则直接执行
    if(isInLoopThread())
    {
        task();
    }
    // 否则添加到任务列表中并唤醒事件循环
    else
    {
        addTask(std::move(task));
    }
}

/**
 * @brief: 将任务添加到tasks中并唤醒事件循环
 * @return {*}
 * @param {Task} & 待添加的task
 */    
void EventLoop::addTask(Task &&task)
{
    // 加锁添加task
    {
        std::lock_guard<std::mutex> lck(mutex_);
        tasks_.push_back(task);
    }

    // 唤醒事件循环
    if(isInLoopThread() == false || taskHandling() == true)
    {
        wakeup();
    }
}

/**
 * @brief: 处理其它线程转移过来的任务
 * @return {*}
 */    
void EventLoop::handleTask()
{
    tasksHandling_ = true;

    // 加锁获取目前的任务列表
    TaskList tmpList;
    {
        std::lock_guard<std::mutex> lck(mutex_);
        tmpList.swap(tasks_);
    }

    // 处理任务
    for (auto &task : tmpList)
    {
        task();
    }

    tasksHandling_ = false;
}

/**
 * @brief: wakefd唤醒后的读回调函数
 * @return {*}
 */    
void EventLoop::readWakefd(Timestamp time)
{
    uint64_t data = 1;
    ssize_t size = read(wakefd_, &data, sizeof(data));
    if(size != sizeof(data))
    {
        LOG_ERROR << "EventLoop Wakeupfd read " << size << " Bytes";
        perror("READ WAKEFD");
    }
}

/**
 * @brief: 唤醒当前事件循环
 * @return {*}
 */    
void EventLoop::wakeup()
{
    uint64_t data = 1;
    ssize_t size = write(wakefd_, &data, sizeof(data));
    if(size != sizeof(data))
    {
        LOG_ERROR << "EventLoop Wakeup Writes " << size << "Bytes";
    }
}

/**
 * @brief: 添加一个只触发一次的定时器
 * @return {*}
 * @param {CallBack} & 定时器触发后的回调函数
 * @param {Timestamp} expiration 定时器启动时间点
 */
TimerID EventLoop::runAt(Timer::CallBack &&cb, Timestamp expiration)
{
    return timerQue_.addTimer(expiration, 0, std::move(cb));
}

/**
 * @brief: 添加一个重复触发的定时器
 * @return {*}
 * @param {CallBack} & 定时器触发后的回调函数
 * @param {Timestamp} expiration 第一次触发的时间点
 * @param {uint64_t} interval 重复触发的时间间隔，单位为微妙
 */
TimerID EventLoop::runEvery(Timer::CallBack &&cb, Timestamp expiration, uint64_t interval)
{
    return timerQue_.addTimer(expiration, interval, std::move(cb));
}

/**
 * @brief: 取消定时器
 * @return {*}
 * @param {TimerID} id 定时器ID
 */
void EventLoop::cancelTimer(TimerID id)
{
    timerQue_.cancelTimer(id);
}

/**
 * @brief: 退出事件循环
 * @return {*}
 */    
void EventLoop::quit()
{
    // 放入tasks中，可以是事件循环执行完一次完整的流程后再退出
    runInLoop(std::bind(&EventLoop::quitInLoop, this));
}

/**
 * @brief: 设置quit_ == true，使事件循环退出
 * @return {*}
 */    
void EventLoop::quitInLoop()
{
    quit_ = true;
}

EventLoop::~EventLoop()
{
    quit_ = false;
}