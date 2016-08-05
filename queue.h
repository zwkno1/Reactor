#ifndef QUEUE_H
#define QUEUE_H

#include "noncopyable.h"

namespace detail
{

template <typename Element>
class Queue;

class QueueAccess
{
public:
  template <typename Element>
  static Element* next(Element* e)
  {
    return static_cast<Element*>(e->next_);
  }

  template <typename Element1, typename Element2>
  static void next(Element1*& e1, Element2* e2)
  {
    e1->next_ = e2;
  }

  template <typename Element>
  static void destroy(Element* e)
  {
    e->destroy();
  }

  template <typename Element>
  static Element*& front(Queue<Element>& q)
  {
    return q.front_;
  }

  template <typename Element>
  static Element*& back(Queue<Element>& q)
  {
    return q.back_;
  }
};

template <typename Element>
class Queue : private Noncopyable
{
public:
  // Constructor.
  Queue()
    : front_(0)
    , back_(0)
    , size_(0)
  {
  }

  // Destructor destroys all Elements.
  ~Queue()
  {
    while (Element* op = front_)
    {
      pop();
      QueueAccess::destroy(op);
    }
  }

  // Get the Element at the front of the queue.
  Element* front()
  {
    return front_;
  }

  // Pop an Element from the front of the queue.
  void pop()
  {
    if (front_)
    {
      Element* tmp = front_;
      front_ = QueueAccess::next(front_);
      if (front_ == 0)
        back_ = 0;
      QueueAccess::next(tmp, static_cast<Element*>(0));
      --size_;
    }
  }

  // Push an Element on to the back of the queue.
  void push(Element* h)
  {
    QueueAccess::next(h, static_cast<Element*>(0));
    if (back_)
    {
      QueueAccess::next(back_, h);
      back_ = h;
    }
    else
    {
      front_ = back_ = h;
    }
    ++size_;
  }

  // Push all Elements from another queue on to the back of the queue. The
  // source queue may contain Elements of a derived type.
  template <typename OtherElement>
  void push(Queue<OtherElement>& q)
  {
    if (Element* other_front = QueueAccess::front(q))
    {
      if (back_)
        QueueAccess::next(back_, other_front);
      else
        front_ = other_front;
      back_ = QueueAccess::back(q);
      QueueAccess::front(q) = 0;
      QueueAccess::back(q) = 0;
      size_ += q.size();
      q.size_ = 0;
    }
  }

  // Whether the queue is empty.
  bool empty() const
  {
    return front_ == 0;
  }

  // The size of queue.
  size_t size() const
  {
    return size_;
  }

private:
  friend class QueueAccess;

  // The front of the queue.
  Element* front_;

  // The back of the queue.
  Element* back_;

  // The size of queue.
  size_t size_;
};

} // namespace detail

#endif // QUEUE_H
