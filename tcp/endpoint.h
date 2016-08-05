#ifndef ENDPOINT_H
#define ENDPOINT_H

#include <string>

class sockaddr_in;

namespace tcp
{
    
class Endpoint
{
public:
    Endpoint(const std::string & ip, unsigned short port);
    bool get_sockaddr(sockaddr_in& addr) const;
private:
    std::string ip_;
    unsigned short port_;
};

}// namespace tcp

#endif // ENDPOINT_H
