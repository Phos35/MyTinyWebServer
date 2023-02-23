# 简易TCP网络库——`MyNet`

## 一、网络编程模型

`MyNet`使用的是主从Reactor模型

### 1. 模型图

![](http://43.138.43.178:8000/image/MSReactor-Frame.drawio.png)

### 2. 模型解析

- `MainReactor`
  - `MainReactor`只负责创建新的连接并将其分配给从`SubReactorPool`中取出的`SubReactor`。`MainReactor`不负责连接的IO事件
- `SubReactor` & `SubReactorPool`
  - `SubReactor`负责连接的IO事件，即负责读取、发送数据（read、write），但其不负责逻辑处理（如报文解析、获取资源等）。`SubReactor`通过`TCPConnection`中的各类回调函数完成读写、分配逻辑处理任务给`Woker`等动作
  - `SubReactorPool`，使用池的思想预先创建固定数量的`SubReactor`，避免为每个连接都动态创建`SubReactor`而造成的资源和事件开销
- Worker & `WorkerPool`
  - Worker负责连接数据的逻辑处理，包括报文解析、资源获取、构建响应数据等。Worker从SubReactor获取数据，将构建的响应递交给`SubReactor`处理
  - `WorkerPool`，如同`SubReactorPool`，是为了避免动态创建销毁的开销而使用池的设计
- Resources
  - 位于硬盘上的资源，文件、数据库等
- Log
  - 异步日志系统，能够在尽量减少对系统正常运行的影响下输出足够多的系统运行状态信息，保证故障发生时能够提供有效的错误检测信息



## 二、核心结构

针对`MyNet`核心的网络功能，先给出一个整体的结构图，然后自下而上逐层解析。

### 1. 结构图

![](http://43.138.43.178:8000/image/MyNetFrame.png)


### 2.`EventLoop`

- `EPoller`

  - 是对`IO Mutiplexing`机制中`epoll`的封装，可以监听指定`fd`关注的事件，事件发生时将发生的事件返回给上层
  - `EPoller`不直接管理`Channel`是为了专一自己监听`fd`的职责。`EPoller`只负责将发生事件的Channel传递给`EventLoop`，而Channel上发生的事件处理则移交给`EventLoop`去负责。

- `Channel`

  - 是对`fd`的封装，但`Channel`并不拥有`fd`。
  - `Channel`的职责是管理`fd`，包括注册、修改、删除`fd`上关注的事件，另外还是`fd`上活跃事件的处理入口

- `EventLoop`

  - 事件循环，遵循`One Loop Per Thread`思想：每个事件循环唯一归属于一个线程，一个线程唯一拥有一个事件循环

  - `EventLoop`的执行流程十分简单：

    ```cpp
    while(!quit)
    {
    	// 获取发生事件的Channel
        activeChannels = EPoller.poll();
        
        // 处理事件
        for(channel : activeChannels)
        {
            channel.handleEvents();
        }
        
        // 处理跨线程任务调用
        handleTasks();
    }
    ```

  - `EventLoop`主要负责使用`EPoller`监听`fd`上发生的事件来获取发生事件的Channel，进而调用`Channel`上的事件处理函数来处理对应事件。

  - 注意`EventLoop`执行流程中的14行的处理跨线程任务，现在来看为什么需要这一操作

    - `EventLoop`处理IO事件均在事件循环所属线程中完成，这一操作尽可能地减少了多线程同步控制
    - 但是从`EventLoop`所属线程外发起一些可能改变`EventLoop`内变量的操作是存在的，如`TCPServer`创建新连接需要为新连接分配`EventLoopThread`中的一个事件循环，此时`TCPServer`与新连接处于不同的事件循环中，在`TCPServer`中设置新连接的一些属性（如添加/修改关注的事件、添加对应的定时器等）就会造成跨线程调用，这一操作就需要多线程同步
    - 为了避免上述多线程同步控制的问题，`EventLoop`中设置了`TaskQueue`（类型为function<void()>，能够存储函数）来存储跨线程的调用，这样就只需要在将跨线程调用添加至`TaskQueue`和处理`TaskQueue`时进行同步控制，相比于对整个跨线程调用进行同步要简单得多

  - 关于`EventLoop`的定时器队列 -- `TimerQueue`

    - 在结构图中并未看到相关的结构，这是因为本项目中的定时器队列使用`timerfd`事件驱动，其触发原理与普通`fd`触发`EventLoop`处理读事件的流程一致。可以认为`TimerQueue`本质上就是一个会定时触发读事件的`Channel`
    - `TimerQueue`中存储`Timer`使用`std::set`，其查找、添加、删除的效率为`O(logn)`，是比较高效的结构。



### 3. `TCPConnection`

- `Channel`
  - 管理`accept`得到的`conn_fd`，包括连接上的事件触发、读写操作（实质上是调用上层结构传递进来的回调函数）等
- `Buffer`
  - 应用层的缓冲区，使用`scatter-gather IO`读取数据，能够动态扩展缓冲区大小，保证数据能够充分读取。
  - 使用原子变量标志保护取代`ONESHOT`，避免了因`ONESHOT`不断重新注册事件而造成的系统调用开销。
  - 为什么需要`ONESHOT`？
    使用`ONESHOT`是为了避免TCP分包造成的多个线程处理同一报文导致的报文解析失败问题。来看一下问题发生的大致原因：
    ![](http://43.138.43.178:8000/image/ReasonOfONESHOT.png)
    使用`ONESHOT`可以在第一次read得到的数据未处理完成之前不会将后半段数据读入到应用层，也就不会将其交给工作线程解析。如上所言，这种方案需要重复注册`EPOLLIN`事件，因此会有比较高的系统调用开销。那么接下来看使用原子变量标志的解决方案：
    ![](http://43.138.43.178:8000/image/BufferAtomic.png)
    这样就可以有效避免`ONESHOT`造成的系统调用开销



### 4. `TCPServer`

- `Acceptor -- Reactor`
  - `Acceptor`顾名思义，即调用`accept`接收新连接的类
  - `Acceptor`所属的事件循环只关注`listen_fd`的上的可读事件，即只负责接收新的连接，而不负责其他的IO事件。这就是主从Reactor模型中`MainReactor`的职责
- `EventLoopThreadPool -- SubReactors`
  - `EventLoopThreadPool`使用池的思想预先创建了一定数量的事件循环线程，避免了新连接到来动态创建事件循环的开销资源开销和时间开销，既节省了内存资源也提高了响应速度
  - 每当有新的连接到来时，`TCPServer`就会轮询式地获取到池中的一个事件循环，并将新连接交给该事件循环负责其IO事件，分配的事件循环就承担着主从Reactor模型中`SubReactor`的职责。



## 三、异步日志系统

日志系统采用双缓冲区实现高效写出

### 1.基本结构

![](http://43.138.43.178:8000/image/AsyncLogSystem.png)



### 2. 运行分析

- 前后端运行流程

  - 前端

    ```cpp
    void append(data)
    {
        // 为前端加锁
        lock_guard(mutex);
        
        // 若cur指向的Buffer有足够的空间，则直接写入
        if(cur->available() >= data.len())
        {
            cur->append(data);
    	}
        // cur指向的Buffer没有足够的空间写入data
        else
        {
            // 将cur的所有权转移至buffers
            buffers.push_back(std::move(cur));
            // 将next的所有权转移至cur，保证其他线程写入前端必写cur
            cur = std::move(next);
            
            // 经过上述操作，cur指向的Buffer必有空间，直接写入
            cur->append(data);
            // 由于已经有一个缓冲区满了，唤醒后端开始写入文件
            notifyBack();
    	}
    }
    ```

    

  - 后端

    ```cpp
    // 预备两个缓冲区和待写入缓冲区vector
    BufferPtr buffer1;
    BufferPtr buffer2;
    BufferVector writing;
    
    while(true)
    {
        // 等待超时或倍前端唤醒
        wait(timeout);
        
        // 加锁获取待写入的缓冲区
        {
            lock_guard(mutex);
            // 将cur的所有权转移至buffers
            buffers.push_back(std::move(cur));
            // 将buffers中的内容转移至writing中，这样空出了前端的buffers
            // 并且将待写入文件的Buffer填入了writing中
            writing.swap(buffers);
            // 将buffer1的所有权转移给cur，保证cur有空间可写
            cur = std::move(buffer1);
            // 若next没有指向任何内容，则将buffer2的所有权转一个next
            // 保证next可写
            if(!next) next = std::move(buffer2);
        }
        
        // 经过上述操作，前端状态恢复到了原始：cur、next分别指向一个Buffer，buffers为空
        // 并且待写入的数据也在writing中
        
        // 将数据写入文件中
        for(buffer : writing)
        {
            writrIntoFile(buffer);
    	}
        
        // 将writing中的所有权转移回buffer1和buffer2中
        if(!buffer1)
        {
            buffer1 = std::move(writing.back());
            writing.pop_back();
        }
        if(!buffer2)
        {
            buffer2 = std::move(writing.back());
            writing.pop_back();
    	}
        
        // 后端写完后，后端也回到了原始状态
        // buffer1、buffer2分别指向一块Buffer内存
        // writing为空
    }
    ```

    

- 图示

  - `cur`写满、`next`也有写入时的前端
    ![](http://43.138.43.178:8000/image/LogFrontDemo.png)
    注意此时`cur`原先持有的`BufferA`的所有权转移到了`buffers`中，而`next`持有的`BufferB`的所有权转移到了`cur`中，`next`此时为空，即：

    - `cur->BufferB`
    - `buffers[0]->BufferA`
    - `next->null`

  - 在上述情况下后端被唤醒

    - cur所有权转移至`buffers`中，即`buffers[1]->BufferB`，`cur->null`
      ![](http://43.138.43.178:8000/image/LogBackDemo1.png)

    - 加锁交换完毕，写入文件前，buffer1的所有权转移给cur，buffer2的所有权转移给next，buffers和writing交换内容，即：

      `cur->BufferC, next->BufferD, writing[0]->BufferA, writing[1]->BufferB`
      ![](http://43.138.43.178:8000/image/LogBackDemo2.png)

    - 写完后收回writing的资源
      ![](http://43.138.43.178:8000/image/LogBackDemo3.png)
      这样，状态又回归到了最初的状态（尽管指向的内存不同，但在可写入性上是一致的）

- 异步日志运行的基本情况如图示，其余各种情况之后单独详细分析

- 事实上，这样的双缓冲区队列含有弊端：在前端运行流程的第19、20行断言交换后cur指向必定有空间可以写入，但实际上如果日志短时间内大量写入，是有可能发生前端两个缓冲区都被写满而后端还尚未处理的情况的，这种情况下cur指向为空而无法被写入内容。这一点上需要改进

