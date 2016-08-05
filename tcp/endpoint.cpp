#include "endpoint.h"

#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>

namespace tcp
{
    
Endpoint::Endpoint(const std::string & ip, unsigned short port)
    : ip_(ip)
    , port_(port)
{
}

bool Endpoint::get_sockaddr(sockaddr_in& addr) const
{
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port_);

    if(inet_pton(AF_INET, ip_.c_str(), &addr.sin_addr) != 1)
        return false;
    return true;
}

}// namespace tcp