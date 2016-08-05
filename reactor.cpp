#include "reactor.h"

#include <sys/epoll.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>

#include "systemexception.h"

Reactor::Reactor()
    : stopped_(false)
    , epoll_fd_(do_epoll_create())
{
}

Reactor::~Reactor()
{
    if (epoll_fd_ != -1)
      ::close(epoll_fd_);
}

void Reactor::run()
{
    while(!stopped_)
    {
        epoll_event events[2048];
        int num = epoll_wait(epoll_fd_, events, 128, -1);
        for(int i = 0; i < num; ++i)
        {
            EventHandler * h = static_cast<EventHandler *>(events[i].data.ptr);
            h->handle_events(events[i].events);
        }
    }
}

int Reactor::register_handle(EventHandler * handler, Event event)
{
    assert(handler != 0);

    epoll_event ev = {0,{0}};
    ev.events = event;
    ev.data.ptr = handler;
    int ret = epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, handler->handle(), &ev);
    if(ret == -1)
        return errno;
    return ret;
}

void Reactor::deregister_handle(EventHandler * handler)
{
    assert(handler != 0);

    epoll_event event = {0,{0}};
    epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, handler->handle(), &event);
}

int Reactor::do_epoll_create()
{
  int fd = epoll_create1(EPOLL_CLOEXEC);

  if (fd == -1 && (errno == EINVAL || errno == ENOSYS))
  {
    fd = epoll_create(epoll_size);
    if (fd != -1)
      ::fcntl(fd, F_SETFD, FD_CLOEXEC);
  }

  if (fd == -1)
    throw_error(errno, "epoll");

  return fd;
}
