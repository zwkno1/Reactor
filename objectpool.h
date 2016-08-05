#ifndef OBJECTPOOL_H
#define OBJECTPOOL_H

#include "noncopyable.h"

namespace detail {

template <typename Object>
class ObjectPool;

class ObjectPoolAccess
{
public:
  template <typename Object>
  static Object* create()
  {
    return new Object;
  }

  template <typename Object>
  static void destroy(Object* o)
  {
    delete o;
  }

  template <typename Object>
  static Object*& next(Object* o)
  {
    return o->next;
  }

  template <typename Object>
  static Object*& prev(Object* o)
  {
    return o->prev;
  }
};

template <typename Object>
class ObjectPool
  : private Noncopyable
{
public:
  // Constructor.
  ObjectPool()
    : live_list_(0),
      free_list_(0)
  {
  }

  // Destructor destroys all objects.
  ~ObjectPool()
  {
    destroy_list(live_list_);
    destroy_list(free_list_);
  }

  // Get the object at the start of the live list.
  Object* first()
  {
    return live_list_;
  }

  // Allocate a new object.
  Object* alloc()
  {
    Object* o = free_list_;
    if (o)
      free_list_ = ObjectPoolAccess::next(free_list_);
    else
      o = ObjectPoolAccess::create<Object>();

    ObjectPoolAccess::next(o) = live_list_;
    ObjectPoolAccess::prev(o) = 0;
    if (live_list_)
      ObjectPoolAccess::prev(live_list_) = o;
    live_list_ = o;

    return o;
  }

  // Free an object. Moves it to the free list. No destructors are run.
  void free(Object* o)
  {
    if (live_list_ == o)
      live_list_ = ObjectPoolAccess::next(o);

    if (ObjectPoolAccess::prev(o))
    {
      ObjectPoolAccess::next(ObjectPoolAccess::prev(o))
        = ObjectPoolAccess::next(o);
    }

    if (ObjectPoolAccess::next(o))
    {
      ObjectPoolAccess::prev(ObjectPoolAccess::next(o))
        = ObjectPoolAccess::prev(o);
    }

    ObjectPoolAccess::next(o) = free_list_;
    ObjectPoolAccess::prev(o) = 0;
    free_list_ = o;
  }

private:
  // Helper function to destroy all elements in a list.
  void destroy_list(Object* list)
  {
    while (list)
    {
      Object* o = list;
      list = ObjectPoolAccess::next(o);
      ObjectPoolAccess::destroy(o);
    }
  }

  // The list of live objects.
  Object* live_list_;

  // The free list.
  Object* free_list_;
};

} // namespace detail


#endif // OBJECTPOOL_H
