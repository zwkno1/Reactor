#include <list>
#include <unordered_set>
#include <vector>
#include <sys/epoll.h>
#include <unistd.h>
#include <stdio.h>

#include "logger.h"
#include "udp/socket.h"
#include "udp/endpoint.h"

#include "deadlinetimer.h"
#include "systemexception.h"
#include "socketops.h"
#include "queue.h"
#include "objectpool.h"

using namespace detail;

class EchoSocket;

class EchoSocketManager
{
public:
    void add(EchoSocket *s);

    void del(EchoSocket *s);

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

class EchoSocket : public udp::Socket
{
    enum { max_send_size = 256, };
public:
    EchoSocket(Reactor & reactor, const udp::Endpoint & ep)
        : Socket(reactor, ep)
    {
    }

    bool send(const udp::Endpoint & ep, Buffer * buf)
    {
        send_buffers_.push(buf);
        return true;
    }

protected:
    void handle_events(Event event)
    {
        if(event & EPOLLIN)
        {
            int ec;
            size_t bytes;
            size_t addrlen = 0;

            while(true)
            {
                Buffer *b = alloc_buffer();
                socket_ops::buf buf;
                socket_ops::init_buf(buf, b->data, b->max_size);
                bool ret = socket_ops::non_blocking_recvfrom(handle(), &buf, 1, 0, 0, &addrlen, ec, bytes);
                if(ret == false)
                {
                    free_buffer(b);
                    break;
                }

                if(ec)
                {
                    free_buffer(b);
                    close();
                    return;
                }
                b->size = bytes;
                
                printf("size %d: ", (int)b->size);
                for(size_t i = 0; i < b->size; ++i)
                {
                    if(i%15 == 0)
                        printf("\n");
                    printf("%2x", 0xFF & b->data[i]);
                }
            }
        }

        if(event & EPOLLOUT)
        {
            
        }

        if(event & (EPOLLERR | EPOLLHUP))
        {
        }
    }

private:
	Buffer * alloc_buffer()
    {
        return buffer_pool.alloc();
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
        
    }

	Queue<Buffer> send_buffers_;
    std::vector<socket_ops::buf> send_buffers_helper_;
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

int main(int argc, char *argv[])
{
    try
    {
        Reactor reactor;
        udp::Endpoint endpoint("0.0.0.0", 60000);
        EchoSocket socket(reactor, endpoint);
        reactor.run();
    }
    catch(const SystemException & err)
    {
        Logger::debug() << err.ec() << "," << err.what();
    }

    return 0;
}
