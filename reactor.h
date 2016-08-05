#pragma once

#include <stdint.h>

typedef uint32_t Event;
typedef int Handle;

class EventHandler
{
public:
    virtual Handle handle() = 0;
    virtual void handle_events(Event events) = 0;
    virtual ~EventHandler()
    {
    }
};

class Reactor
{
public:
    Reactor();
    ~Reactor();

    void run();
    int register_handle(EventHandler * handler, Event event);
    void deregister_handle(EventHandler *handler);

private:
    friend class Proactor;

    enum { epoll_size = 20000 };

    int do_epoll_create();

    int do_timerfd_create();

    bool stopped_;

    int epoll_fd_;
};


