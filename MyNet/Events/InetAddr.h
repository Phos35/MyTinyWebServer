#ifndef INET_ADDR_H
#define INET_ADDR_H
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string>

class InetAddr{
public:
    InetAddr(const std::string& ip, uint16_t port);
    InetAddr(const sockaddr_in &addr);

    /*成员/属性/状态 get函数*/
    struct sockaddr *sockaddr()const { return (struct sockaddr *)&addr_; }
    std::string fullAddr()const;

private:
    sockaddr_in addr_;
};

#endif