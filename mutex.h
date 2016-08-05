#ifndef MUTEX_H
#define MUTEX_H

#include <mutex>
#include "scopedlock.h"
#include "noncopyable.h"

class Mutex : private Noncopyable
{
public:
    typedef detail::ScopedLock<Mutex> ScopedLock;

    void lock()
    {
        mutex_.lock();
    }

    void unlock()
    {
        mutex_.unlock();
    }

private:
    std::mutex mutex_;
};

#endif // MUTEX_H
