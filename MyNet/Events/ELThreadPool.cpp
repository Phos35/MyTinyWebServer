#include "ELThreadPool.h"
#include "AsyncLog.h"

int ELThreadPool::DEFAULT_SIZE = 5;

/**
 * @brief: 创建事件循环线程池
 * @return {*}
 * @param {EventLoop*} ownerLoop 线程池所属的时间循环
 * @param {int} size 线程池大小
 */
ELThreadPool::ELThreadPool(EventLoop *ownerLoop, int size)
: ownerLoop_(ownerLoop), size_(size), nextIndex_(0)
{
}

/**
 * @brief: 创建线程池内的事件循环线程
 * @return {*}
 */    
void ELThreadPool::start()
{
    for (int i = 0; i < size_; i++)
    {
        UELTPtr th(new ELThread());
        ELPool_.push_back(th->startLoop());
        THPool_.push_back(std::move(th));
    }

    for (int i = 0; i < size_; i++)
    {
        LOG_TRACE << "PoolLoop " << i << ", wakefd " << ELPool_[i]->wakefd();
    }
}