#ifndef SOCKET_H
#define SOCKET_H

#include "reactor.h"
#include "endpoint.h"

namespace udp
{
    
class Socket : public EventHandler
{
public:
    Socket(Reactor & reactor, const Endpoint & ep);
    
    ~Socket();
    
    bool is_closed() { return closed_; }

    virtual int handle() { return socket_; }

private:
    void close();
    
    Reactor * reactor_;
    
    Endpoint endpoint_;
    
    int socket_;
    
    bool closed_;
};

}// namespace udp

#endif // SOCKET_H
