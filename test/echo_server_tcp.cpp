#include <list>
#include <unordered_set>
#include <vector>
#include <sys/epoll.h>
#include <unistd.h>
#include <atomic>

#include "logger.h"
#include "tcp/socket.h"
#include "tcp/acceptor.h"
#include "deadlinetimer.h"
#include "systemexception.h"
#include "socketops.h"
#include "queue.h"
#include "objectpool.h"

std::atomic<uint64_t> read_event_count(0);
std::atomic<uint64_t> write_event_count(0);

#define __DEBUG__

using namespace detail;

class EchoSocket;

class EchoSocketManager
{
public:
    void add(EchoSocket *s);

    void del(EchoSocket *s);

    void check_timeout();
private:
    std::unordered_set<EchoSocket *> sockets_;
};

EchoSocketManager socket_manager;

class Buffer : private Noncopyable
{
    friend class detail::QueueAccess;
    friend class detail::ObjectPoolAccess;
public:
    Buffer()
        :size(0)
    {
    }

    void destroy();

    enum { max_size = 256, };
    char data[max_size];

    Buffer * next_;
    Buffer * prev;
    Buffer * next;
    size_t size;
};

ObjectPool<Buffer> buffer_pool;

void Buffer::destroy()
{
    buffer_pool.free(this);
}

class EchoSocket : public tcp::Socket
{
public:
    EchoSocket(Reactor & reactor, int sockfd)
        : Socket(reactor, sockfd)
        , send_buffer_offset_(0)
        , timestamp_(std::time(0))
    {
    }

    bool send(Buffer * buf)
    {
        //if(send_buffers_.size() > 10240)
        //    return false;
        send_buffers_.push(buf);
        do_send();
        return true;
    }

    bool send(Queue<Buffer> & bufs)
    {
        if(send_buffers_.size() > 10240)
            return false;
        send_buffers_.push(bufs);
        do_send();
        return true;
    }

    bool check_timeout(time_t now)
    {
        if(timestamp_ + 60 > now)
            return false;
        else
            return true;
    }

protected:
    void handle_events(Event event)
    {
//#ifdef __DEBUG__
//        Logger::debug() << "socket(" << this->get_handle() << ") "
//                        <<(event & EPOLLIN ? "IN " : "")
//                       << (event & EPOLLOUT ? "OUT " : "")
//                       << (event & EPOLLERR ? "ERR " : "");
//#endif
        if(event & EPOLLIN)
        {
			++read_event_count;
            int ec;
            size_t bytes;
            Queue<Buffer> bufs;

            while(true)
            {
                Buffer *b = alloc_buffer();
                socket_ops::buf buf;
                socket_ops::init_buf(buf, b->data, b->max_size);
                bool ret = socket_ops::non_blocking_recv(handle(), &buf, 1, 0, true, ec, bytes);
                if(ret == false)
                {
                    free_buffer(b);
                    break;
                }

                if(ec)
                {
					Logger::debug() << "socket recv err(" << ec << "): " << strerror(ec) << event ; 
                    free_buffer(b);
                    close();
                    return;
                }

                b->size = bytes;
                bufs.push(b);
            }
            if(send(bufs) == false)
			{
			  Logger::debug() << "send buf too long";
			  close();
			  return;
			}
            timestamp_ = std::time(0);
        }

        if(event & EPOLLOUT)
        {
			++write_event_count;
            do_send();
        }

        if(event & (EPOLLERR | EPOLLHUP))
        {
            close();
        }
    }

private:
	Buffer * alloc_buffer()
    {
        return buffer_pool.alloc();
        //return new Buffer;
    }

	void free_buffer(Buffer * b)
    {
        buffer_pool.free(b);
        //delete b;
	}

    void close()
    {
        socket_manager.del(this);
    }

