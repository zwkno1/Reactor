#ifndef SCOPEDLOCK_H
#define SCOPEDLOCK_H

#include "noncopyable.h"

namespace detail
{

// Helper class to lock and unlock a mutex automatically.
template <typename Mutex>
class ScopedLock
  : private Noncopyable
{
public:
  // Tag type used to distinguish constructors.
  enum adopt_lock_t { adopt_lock };

  // Constructor adopts a lock that is already held.
  ScopedLock(Mutex& m, adopt_lock_t)
    : mutex_(m),
      locked_(true)
  {
  }

  // Constructor acquires the lock.
  explicit ScopedLock(Mutex& m)
    : mutex_(m)
  {
    mutex_.lock();
    locked_ = true;
  }

  // Destructor releases the lock.
  ~ScopedLock()
  {
    if (locked_)
      mutex_.unlock();
  }

  // Explicitly acquire the lock.
  void lock()
  {
    if (!locked_)
    {
      mutex_.lock();
      locked_ = true;
    }
  }

  // Explicitly release the lock.
  void unlock()
  {
    if (locked_)
    {
      mutex_.unlock();
      locked_ = false;
    }
  }

  // Test whether the lock is held.
  bool locked() const
  {
    return locked_;
  }

  // Get the underlying mutex.
  Mutex& mutex()
  {
    return mutex_;
  }

private:
  // The underlying mutex.
  Mutex& mutex_;

  // Whether the mutex is currently locked or unlocked.
  bool locked_;
};

} // namespace detail

#endif // SCOPEDLOCK_H
