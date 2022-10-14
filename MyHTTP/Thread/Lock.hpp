/* 封装 pthread 中的线程同步机制，使其满足RAII原则 */
#ifndef LOCK_H
#define LOCK_H
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

// mutex 封装
class Mutex{
private:
    pthread_mutex_t _mutex;

public:
    Mutex(pthread_mutexattr_t* attr = nullptr)
    {
        pthread_mutex_init(&_mutex, attr);
    }

    ~Mutex()
    {
        pthread_mutex_destroy(&_mutex);
    }

    /**
     * @brief:阻塞式加锁 
     * @return {*int} 成功则返回0；失败则返回对应的errno
     */    
    int lock()
    {
        return pthread_mutex_lock(&_mutex);
    }

    /**
     * @brief: 非阻塞式加锁 
     * @return {*int} 成功则返回0；失败则返回对应的errno
     */    
    int trylock()
    {
        return pthread_mutex_trylock(&_mutex);
    }

    /**
     * @brief: 解锁
     * @return {*int} 成功则返回0；失败则返回对应的errno
     */    
    int unlock()
    {
        return pthread_mutex_unlock(&_mutex);
    }

    pthread_mutex_t* get()
    {
        return &_mutex;
    }
};

// semaphore 封装
class Semaphore{
private:
    sem_t _sem;

public:
    /**
     * @brief: Semaphore 构造
     * @return {*}
     * @param {int} pshared 指示该 semaphore 是线程间共享还是进程间共享;
     *              pshared == 0：该 semaphore 为线程间共享
     *              pshared != 0：该 semaphore 为进程间共享
     * @param {unsigned int} value semaphore 的初始值
     */
    Semaphore(int pshared, unsigned int value)
    {
        sem_init(&_sem, pshared, value);
    }

    ~Semaphore()
    {
        sem_destroy(&_sem);
    }

    /**
     * @brief: 递增(解锁) semaphore
     * @return {*int} 成功则返回0；失败则返回-1，并设置errno
     */    
    int post()
    {
        return sem_post(&_sem);
    }

    /**
     * @brief: 阻塞式 递减(加锁) semaphore
     * @return {*int}成功则返回0；失败则返回-1，并设置errno
     */    
    int wait()
    {
        return sem_wait(&_sem);
    }

    /**
     * @brief: 非阻塞式 递减(加锁) semaphore
     * @return {*int}成功则返回0；semaphore == 0 时返回EAGAIN；失败返回-1，并设置errno
     */    
    int trywait()
    {
        return sem_trywait(&_sem);
    }

    /**
     * @brief: 等待指定时间的 递减(加锁) semaphore
     * @return {*int} 成功则返回0；超时设置errno = ETIMEDOUT；失败则返回-1，并设置errno
     * @param {unsigned int} s 超时时限，单位为秒(s)
     */
    int timedwait(unsigned int seconds)
    {
        timespec t;
        clock_gettime(CLOCK_REALTIME, &t);
        t.tv_sec += seconds;

        return sem_timedwait(&_sem, &t);
    }

    /**
     * @brief: 获取阻塞在 wait/timedwait 上的线程/进程数 
     * @return {*}成功返回对应的值；失败则返回-1，并设置对应的errno
     */    
    int getvalue()
    {
        int val = 0;
        int ret = sem_getvalue(&_sem, &val);
        
        if(ret != 0) return -1;
        else return -val;
    }
};


// conditon variable 封装
class Condition{
private:
    pthread_cond_t _cond;

public:
    Condition(pthread_condattr_t* attr = nullptr)
    {
        pthread_cond_init(&_cond, attr);
    }

    ~Condition()
    {
        pthread_cond_destroy(&_cond);
    }

    /**
     * @brief: 唤醒1个等待条件变量的线程
     * @return {*}成功则返回0；失败则返回对应的errno
     */    
    int signal()
    {
        return pthread_cond_signal(&_cond);
    }

    /**
     * @brief: 唤醒等待条件变量的所有线程
     * @return {*} 成功则返回0；失败则返回对应的errno
     */    
    int broadcast()
    {
        return pthread_cond_broadcast(&_cond);
    }

    /**
     * @brief: 阻塞式等待条件变量
     * @return {*}成功则返回0；失败则返对应的errno
     * @param {Mutex&} m 与条件变量关联的Mutex
     */    
    int wait(Mutex& m)
    {
        return pthread_cond_wait(&_cond, m.get());
    }

    /**
     * @brief: 阻塞等待条件变量一定时间
     * @return {*}成功则返回0；失败则返回对应的errno
     * @param {Mutex&} m 与条件变量关联的Mutex
     * @param {unsigned int} seconds 指定的等待时间，单位为秒(s)
     */    
    int timedwait(Mutex& m, unsigned int seconds)
    {
        timespec t;
        clock_gettime(CLOCK_REALTIME, &t);
        t.tv_sec += seconds;

        return pthread_cond_timedwait(&_cond, m.get(), &t);
    }



/***** 禁用拷贝构造 *****/
private:
Condition(const Condition&);
Condition &operator=(const Condition &);
/***** 禁用拷贝构造 *****/
/*
* 只用pthread_cond_t* cond自身能够发挥同步作用，使用cond的副本进行signal、wait等操作
* 其行为是未定义的。
* POSIX Programmer's Manual
* Only  cond itself may be used for performing synchronization. The result of referring to
* copies   of   cond   in   calls   to   pthread_cond_wait(),    pthread_cond_timedwait(),
* pthread_cond_signal(),  pthread_cond_broadcast(),  and  pthread_cond_destroy()  is unde‐
* fined.
*/
};

#endif