    void do_send()
    {
        if(send_buffers_.front() == 0 || is_closed())
            return;

        int ec;
        size_t bytes;
        send_buffers_helper_.resize(send_buffers_.size());
        auto bh = send_buffers_helper_.begin();
        auto buf = send_buffers_.front();
        socket_ops::init_buf(*bh, buf->data + send_buffer_offset_, buf->size - send_buffer_offset_);
        ++bh;
        buf = buf->next_;
        while(buf)
        {
            socket_ops::init_buf(*bh, buf->data, buf->size);
			buf = buf->next_;
            ++bh;
        }

        bool complete = socket_ops::non_blocking_send(handle(), &send_buffers_helper_[0], send_buffers_helper_.size(), 0, ec, bytes);
        if(complete && ec)
        {
			Logger::debug() << "socket send err(" << ec << "): " << strerror(ec) ; 
            close();
            return;
        }

        buf = send_buffers_.front();

        if(bytes >= buf->size - send_buffer_offset_)
        {
            bytes -= buf->size - send_buffer_offset_;
			Buffer *tmp = buf;
			send_buffers_.pop();
            free_buffer(tmp);
            buf = send_buffers_.front();
            while(buf)
            {
                if(bytes >= buf->size)
                {
                    bytes -= buf->size;
					Buffer *tmp = buf;
					send_buffers_.pop();
                    free_buffer(tmp);
                    buf = send_buffers_.front();
                }
                else
                {
                    send_buffer_offset_ = bytes;
                    break;
                }
            }
        }
        else
        {
            send_buffer_offset_ += bytes;
        }
    }

	Queue<Buffer> send_buffers_;
    size_t send_buffers_size_;
    size_t send_buffer_offset_;
    std::vector<socket_ops::buf> send_buffers_helper_;
    time_t timestamp_;
};


void EchoSocketManager::add(EchoSocket *s)
{
#ifdef __DEBUG__
    Logger::debug() << "add socket: " << s->handle() ;
#endif
    sockets_.insert(s);
}

void EchoSocketManager::del(EchoSocket *s)
{
#ifdef __DEBUG__
        Logger::debug() << "del socket: " << s->handle() ;
#endif
        sockets_.erase(s);
        delete s;
}

void EchoSocketManager::check_timeout()
{
    auto now = std::time(0);
    for(auto i = sockets_.begin(); i != sockets_.end();)
    {
        if((*i)->check_timeout(now))
        {
#ifdef __DEBUG__
            Logger::debug() << "socket timeout: " << (*i)->handle() ;
#endif
            delete *i;
            i = sockets_.erase(i);
        }
        else
        {
            ++i;
        }
    }
}

class EchoAcceptor : public tcp::Acceptor
{
public:
    EchoAcceptor(Reactor & reactor, const tcp::Endpoint & endpoint)
        : Acceptor(reactor, endpoint)
    {
    }
protected:
    void handle_events(Event event)
    {
        if(event | EPOLLIN)
        {
            while(true)
            {
                int sock = accept(handle(), 0, 0);
                if(sock != -1)
                {
                    socket_manager.add(new EchoSocket(get_reactor(), sock));
                }
                else
                    break;
            }
        }
    }
};

class EchoTimer : public DeadlineTimer
{
public:
    EchoTimer(Reactor & reactor)
        : DeadlineTimer(reactor)
    {
        this->set_interval(1);
    }

protected:
    void handle_events(Event event)
    {
        if(event | EPOLLIN)
        {
            uint64_t exp = 0;
            while((exp = read(handle(), &exp, sizeof(exp))) == sizeof(exp))
            {
                //Logger::debug() << "tick..";
                socket_manager.check_timeout();
				uint64_t c1 = read_event_count;
				uint64_t c2 = write_event_count;
				Logger::debug() << "process event (read: " << c1 << ", write: " << c2 << ")";
				read_event_count = 0;
				write_event_count = 0;
            }
        }
    }
};

int main(int argc, char *argv[])
{
    try
    {
        Reactor reactor;
        tcp::Endpoint endpoint("0.0.0.0", 20000);
        EchoAcceptor acceptor(reactor, endpoint);
        EchoTimer timer(reactor);
        reactor.run();
    }
    catch(const SystemException & err)
    {
        Logger::debug() << err.ec() << "," << err.what();
    }

    return 0;
}
