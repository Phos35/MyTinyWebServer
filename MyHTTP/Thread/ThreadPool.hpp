#ifndef THREADPOOL_H
#define THREADPOOL_H
#include <functional>
#include "BlockQue.hpp"
#include <atomic>

// 处理线程池 模板类
template <typename T>
class ThreadPool{
private:
    typedef std::function<void(const T&)> WorkerCallBack;

    BlockQueue<T> que;      // 任务队列

    int THREAD_NUM;         // 线程池线程数量
    int QUE_CAPACITY;       // 请求队列容量

    WorkerCallBack worker;  // 处理回调函数

    /**
     * @brief: 线程池中各线程的启动函数，用于调用线程工作函数
     * @return {*}
     * @param {void*} arg 函数参数，实际传入ThreadPool的this指针
     */    
    static void* start(void* arg)
    {
        ThreadPool *pool = (ThreadPool *)arg;

        // 启动线程工作函数
        pool->work();
        return nullptr;
    }

    /**
     * @brief: 线程工作函数，从线程池中获取指定的请求/对象，并调用对应的处理函数
     * @return {*}
     */    
    void work()
    {
        while(true)
        {
            // 从队列中取出任务
            T task = que.pop();

            // 处理
            if(worker)
            {
                worker(task);
            }
        }
    }

public:
    ThreadPool(int threadNum, int queCapacity)
    :THREAD_NUM(threadNum), que(queCapacity)
    {
        // 创建指定数量的线程并设置detach
        pthread_t threadId;
        int ret = 0;
        for (int i = 0; i < THREAD_NUM; i++)
        {
            ret = pthread_create(&threadId, nullptr, start, (void *)this);
            if(ret != 0)
            {
                perror("Thread Create");
                exit(-1);
            }
            pthread_detach(threadId);
        }

    }

    /**
     * @brief: 向请求队列中追加请求
     * @return {*}成功则返回0；失败则返回-1，表示队列已满
     */    
    int push(T request)
    {
        return que.push(request);
    }

    /**
     * @brief: 设置处理回调函数
     * @return {*}
     * @param {WorkerCallBack} cb
     */    
    void setWorker(WorkerCallBack cb)
    {
        worker = cb;
    }
};

#endif