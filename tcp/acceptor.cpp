#include "acceptor.h"

#include <sys/epoll.h>

#include "socketops.h"
#include "systemexception.h"

namespace tcp
{
    
Acceptor::Acceptor(Reactor & reactor, const Endpoint & ep)
    : reactor_(&reactor)
    , endpoint_(ep)
    , handle_(do_acceptor_create())
    , closed_(false)
{
    Event event = EPOLLIN | EPOLLERR | EPOLLET;
    int ec = reactor_->register_handle(this, event);
    throw_error(ec, "register acceptor");
}

Acceptor::~Acceptor()
{
    destroy();
}

void Acceptor::destroy()
{
    if(handle_ != socket_ops::invalid_socket)
    {
        int ec;
        socket_ops::close(handle_, true, ec);
        handle_ = socket_ops::invalid_socket;
    }
}

int Acceptor::do_acceptor_create()
{
    int ec;
    int fd = socket_ops::socket(AF_INET, SOCK_STREAM, 0, ec);
    throw_error(ec, "bind: create socket");

    int opt=1;
    socket_ops::setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt), ec);
    throw_error(ec, "bind: set reuseaddr");

    socket_ops::set_non_blocking(fd, true, ec);
    throw_error(ec, "bind: set nonblocking");

    struct sockaddr_in addr;
    int result = endpoint_.get_sockaddr(addr);
    if(!result)
        throw_error(-1, "bind: bad addr");

    socket_ops::bind(fd, (sockaddr *)&addr, sizeof(addr), ec);
    throw_error(ec, "bind: bind");

    socket_ops::listen(fd, 100, ec);
    throw_error(ec, "bind: listen");

    return fd;
}

}// namespace tcp