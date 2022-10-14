# 项目整体框架

# 一、框架图

![](http://43.138.43.178:8000/image/BasicFrame.png)



# 二、各层功能与接口概述

- `MyNet` 
  - 功能
    基于事件循环模型和IO Multiplexing（目前使用epoll）对TCP通信中的Server端进行各层封装，最终提供功能完善的`TCPServer`和`TCPConnection`，便于上层用户直接使用TCP通信
  - 向上层提供的接口
    - `TCPServer`
      对TCP通信中Server端的抽象封装，包括对`listenfd`的封装`Acceptor`和对`connfd`的封装`TCPConnection`
    - `TCPConnection`
      对TCP通信中连接的抽象封装，包含了一次连接五元组的必要信息和对应的读写操作
  - 详细解析：[MyNet](https://github.com/Phos35/MyTinyWebServer/tree/master/Documents/MyNet)
- `MyHTTP`
  - 功能
    基于`MyNet`进行HTTP封装，主要为解析`MyNet`接收的数据，解析为HTTP请求报文并传递给更上层的用户
  - 向上层提供的接口
    - `HTTPServer`
      对`TCPServer`的进一步封装，对上层暴露`start`接口用于启动HTTP服务器；更主要的是向上层暴露了`setOnGET`、`setOnPOST`等方法（目前只支持`GET`和`POST`），使得用户能够自定义请求处理函数
  - 详细解析：[MyHTTP](https://github.com/Phos35/MyTinyWebServer/tree/master/Documents/MyHTTP)
- `User`
  - 功能
    使用`MyHTTP`提供的接口设置HTTP请求报文的处理、配置运行HTTP服务器