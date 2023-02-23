# 基于`MyNet`的HTTP封装——`MyHTTP`

## 一、功能

封装`MyNet`中的`TCPServer`和`TCPConnection`，由此更上层提供更加简单的`start`和请求处理方法接口，使得项目能够个性化设计用户自己想要实现的HTTP服务器



## 二、实现

- `HTTPServer`——封装`TCPServer`
  - 该封装隐藏了HTTP连接创建、移除等过程，只提供上层用户需要的一些接口
  - 提供`setOnGet`等接口，用户能够通过这些接口自定义HTTP请求的处理方法
  - 提供`createAndStart`接口，使得上层用户能够简单地通过该接口创建并启动一个服务器
- `HTTPConnection`——封装`TCPConnection`
  - 该封装隐藏了HTTP报文的读取、解析、发送过程，使得上层用户在得到`HTTPConnection`时只能看到对应的解析完成的`HTTPRequest`对象，直接进行操作



## 三、有关`WokerPool`

HTTP报文解析需要`WorkerPool`中的`Woker`线程进行处理，本项目中实现了一个基于放入不阻塞、取出阻塞的半阻塞任务队列的线程池来存放`Woker`线程和对应的处理任务。
有关Woker解析HTTP请求报文的流程详见：[HTTP报文解析](https://github.com/Phos35/MyTinyWebServer/blob/master/Documents/MyHTTP/HTTPRequestParser.md)

### 1. 线程池一般模型

![](http://43.138.43.178:8000/image/ThreadPool.drawio.png)

- 由模型图可以看出该模型属于于经典的线程同步模型——**生产者消费者模型** 

- 有关于线程同步
  - 可以使用 [TinyWebserver](https://github.com/qinguoyi/TinyWebServer) 在工作线程、生产者线程访问任务队列时显式地使用同步技术（semaphore 或者 conditional variable）。但这种方法会显得代码十分冗杂
    - 取出任务的场景 —— 工作线程中取出队首任务部分
      ![](http://43.138.43.178:8000/image/obsolete_pop.png)
    - 放入任务的场景 —— `ThreadPool.push()`
      ![](http://43.138.43.178:8000/image/obsolete_push.png)

  - 可以使用多线程同步中常用的数据结构——**阻塞队列**。该结构将同步机制封装在类内，使得结构本身成为线程安全类型，同时也简化了取出任务和放入任务的接口
    - 取出任务的场景 —— 工作线程中取出队首任务部分
      ![](http://43.138.43.178:8000/image/new_pop.png)
    - 放入任务的场景 —— `ThreadPool.push()`
      ![](http://43.138.43.178:8000/image/new_push.png)



### 2.使用阻塞队列实现任务队列

- 前提

  - 依据 `RAII` 原则，首先需要对 `pthread` 下的一些基本同步机制进行封装，包括 `pthread_mutex`, `sem`, `pthread_cond`
  - 封装重点在于实现构造函数中 `init` (`pthread_mutex_init`, `pthread_cond_init`)， 析构函数中 `destroy`(`pthread_mutex_destroy`, `pthread_cond_destroy`)，其余各个接口可以参考 `Manual`选择所需的部分进行实现

- 成员变量

  - ```c++
    std::queue<T> que;      // 资源队列
    Mutex qMutex;           // 队列访问互斥锁
    Condition qCond;        // 队列条件变量
    
    int capacity;           // 队列最大容量
    ```

  

- 接口

  - ```c++
    /**
     * @brief: 非阻塞式添加元素 
     * @return {*} 成功则返回0；失败则返回-1，标志队列已满
     * @param {T*} item 待放入队列中的元素
     */    
    int push(T item);
    
    /**
     * @brief: 阻塞式获取元素
     * @return {*} 返回队列中成功获取到的元素
     */    
    T pop();
    
    /**
     * @brief: 判断队列是否已满
     * @return {*} 队列已满则返回true，未满则返回false
     */    
    bool full();
    
    /**
     * @brief: 判断队列是否已空
     * @return {*} 队列已空则返回true，未空则返回false
     */    
    bool empty();
    ```

  

- 线程同步的核心

  - `pop` —— 阻塞

    - 实现

      ```c++
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
      ```

      

    - 原理
      利用条件变量 `qCond` 的 `wait` 操作实现阻塞式等待，直至队列中有任务被放入

  - `push` —— 非阻塞

    - 实现

      ```c++
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
      
      ```

      

    - 非阻塞的原因
      传统阻塞队列在 **"队列空时取出任务"（情景A）**、**“队列满时放入任务”（情景B）** 两种情景下均为阻塞，但本项目中实现的阻塞队列在情景B中使用了队列满时直接返回的形式，其原因为：
      HTTP请求应当尽可能快地做出响应，避免用户长时间等待响应。因此在队列满时采取了立即返回（队列已满，返回客户端的响应为：**服务器繁忙**）的形式（`TODO` 待完善繁忙响应）

