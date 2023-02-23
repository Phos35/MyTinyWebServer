#include "InetAddr.h"
#include <string.h>

InetAddr::InetAddr(const std::string& ip, uint16_t port)
{
    addr_.sin_family = AF_INET;
    inet_pton(AF_INET, ip.c_str(), &addr_.sin_addr.s_addr);
    addr_.sin_port = htons(port);
}

InetAddr::InetAddr(const sockaddr_in &addr)
{
    memcpy(&addr_, &addr, sizeof(addr_));
}

std::string InetAddr::fullAddr()const
{
    char ip[64] = {0};
    inet_ntop(AF_INET, &addr_.sin_addr.s_addr, ip, sizeof(ip));
    return ip + std::to_string(addr_.sin_port);
}