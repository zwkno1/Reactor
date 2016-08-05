#include "socket.h"

#include <sys/epoll.h>

#include "socketops.h"
#include "systemexception.h"

namespace udp
{
    
Socket::Socket(Reactor & reactor, const Endpoint & ep)
    : reactor_(&reactor)
    , endpoint_(ep)
    , closed_(true)
{
    int ec;
    socket_ = socket_ops::socket(AF_INET, SOCK_DGRAM, 0, ec);
    throw_error(ec, "create udp socket");
    closed_ = false;
    
    socket_ops::bind(socket_, (const sockaddr *)&ep.addr(), sizeof(ep.addr()), ec);
    throw_error(ec, "bind udp socket");
    
    socket_ops::set_non_blocking(socket_, true, ec);
    throw_error(ec, "set noblocking");

    Event event = EPOLLIN | EPOLLOUT | EPOLLPRI | EPOLLERR | EPOLLET;
    ec = reactor_->register_handle(this, event);
    throw_error(ec, "register socket");
}

Socket::~Socket()
{
    close();
}

void Socket::close()
{
    if(!closed_)
    {
        int ec;
        socket_ops::close(socket_, true, ec);
        closed_ = true;
    }
}

}// namespace udp
