#include "endpoint.h"

#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>

#include "systemexception.h"

namespace udp
{
    
Endpoint::Endpoint(const std::string & ip, unsigned short port)
{
    std::memset(&addr_, 0, sizeof(addr_));
    addr_.sin_family = AF_INET;
    addr_.sin_port = htons(port);
    if(inet_pton(AF_INET, ip.c_str(), &addr_.sin_addr) != 1)
    {
        throw_error(-1, "bad endpoint");
    }
}

Endpoint::Endpoint(const Endpoint & other)
{
    memcpy(&this->addr_, &other.addr_, sizeof(this->addr_));
}

Endpoint::Endpoint(Endpoint && other)
{
    memcpy(&this->addr_, &other.addr_, sizeof(this->addr_));
}

const sockaddr_in & Endpoint::addr() const
{
    return addr_;
}

}// namespace udp