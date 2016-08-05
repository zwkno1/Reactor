#include "socket.h"

#include <sys/epoll.h>

#include "socketops.h"
#include "systemexception.h"

namespace tcp
{
    
Socket::Socket(Reactor & reactor, int socket)
    : reactor_(&reactor)
    , socket_(socket)
    , closed_(false)
{
    int ec;
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

}// namespace tcp
