#include "deadlinetimer.h"

#include <sys/timerfd.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <errno.h>

#include "socketops.h"
#include "systemexception.h"

DeadlineTimer::DeadlineTimer(Reactor & reactor)
    : reactor_(&reactor)
    , timer_fd_(do_timerfd_create())
{
    int ec = 0;
    socket_ops::set_non_blocking(timer_fd_,  true, ec);
    throw_error(ec, "set noblocking");
    Event event = EPOLLIN | EPOLLERR | EPOLLET;
    reactor_->register_handle(this, event);
}

DeadlineTimer::~DeadlineTimer()
{
    if(timer_fd_ > 0)
        ::close(timer_fd_);
}

int DeadlineTimer::do_timerfd_create()
{
  int fd = timerfd_create(CLOCK_REALTIME, TFD_CLOEXEC | TFD_NONBLOCK);
  if (fd == -1)
      throw_error(errno, "timefd");
  return fd;
}

bool DeadlineTimer::set_interval(int seconds)
{
    timespec now;
    if(clock_gettime(CLOCK_REALTIME, &now) == -1)
        return false;
    itimerspec val = {0};
    val.it_value.tv_sec = now.tv_sec;
    val.it_value.tv_nsec = now.tv_nsec;

    val.it_interval.tv_sec = seconds;
    val.it_interval.tv_nsec = 0;
    return timerfd_settime(timer_fd_, TFD_TIMER_ABSTIME, &val, NULL) != -1;
}
