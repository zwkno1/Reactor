#ifndef SOCKETOPTIONS_H
#define SOCKETOPTIONS_H

#include <netinet/in.h>
#include <cstddef>
#include <sys/un.h>

namespace socket_ops
{

// Socket state bits.
enum
{
  // The user wants a non-blocking socket.
  user_set_non_blocking = 1,

  // The socket has been set non-blocking.
  internal_non_blocking = 2,

  // Helper "state" used to determine whether the socket is non-blocking.
  non_blocking = user_set_non_blocking | internal_non_blocking,

  // User wants connection_aborted errors, which are disabled by default.
  enable_connection_aborted = 4,

  // The user set the linger option. Needs to be checked when closing.
  user_set_linger = 8,

  // The socket is stream-oriented.
  stream_oriented = 16,

  // The socket is datagram-oriented.
  datagram_oriented = 32,

  // The socket may have been dup()-ed.
  possible_dup = 64
};

typedef int socket_type;
const int invalid_socket = -1;
const int socket_error_retval = -1;
const int max_addr_v4_str_len = INET_ADDRSTRLEN;

const int custom_socket_option_level = 0xA5100000;
const int enable_connection_aborted_option = 1;
const int always_fail_option = 2;

typedef sockaddr socket_addr_type;
typedef unsigned int sockaddr_len_type;
typedef in_addr in4_addr_type;

typedef sockaddr_in sockaddr_in4_type;

typedef sockaddr_un sockaddr_un_type;
//typedef addrinfo addrinfo_type;
typedef ::linger linger_type;
typedef int ioctl_arg_type;
typedef uint32_t u_long_type;
typedef uint16_t u_short_type;

typedef int error_code_type;
typedef unsigned char state_type;
typedef ssize_t signed_size_type;


socket_type accept(socket_type s, socket_addr_type* addr,
    std::size_t* addrlen, error_code_type& ec);

socket_type sync_accept(socket_type s,
    state_type state, socket_addr_type* addr,
    std::size_t* addrlen, error_code_type& ec);

bool non_blocking_accept(socket_type s,
    state_type state, socket_addr_type* addr, std::size_t* addrlen,
    error_code_type& ec, socket_type& new_socket);

int bind(socket_type s, const socket_addr_type* addr,
    std::size_t addrlen, error_code_type& ec);

int close(socket_type s, bool destruction, error_code_type& ec);

bool set_non_blocking(socket_type s, bool value, error_code_type& ec);

int shutdown(socket_type s,
    int what, error_code_type& ec);

int connect(socket_type s, const socket_addr_type* addr,
    std::size_t addrlen, error_code_type& ec);

void sync_connect(socket_type s, const socket_addr_type* addr,
    std::size_t addrlen, error_code_type& ec);

bool non_blocking_connect(socket_type s,
    error_code_type& ec);

int socketpair(int af, int type, int protocol,
    socket_type sv[2], error_code_type& ec);

bool sockatmark(socket_type s, error_code_type& ec);

size_t available(socket_type s, error_code_type& ec);

int listen(socket_type s,
    int backlog, error_code_type& ec);

typedef iovec buf;

void init_buf(buf& b, void* data, size_t size);

void init_buf(buf& b, const void* data, size_t size);

signed_size_type recv(socket_type s, buf* bufs,
    size_t count, int flags, error_code_type& ec);

size_t sync_recv(socket_type s, state_type state, buf* bufs,
    size_t count, int flags, bool all_empty, error_code_type& ec);

bool non_blocking_recv(socket_type s,
    buf* bufs, size_t count, int flags, bool is_stream,
    error_code_type& ec, size_t& bytes_transferred);

signed_size_type recvfrom(socket_type s, buf* bufs,
    size_t count, int flags, socket_addr_type* addr,
    std::size_t* addrlen, error_code_type& ec);

size_t sync_recvfrom(socket_type s, state_type state,
    buf* bufs, size_t count, int flags, socket_addr_type* addr,
    std::size_t* addrlen, error_code_type& ec);

bool non_blocking_recvfrom(socket_type s,
    buf* bufs, size_t count, int flags,
    socket_addr_type* addr, std::size_t* addrlen,
    error_code_type& ec, size_t& bytes_transferred);

signed_size_type recvmsg(socket_type s, buf* bufs,
    size_t count, int in_flags, int& out_flags,
    error_code_type& ec);

size_t sync_recvmsg(socket_type s, state_type state,
    buf* bufs, size_t count, int in_flags, int& out_flags,
    error_code_type& ec);

bool non_blocking_recvmsg(socket_type s,
    buf* bufs, size_t count, int in_flags, int& out_flags,
    error_code_type& ec, size_t& bytes_transferred);

signed_size_type send(socket_type s, const buf* bufs,
    size_t count, int flags, error_code_type& ec);

size_t sync_send(socket_type s, state_type state,
    const buf* bufs, size_t count, int flags,
    bool all_empty, error_code_type& ec);

bool non_blocking_send(socket_type s,
    const buf* bufs, size_t count, int flags,
    error_code_type& ec, size_t& bytes_transferred);

signed_size_type sendto(socket_type s, const buf* bufs,
    size_t count, int flags, const socket_addr_type* addr,
    std::size_t addrlen, error_code_type& ec);

size_t sync_sendto(socket_type s, state_type state,
    const buf* bufs, size_t count, int flags, const socket_addr_type* addr,
    std::size_t addrlen, error_code_type& ec);

socket_type socket(int af, int type, int protocol,
    error_code_type& ec);

int setsockopt(socket_type s, int level, int optname,
    const void* optval, std::size_t optlen, error_code_type& ec);

int getsockopt(socket_type s, state_type state,
    int level, int optname, void* optval,
    size_t* optlen, error_code_type& ec);

int poll_connect(socket_type s, error_code_type& ec);

int ioctl(socket_type s, state_type& state,
    int cmd, ioctl_arg_type* arg, error_code_type& ec);

int select(int nfds, fd_set* readfds, fd_set* writefds,
    fd_set* exceptfds, timeval* timeout, error_code_type& ec);

int poll_read(socket_type s,
    state_type state, error_code_type& ec);

int poll_write(socket_type s,
    state_type state, error_code_type& ec);

int poll_connect(socket_type s, error_code_type& ec);

} // namespace socket_ops

#endif
