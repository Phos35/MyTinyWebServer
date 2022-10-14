#ifndef SOCKET_H
#define SOCKET_H
#include <sys/socket.h>
#include "InetAddr.h"

class Socket{
public:
    static Socket create(int domain, int type, int protocol, const std::string &ip, uint16_t port);

    Socket(int fd, const InetAddr &addr)
    :sockfd_(fd), addr_(addr){}

    /**
     * @brief: 绑定地址和端口 -- Server使用
     * @return {*}
     */    
    int bind();

    /**
     * @brief: 进入监听状态
     * @return {*}
     */    
    int listen();

    /**
     * @brief: 接收新的连接
     * @param {bool} block socket设置为阻塞的标志，若为false则设置nonblock
     * @return {*}
     */    
    Socket accept(bool block);

    /**
     * @brief: 关闭Socket
     * @return {*}
     */
    void close();

    /*成员/属性get函数*/
    int fd()const { return sockfd_; }
    std::string fullAddr()const { return addr_.fullAddr(); }
    bool valid()const { return sockfd_ >= 0; }

    /*设置Socket属性*/
    void setTCPNoDelay(bool on);
    void setResueAddr(bool on);
    void setReusePort(bool on);
    

private:
    int sockfd_;    // 套接字对应的文件描述符
    InetAddr addr_; // 套接字对应的地址
};

#endif