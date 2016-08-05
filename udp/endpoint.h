#ifndef ENDPOINT_H
#define ENDPOINT_H

#include <string>
#include <netinet/in.h>

namespace udp
{
    
class Endpoint
{
public:
    Endpoint(const std::string & ip, unsigned short port);
    
    Endpoint(const Endpoint & oher);
    Endpoint(Endpoint && oher);
    
    const sockaddr_in & addr() const;
    
private:
    sockaddr_in addr_;
};

}// namespace udp

#endif // ENDPOINT_H
