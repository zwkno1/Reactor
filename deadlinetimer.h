#ifndef DEADLINETIMER_H
#define DEADLINETIMER_H

#include "reactor.h"

class DeadlineTimer : public EventHandler
{
public:
    DeadlineTimer(Reactor & reactor);
    ~DeadlineTimer();

    virtual int handle() { return timer_fd_; }

    bool set_interval(int seconds);

private:
    int do_timerfd_create();

    Reactor * reactor_;
    int timer_fd_;
};

#endif // DEADLINETIMER_H
