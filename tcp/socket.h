#ifndef SOCKET_H
#define SOCKET_H

#include "reactor.h"

namespace tcp
{
    
class Socket : public EventHandler
{
public:
    Socket(Reactor & reactor, int socket);
    ~Socket();
    bool is_closed() { return closed_; }

    virtual int handle() { return socket_; }

private:
    void close();

    Reactor * reactor_;
    int socket_;
    bool closed_;
};

}// namespace tcp

#endif // SOCKET_H
