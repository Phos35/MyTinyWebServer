# MyTinyWebServer

## 一、简介

本项目为Linux下C++轻量级高并发Web服务器。该项目参考muduo设计实现了一个简单的TCP网络库 -- MyNet，并在该网络库的基础上实现一层简单的HTTP封装，使得用户能够自定义HTTP请求的处理。



## 二、特点

- 使用C++11标准参考《Linux多线程服务端编程》实现了一个基于事件循环的简易网络库 -- MyNet
  - 主从Reactor并发模型
  - 事件驱动的线程异步唤醒机制
  - 事件驱动的定时器队列
  - 双缓冲区异步日志系统
  - 智能指针等RAII机制，减少内存泄漏的可能
- 基于上述网络库MyNet封装了HTTP请求、响应的处理
  - 使用主从状态机解析请求报文
  - 利用缓冲区的原子变量代替ONESHOT方案
  - 向上层提供`setOnGET`、`setOnPOST`接口，允许用户自定义HTTP请求处理方法
  - 利用定时器队列定时删除超时连接



## 三、各模块解析

- 项目框架：[基本框架解析](https://github.com/Phos35/MyTinyWebServer/tree/master/Documents/BasicFrame)
- 简易网络库MyNet：[MyNet](https://github.com/Phos35/MyTinyWebServer/tree/master/Documents/MyNet)
- 基于MyNet的HTTP封装：[MyHTTP](https://github.com/Phos35/MyTinyWebServer/tree/master/Documents/MyHTTP)



## 四、运行环境

- Machine：腾讯云轻量应用服务器
  - CPU：Intel(R) Xeon(R) Platinum 8255C CPU @ 2.50GHz
  - Memory: 8GB
  - NetWork Bandwidth: 10Mbps
- OS：Ubuntu 20.04 LTS
- Compiler:  g++ 9.4.0
- Debugger:  GNU gdb 9.2
- CMake: 3.16.3



## 五、运行

- 构建命令

  ```shell
  cd Build
  cmake ..
  make
  ```

- 运行

  ```shell
  ./Webserver [-p port] [-d dbpoll] [-e eventloop] [-w workerpool] [-l logtype] [-t trigger]
  ```

- 选项解析

  ```shell
  -p, --port=PORT_NUM            服务器端口号
  -d, --dbpool=POOL_SIZE         数据库连接池的容量
  -e, --eventloop=POOL_SIZE      TCP事件循环线程池的容量
  -w, --workerpool=POOL_SIZE     HTTP请求解析工作线程池的容量
  -l, --logtype=LOG_TYPE         日志输出的方式. F-输出到文件；S-标准输出
  -t, --trigger=TRIGGER_MODE     epoll针对TCP连接的触发模式. L-LT；E-ET
  ```

  



## 四、Demo演示

[Demo](http://43.138.43.178:8000/image/demo.gif)



## 五、压力测试

- 测试条件
  - 测试主机信息
    - CPU：Intel(R) Xeon(R) Platinum 8163 CPU @ 2.50GHz
    - Memory：4GB
    - Network Bandwidth：5Mbps
    - OS：Ubuntu 20.04.5 LTS
  - 被测试程序
    - 本项目、对比项目（[TinyWebServer](https://github.com/qinguoyi/TinyWebServer)）
    - listenfd均使用LT，可测试变量为connfd的触发模式（LT/ET）
    - 日志使用异步、写入文件的模式
    - 工作线程池容量均为10
    - 均使用`-O2`优化
    - 性能表现使用10次测试中的最高值
- 测试结果
  - 测试语句
    ` webbench -2 -c 5000 -t 10 http://43.138.43.178:8000/html/home.html`
    - 语句解析
      使用HTTP/1.1，并发5000客户端，持续请求10s。
  - 本项目表现
    - LT模式
      ![](http://43.138.43.178:8000/image/ShortLink-Self-LT.png)
    - ET模式
      ![](http://43.138.43.178:8000/image/ShortLink-Self-ET.png)
  - 对比项目表现
    - LT模式
      ![](http://43.138.43.178:8000/image/ShortLink-Compare-LT.png)
    - ET模式
      ![](http://43.138.43.178:8000/image/ShortLink-Compare-ET.png)
- 注：对比项目介绍在LT+LT模式下可达到Speed=5595108 pages/min，本测试结果数值远远低于该值，原因可能为云服务器的带宽限制（10Mbps）