#ifndef ACCEPTOR_H
#define ACCEPTOR_H

#include "reactor.h"
#include "endpoint.h"

namespace tcp
{

class Acceptor : public EventHandler
{
public:
    Acceptor(Reactor & reactor, const Endpoint & ep);
    ~Acceptor();
    virtual int handle() { return handle_; }
    Reactor & get_reactor() { return *reactor_; }

private:
    void destroy();

    int do_acceptor_create();

    Reactor * reactor_;
    Endpoint endpoint_;

    int handle_;

    bool closed_;
};

}// namespace tcp

#endif // ACCEPTOR_H
