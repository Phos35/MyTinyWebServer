#ifndef EVENT_LOOP_THREAD_POOL_H
#define EVENT_LOOP_THREAD_POOL_H
#include "ELThread.h"
#include <vector>
#include <memory>

class ELThreadPool{
public:
    static int DEFAULT_SIZE;
    typedef EventLoop *ELPtr;
    typedef std::unique_ptr<ELThread> UELTPtr;

    /**
     * @brief: 创建事件循环线程池
     * @return {*}
     * @param {EventLoop*} ownerLoop 线程池所属的时间循环
     * @param {int} size 线程池大小
     */    
    ELThreadPool(EventLoop* ownerLoop, int size = DEFAULT_SIZE);

    /**
     * @brief: 创建线程池内的事件循环线程
     * @return {*}
     */    
    void start();

    /*成员/属性get函数*/
    int size() { return size_; }
    EventLoop *next() { return ELPool_[(nextIndex_++ % size_)]; }

private:
    typedef std::vector<ELPtr> ELPool;
    typedef std::vector<UELTPtr> THPool;

    EventLoop *ownerLoop_;      // 线程池所属的事件循环
    ELPool ELPool_;             // 存储事件循环的vector
    THPool THPool_;             // 存储事件循环对应线程的vector   
    int size_;                  // 线程池大小

    int nextIndex_;             // 下一个要取的事件循环的下标
};

#endif