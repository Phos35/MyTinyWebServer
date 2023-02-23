#include "Socket.h"
#include <netinet/tcp.h>
#include <unistd.h>

Socket Socket::create(int domain, int type, int protocol, 
                      const std::string &ip, uint16_t port)
{
    int fd = ::socket(domain, type, protocol);
    return Socket(fd, InetAddr(ip, port));
}

/**
 * @brief: 绑定地址和端口
 * @return {*}
 */    
int Socket::bind()
{
    return ::bind(sockfd_, addr_.sockaddr(), sizeof(sockaddr_in));
}

/**
 * @brief: 进入监听状态
 * @return {*}
 */    
int Socket::listen()
{
    return ::listen(sockfd_, 10000);
}

/**
 * @brief: 接收新的连接
 * @param {bool} block socket设置为阻塞的标志，若为false则设置nonblock
 * @return {*}
 */    
Socket Socket::accept(bool block)
{
    int flag = SOCK_CLOEXEC;
    if(block == false)
        flag |= SOCK_NONBLOCK;

    sockaddr_in client;
    socklen_t len = sizeof(client);
    int connfd = accept4(sockfd_, (sockaddr *)&client, &len, flag);

    return Socket(connfd, InetAddr(client));
}

/**
 * @brief: 取消Nagle算法
 * @return {*}
 */
void Socket::setTCPNoDelay(bool on)
{
    int opt = (on == true ? 1 : 0);
    ::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt));
}

/**
 * @brief: 复用地址
 * @return {*}
 */
void Socket::setResueAddr(bool on)
{
    int opt = (on == true ? 1 : 0);
    ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
}

/**
 * @brief: 复用端口
 * @return {*}
 */
void Socket::setReusePort(bool on)
{
    int opt = (on == true ? 1 : 0);
    ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));
}

/**
 * @brief: 关闭Socket
 * @return {*}
 */
void Socket::close()
{
    ::close(sockfd_);
}