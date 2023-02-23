#ifndef BLOCKQUE_H
#define BLOCKQUE_H
#include <queue>
#include "Lock.hpp"

// 阻塞队列 模板类
template <typename T>
class BlockQueue{
private:
    std::queue<T> que;      // 资源队列
    Mutex qMutex;           // 队列访问互斥锁
    Condition qCond;        // 队列条件变量

    int capacity;           // 队列最大容量

public:
    BlockQueue(int capacity = 0)
    :capacity(capacity)
    {}

    /**
     * @brief: 非阻塞式添加元素 
     * @return {*} 成功则返回0；失败则返回-1，标志队列已满
     * @param {T*} item 待放入队列中的元素
     */    
    int push(T item)
    {
        qMutex.lock();
        
        // 若队列已满，则返回-1
        if(full() == true)
        {
            qMutex.unlock();
            return -1;
        }

        // 队列未满，则放入元素
        que.push(item);
        qCond.signal();

        qMutex.unlock();

        return 0;
    }

    /**
     * @brief: 阻塞式获取元素
     * @return {*} 返回队列中成功获取到的元素
     */    
    T pop()
    {
        qMutex.lock();

        // 队列为空则等待
        while(empty() == true)
        {
            qCond.wait(qMutex);
        }
        
        // 获取元素
        T ret = que.front();
        que.pop();

        qMutex.unlock();

        return ret;
    }

    /**
     * @brief: 判断队列是否已满
     * @return {*} 队列已满则返回true，未满则返回false
     */    
    bool full()
    {
        return que.size() == capacity;
    }

    /**
     * @brief: 判断队列是否已空
     * @return {*} 队列已空则返回true，未空则返回false
     */    
    bool empty()
    {
        return que.size() == 0;
    }

    /**
     * @brief: 更改队列容量大小
     * @return {*}
     * @param {int} newCapacity
     */    
    void reserve(int newCapacity)
    {
        capacity = newCapacity;
    }
};

#endif