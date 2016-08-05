#include "socketops.h"

#include <errno.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <poll.h>

#include "error.h"

namespace socket_ops
{



inline void clear_last_error()
{
  errno = 0;
}

template <typename ReturnType>
inline ReturnType error_wrapper(ReturnType return_value,
    error_code_type& ec)
{
  ec = errno;
  return return_value;
}

template <typename SockLenType>
inline socket_type call_accept(SockLenType msghdr::*,
    socket_type s, socket_addr_type* addr, std::size_t* addrlen)
{
  SockLenType tmp_addrlen = addrlen ? (SockLenType)*addrlen : 0;
  socket_type result = ::accept(s, addr, addrlen ? &tmp_addrlen : 0);
  if (addrlen)
    *addrlen = (std::size_t)tmp_addrlen;
  return result;
}

socket_type accept(socket_type s, socket_addr_type* addr,
    std::size_t* addrlen, error_code_type& ec)
{
  if (s == invalid_socket)
  {
    ec = detail::error::bad_descriptor;
    return invalid_socket;
  }

  clear_last_error();

  socket_type new_s = error_wrapper(call_accept(
        &msghdr::msg_namelen, s, addr, addrlen), ec);
  if (new_s == invalid_socket)
    return new_s;

  ec = 0;
  return new_s;
}

bool non_blocking_accept(socket_type s,
    state_type state, socket_addr_type* addr, std::size_t* addrlen,
    socket_type& new_socket, error_code_type& ec)
{
  for (;;)
  {
    // Accept the waiting connection.
    new_socket = socket_ops::accept(s, addr, addrlen, ec);

    // Check if operation succeeded.
    if (new_socket != invalid_socket)
      return true;

    // Retry operation if interrupted by signal.
    if (ec == detail::error::interrupted)
      continue;

    // Operation failed.
    if (ec == detail::error::would_block
        || ec == detail::error::try_again)
    {
      if (state & user_set_non_blocking)
        return true;
      // Fall through to retry operation.
    }
    else if (ec == detail::error::connection_aborted)
    {
      if (state & enable_connection_aborted)
        return true;
      // Fall through to retry operation.
    }
#if defined(EPROTO)
    else if (ec == EPROTO)
    {
      if (state & enable_connection_aborted)
        return true;
      // Fall through to retry operation.
    }
#endif // defined(EPROTO)
    else
      return true;

    return false;
  }
}


template <typename SockLenType>
inline int call_bind(SockLenType msghdr::*,
    socket_type s, const socket_addr_type* addr, std::size_t addrlen)
{
  return ::bind(s, addr, (SockLenType)addrlen);
}

int bind(socket_type s, const socket_addr_type* addr,
    std::size_t addrlen, error_code_type& ec)
{
  if (s == invalid_socket)
  {
    ec = detail::error::bad_descriptor;
    return socket_error_retval;
  }

  clear_last_error();
  int result = error_wrapper(call_bind(
        &msghdr::msg_namelen, s, addr, addrlen), ec);
  if (result == 0)
    ec = 0;
  return result;
}

int close(socket_type s, bool destruction, error_code_type& ec)
{
  int result = 0;
  if (s != invalid_socket)
  {
    // We don't want the destructor to block, so set the socket to linger in
    // the background. If the user doesn't like this behaviour then they need
    // to explicitly close the socket.
    if (destruction)
    {
      ::linger opt;
      opt.l_onoff = 0;
      opt.l_linger = 0;
      error_code_type ignored_ec;
      socket_ops::setsockopt(s, SOL_SOCKET,
          SO_LINGER, &opt, sizeof(opt), ignored_ec);
    }

    clear_last_error();
    result = error_wrapper(::close(s), ec);

    if (result != 0
        && (ec == detail::error::would_block
          || ec == detail::error::try_again))
    {
      // According to UNIX Network Programming Vol. 1, it is possible for
      // close() to fail with EWOULDBLOCK under certain circumstances. What
      // isn't clear is the state of the descriptor after this error. The one
      // current OS where this behaviour is seen, Windows, says that the socket
      // remains open. Therefore we'll put the descriptor back into blocking
      // mode and have another attempt at closing it.      ioctl_arg_type arg = 0;
      ioctl_arg_type arg = 0;
      ::ioctl(s, FIONBIO, &arg);

      clear_last_error();
      result = error_wrapper(::close(s), ec);
    }
  }

  if (result == 0)
    ec = 0;
  return result;
}

bool set_non_blocking(socket_type s, bool value, error_code_type& ec)
{
  if (s == invalid_socket)
  {
    ec = detail::error::bad_descriptor;
    return false;
  }

  clear_last_error();
  ioctl_arg_type arg = (value ? 1 : 0);
  int result = error_wrapper(::ioctl(s, FIONBIO, &arg), ec);

  if (result >= 0)
  {
    ec = 0;
    return true;
  }

  return false;
}

int shutdown(socket_type s, int what, error_code_type& ec)
{
  if (s == invalid_socket)
  {
    ec = detail::error::bad_descriptor;
    return socket_error_retval;
  }

  clear_last_error();
  int result = error_wrapper(::shutdown(s, what), ec);
  if (result == 0)
    ec = 0;
  return result;
}

template <typename SockLenType>
inline int call_connect(SockLenType msghdr::*,
    socket_type s, const socket_addr_type* addr, std::size_t addrlen)
{
  return ::connect(s, addr, (SockLenType)addrlen);
}

int connect(socket_type s, const socket_addr_type* addr,
    std::size_t addrlen, error_code_type& ec)
{
  if (s == invalid_socket)
  {
    ec = detail::error::bad_descriptor;
    return socket_error_retval;
  }

  clear_last_error();
  int result = error_wrapper(call_connect(
        &msghdr::msg_namelen, s, addr, addrlen), ec);
  if (result == 0)
    ec = 0;
  else if (ec == detail::error::try_again)
    ec = detail::error::no_buffer_space;
  return result;
}

void sync_connect(socket_type s, const socket_addr_type* addr,
    std::size_t addrlen, error_code_type& ec)
{
  // Perform the connect operation.
  socket_ops::connect(s, addr, addrlen, ec);
  if (ec != detail::error::in_progress
      && ec != detail::error::would_block)
  {
    // The connect operation finished immediately.
    return;
  }

  // Wait for socket to become ready.
  if (socket_ops::poll_connect(s, ec) < 0)
    return;

  // Get the error code from the connect operation.
  int connect_error = 0;
  size_t connect_error_len = sizeof(connect_error);
  if (socket_ops::getsockopt(s, 0, SOL_SOCKET, SO_ERROR,
        &connect_error, &connect_error_len, ec) == socket_error_retval)
    return;

  // Return the result of the connect operation.
  ec = connect_error;
}

bool non_blocking_connect(socket_type s, error_code_type& ec)
{
  pollfd fds;
  fds.fd = s;
  fds.events = POLLOUT;
  fds.revents = 0;
  int ready = ::poll(&fds, 1, 0);

  if (ready == 0)
  {
    // The asynchronous connect operation is still in progress.
    return false;
  }

  // Get the error code from the connect operation.
  int connect_error = 0;
  size_t connect_error_len = sizeof(connect_error);
  if (socket_ops::getsockopt(s, 0, SOL_SOCKET, SO_ERROR,
        &connect_error, &connect_error_len, ec) == 0)
  {
    if (connect_error)
    {
      ec = connect_error;
    }
    else
      ec = 0;
  }

  return true;
}

size_t available(socket_type s, error_code_type& ec)
{
  if (s == invalid_socket)
  {
    return 0;
  }

  ioctl_arg_type value = 0;

  int result = error_wrapper(::ioctl(s, FIONREAD, &value), ec);
  if (result == 0)
    ec = 0;
  if (ec == ENOTTY)
    ec = detail::error::not_socket;
  return ec ? static_cast<size_t>(0) : static_cast<size_t>(value);
}

int listen(socket_type s, int backlog, error_code_type& ec)
{
  if (s == invalid_socket)
  {
    ec = detail::error::bad_descriptor;
    return socket_error_retval;
  }

  clear_last_error();
  int result = error_wrapper(::listen(s, backlog), ec);
  if (result == 0)
    ec = 0;
  return result;
}

inline void init_buf_iov_base(void*& base, void* addr)
{
  base = addr;
}

template <typename T>
inline void init_buf_iov_base(T& base, void* addr)
{
  base = static_cast<T>(addr);
}

typedef iovec buf;

void init_buf(buf& b, void* data, size_t size)
{
  init_buf_iov_base(b.iov_base, data);
  b.iov_len = size;
}

void init_buf(buf& b, const void* data, size_t size)
{
  init_buf_iov_base(b.iov_base, const_cast<void*>(data));
  b.iov_len = size;
}

inline void init_msghdr_msg_name(void*& name, socket_addr_type* addr)
{
  name = addr;
}

inline void init_msghdr_msg_name(void*& name, const socket_addr_type* addr)
{
  name = const_cast<socket_addr_type*>(addr);
}

template <typename T>
inline void init_msghdr_msg_name(T& name, socket_addr_type* addr)
{
  name = reinterpret_cast<T>(addr);
}

template <typename T>
inline void init_msghdr_msg_name(T& name, const socket_addr_type* addr)
{
  name = reinterpret_cast<T>(const_cast<socket_addr_type*>(addr));
}

signed_size_type recv(socket_type s, buf* bufs, size_t count,
    int flags, error_code_type& ec)
{
  clear_last_error();
  msghdr msg = msghdr();
  msg.msg_iov = bufs;
  msg.msg_iovlen = static_cast<int>(count);
  signed_size_type result = error_wrapper(::recvmsg(s, &msg, flags), ec);
  if (result >= 0)
    ec = 0;
  return result;
}

size_t sync_recv(socket_type s, state_type state, buf* bufs,
    size_t count, int flags, bool all_empty, error_code_type& ec)
{
  if (s == invalid_socket)
  {
    ec = detail::error::bad_descriptor;
    return 0;
  }

  // A request to read 0 bytes on a stream is a no-op.
  if (all_empty && (state & stream_oriented))
  {
    ec = 0;
    return 0;
  }

  // Read some data.
  for (;;)
  {
    // Try to complete the operation without blocking.
    signed_size_type bytes = socket_ops::recv(s, bufs, count, flags, ec);

    // Check if operation succeeded.
    if (bytes > 0)
      return bytes;

    // Check for EOF.
    if ((state & stream_oriented) && bytes == 0)
    {
      ec = detail::error::eof;
      return 0;
    }

    // Operation failed.
    if ((state & user_set_non_blocking)
        || (ec != detail::error::would_block
          && ec != detail::error::try_again))
      return 0;

    // Wait for socket to become ready.
    if (socket_ops::poll_read(s, 0, ec) < 0)
      return 0;
  }
}

bool non_blocking_recv(socket_type s,
    buf* bufs, size_t count, int flags, bool is_stream,
    error_code_type& ec, size_t& bytes_transferred)
{
  for (;;)
  {
    // Read some data.
    signed_size_type bytes = socket_ops::recv(s, bufs, count, flags, ec);

    // Check for end of stream.
    if (is_stream && bytes == 0)
    {
        ec = detail::error::eof;
        return true;
    }

    // Retry operation if interrupted by signal.
    if (ec == detail::error::interrupted)
        continue;

    // Check if we need to run the operation again.
    if (ec == detail::error::would_block
            || ec == detail::error::try_again)
        return false;

    // Operation is complete.
    if (bytes >= 0)
    {
        ec = 0;
        bytes_transferred = bytes;
    }
    else
        bytes_transferred = 0;

    return true;
  }
}

signed_size_type recvfrom(socket_type s, buf* bufs, size_t count,
                          int flags, socket_addr_type* addr, std::size_t* addrlen,
                          error_code_type& ec)
{
    clear_last_error();

    msghdr msg = msghdr();
    init_msghdr_msg_name(msg.msg_name, addr);
    msg.msg_namelen = static_cast<int>(*addrlen);
    msg.msg_iov = bufs;
    msg.msg_iovlen = static_cast<int>(count);
    signed_size_type result = error_wrapper(::recvmsg(s, &msg, flags), ec);
    *addrlen = msg.msg_namelen;
    if (result >= 0)
        ec = 0;
    return result;
}

size_t sync_recvfrom(socket_type s, state_type state, buf* bufs,
                     size_t count, int flags, socket_addr_type* addr,
                     std::size_t* addrlen, error_code_type& ec)
{
    if (s == invalid_socket)
    {
        ec = detail::error::bad_descriptor;
        return 0;
    }

    // Read some data.
    for (;;)
    {
        // Try to complete the operation without blocking.
        signed_size_type bytes = socket_ops::recvfrom(
                    s, bufs, count, flags, addr, addrlen, ec);

        // Check if operation succeeded.
        if (bytes >= 0)
            return bytes;

        // Operation failed.
        if ((state & user_set_non_blocking)
                || (ec != detail::error::would_block
                    && ec != detail::error::try_again))
            return 0;

        // Wait for socket to become ready.
        if (socket_ops::poll_read(s, 0, ec) < 0)
            return 0;
    }
}

bool non_blocking_recvfrom(socket_type s,
                           buf* bufs, size_t count, int flags,
                           socket_addr_type* addr, std::size_t* addrlen,
                           error_code_type& ec, size_t& bytes_transferred)
{
    for (;;)
    {
        // Read some data.
        signed_size_type bytes = socket_ops::recvfrom(
                    s, bufs, count, flags, addr, addrlen, ec);

        // Retry operation if interrupted by signal.
        if (ec == detail::error::interrupted)
            continue;

        // Check if we need to run the operation again.
        if (ec == detail::error::would_block
                || ec == detail::error::try_again)
            return false;

        // Operation is complete.
    if (bytes >= 0)
    {
      ec = 0;
      bytes_transferred = bytes;
    }
    else
      bytes_transferred = 0;

    return true;
  }
}


signed_size_type recvmsg(socket_type s, buf* bufs, size_t count,
    int in_flags, int& out_flags, error_code_type& ec)
{
  clear_last_error();
#if defined(BOOST_ASIO_WINDOWS) || defined(__CYGWIN__)
  out_flags = 0;
  return socket_ops::recv(s, bufs, count, in_flags, ec);
#else // defined(BOOST_ASIO_WINDOWS) || defined(__CYGWIN__)
  msghdr msg = msghdr();
  msg.msg_iov = bufs;
  msg.msg_iovlen = static_cast<int>(count);
  signed_size_type result = error_wrapper(::recvmsg(s, &msg, in_flags), ec);
  if (result >= 0)
  {
    ec = 0;
    out_flags = msg.msg_flags;
  }
  else
    out_flags = 0;
  return result;
#endif // defined(BOOST_ASIO_WINDOWS) || defined(__CYGWIN__)
}

size_t sync_recvmsg(socket_type s, state_type state,
    buf* bufs, size_t count, int in_flags, int& out_flags,
    error_code_type& ec)
{
  if (s == invalid_socket)
  {
    ec = detail::error::bad_descriptor;
    return 0;
  }

  // Read some data.
  for (;;)
  {
    // Try to complete the operation without blocking.
    signed_size_type bytes = socket_ops::recvmsg(
        s, bufs, count, in_flags, out_flags, ec);

    // Check if operation succeeded.
    if (bytes >= 0)
      return bytes;

    // Operation failed.
    if ((state & user_set_non_blocking)
        || (ec != detail::error::would_block
          && ec != detail::error::try_again))
      return 0;

    // Wait for socket to become ready.
    if (socket_ops::poll_read(s, 0, ec) < 0)
      return 0;
  }
}

bool non_blocking_recvmsg(socket_type s,
    buf* bufs, size_t count, int in_flags, int& out_flags,
    error_code_type& ec, size_t& bytes_transferred)
{
  for (;;)
  {
    // Read some data.
    signed_size_type bytes = socket_ops::recvmsg(
        s, bufs, count, in_flags, out_flags, ec);

    // Retry operation if interrupted by signal.
    if (ec == detail::error::interrupted)
      continue;

    // Check if we need to run the operation again.
    if (ec == detail::error::would_block
        || ec == detail::error::try_again)
      return false;

    // Operation is complete.
    if (bytes >= 0)
    {
      ec = 0;
      bytes_transferred = bytes;
    }
    else
      bytes_transferred = 0;

    return true;
  }
}

signed_size_type send(socket_type s, const buf* bufs, size_t count,
    int flags, error_code_type& ec)
{
  clear_last_error();

  msghdr msg = msghdr();
  msg.msg_iov = const_cast<buf*>(bufs);
  msg.msg_iovlen = static_cast<int>(count);
  flags |= MSG_NOSIGNAL;
  signed_size_type result = error_wrapper(::sendmsg(s, &msg, flags), ec);
  if (result >= 0)
    ec = 0;
  return result;
}

size_t sync_send(socket_type s, state_type state, const buf* bufs,
    size_t count, int flags, bool all_empty, error_code_type& ec)
{
  if (s == invalid_socket)
  {
    ec = detail::error::bad_descriptor;
    return 0;
  }

  // A request to write 0 bytes to a stream is a no-op.
  if (all_empty && (state & stream_oriented))
  {
    ec = 0;
    return 0;
  }

  // Read some data.
  for (;;)
  {
    // Try to complete the operation without blocking.
    signed_size_type bytes = socket_ops::send(s, bufs, count, flags, ec);

    // Check if operation succeeded.
    if (bytes >= 0)
      return bytes;

    // Operation failed.
    if ((state & user_set_non_blocking)
        || (ec != detail::error::would_block
          && ec != detail::error::try_again))
      return 0;

    // Wait for socket to become ready.
    if (socket_ops::poll_write(s, 0, ec) < 0)
      return 0;
  }
}

bool non_blocking_send(socket_type s,
    const buf* bufs, size_t count, int flags,
    error_code_type& ec, size_t& bytes_transferred)
{
  for (;;)
  {
    // Write some data.
    signed_size_type bytes = socket_ops::send(s, bufs, count, flags, ec);

    // Retry operation if interrupted by signal.
    if (ec == detail::error::interrupted)
      continue;

    // Check if we need to run the operation again.
    if (ec == detail::error::would_block
        || ec == detail::error::try_again)
      return false;

    // Operation is complete.
    if (bytes >= 0)
    {
      ec = 0;
      bytes_transferred = bytes;
    }
    else
      bytes_transferred = 0;

    return true;
  }
}

socket_type socket(int af, int type, int protocol,
    error_code_type& ec)
{
  clear_last_error();

  int s = error_wrapper(::socket(af, type, protocol), ec);
  if (s >= 0)
    ec = 0;
  return s;
}

template <typename SockLenType>
inline int call_setsockopt(SockLenType msghdr::*,
    socket_type s, int level, int optname,
    const void* optval, std::size_t optlen)
{
  return ::setsockopt(s, level, optname,
      (const char*)optval, (SockLenType)optlen);
}

int setsockopt(socket_type s, int level, int optname,
    const void* optval, std::size_t optlen, error_code_type& ec)
{
  if (s == invalid_socket)
  {
    ec = detail::error::bad_descriptor;
    return socket_error_retval;
  }

  if (level == custom_socket_option_level && optname == always_fail_option)
  {
    ec = detail::error::invalid_argument;
    return socket_error_retval;
  }

  if (level == custom_socket_option_level
      && optname == enable_connection_aborted_option)
  {
    if (optlen != sizeof(int))
    {
      ec = detail::error::invalid_argument;
      return socket_error_retval;
    }

    ec = 0;
    return 0;
  }

  clear_last_error();
  int result = error_wrapper(call_setsockopt(&msghdr::msg_namelen,
        s, level, optname, optval, optlen), ec);
  if (result == 0)
  {
    ec = 0;
  }

  return result;
}

template <typename SockLenType>
inline int call_getsockopt(SockLenType msghdr::*,
    socket_type s, int level, int optname,
    void* optval, std::size_t* optlen)
{
  SockLenType tmp_optlen = (SockLenType)*optlen;
  int result = ::getsockopt(s, level, optname, (char*)optval, &tmp_optlen);
  *optlen = (std::size_t)tmp_optlen;
  return result;
}

int getsockopt(socket_type s, state_type state, int level, int optname,
    void* optval, size_t* optlen, error_code_type& ec)
{
  if (s == invalid_socket)
  {
    ec = detail::error::bad_descriptor;
    return socket_error_retval;
  }

  if (level == custom_socket_option_level && optname == always_fail_option)
  {
    ec = detail::error::invalid_argument;
    return socket_error_retval;
  }

  if (level == custom_socket_option_level
      && optname == enable_connection_aborted_option)
  {
    if (*optlen != sizeof(int))
    {
      ec = detail::error::invalid_argument;
      return socket_error_retval;
    }

    *static_cast<int*>(optval) = (state & enable_connection_aborted) ? 1 : 0;
    ec = 0;
    return 0;
  }


  clear_last_error();
  int result = error_wrapper(call_getsockopt(&msghdr::msg_namelen,
        s, level, optname, optval, optlen), ec);
  if (result == 0 && level == SOL_SOCKET && *optlen == sizeof(int)
      && (optname == SO_SNDBUF || optname == SO_RCVBUF))
  {
    // On Linux, setting SO_SNDBUF or SO_RCVBUF to N actually causes the kernel
    // to set the buffer size to N*2. Linux puts additional stuff into the
    // buffers so that only about half is actually available to the application.
    // The retrieved value is divided by 2 here to make it appear as though the
    // correct value has been set.
    *static_cast<int*>(optval) /= 2;
  }
  if (result == 0)
    ec = 0;
  return result;
}

int ioctl(socket_type s, state_type& state, int cmd,
    ioctl_arg_type* arg, error_code_type& ec)
{
  if (s == invalid_socket)
  {
    ec = detail::error::bad_descriptor;
    return socket_error_retval;
  }

  clear_last_error();
  int result = error_wrapper(::ioctl(s, cmd, arg), ec);
  if (result >= 0)
  {
    ec = error_code_type();

    // When updating the non-blocking mode we always perform the ioctl syscall,
    // even if the flags would otherwise indicate that the socket is already in
    // the correct state. This ensures that the underlying socket is put into
    // the state that has been requested by the user. If the ioctl syscall was
    // successful then we need to update the flags to match.
    if (cmd == static_cast<int>(FIONBIO))
    {
      if (*arg)
      {
        state |= user_set_non_blocking;
      }
      else
      {
        // Clearing the non-blocking mode always overrides any internally-set
        // non-blocking flag. Any subsequent asynchronous operations will need
        // to re-enable non-blocking I/O.
        state &= ~(user_set_non_blocking | internal_non_blocking);
      }
    }
  }

  return result;
}

int select(int nfds, fd_set* readfds, fd_set* writefds,
    fd_set* exceptfds, timeval* timeout, error_code_type& ec)
{
  clear_last_error();

  int result = error_wrapper(::select(nfds, readfds,
        writefds, exceptfds, timeout), ec);
  if (result >= 0)
    ec = error_code_type();
  return result;
}

int poll_read(socket_type s, state_type state, error_code_type& ec)
{
  if (s == invalid_socket)
  {
    ec = detail::error::bad_descriptor;
    return socket_error_retval;
  }

  pollfd fds;
  fds.fd = s;
  fds.events = POLLIN;
  fds.revents = 0;
  int timeout = (state & user_set_non_blocking) ? 0 : -1;
  clear_last_error();
  int result = error_wrapper(::poll(&fds, 1, timeout), ec);

  if (result == 0)
    ec = (state & user_set_non_blocking)
      ? detail::error::would_block : error_code_type();
  else if (result > 0)
    ec = error_code_type();
  return result;
}

int poll_write(socket_type s, state_type state, error_code_type& ec)
{
  if (s == invalid_socket)
  {
    ec = detail::error::bad_descriptor;
    return socket_error_retval;
  }

  pollfd fds;
  fds.fd = s;
  fds.events = POLLOUT;
  fds.revents = 0;
  int timeout = (state & user_set_non_blocking) ? 0 : -1;
  clear_last_error();
  int result = error_wrapper(::poll(&fds, 1, timeout), ec);  if (result == 0)
    ec = (state & user_set_non_blocking)
      ? detail::error::would_block : error_code_type();
  else if (result > 0)
    ec = error_code_type();
  return result;
}

int poll_connect(socket_type s, error_code_type& ec)
{
  if (s == invalid_socket)
  {
    ec = detail::error::bad_descriptor;
    return socket_error_retval;
  }

  pollfd fds;
  fds.fd = s;
  fds.events = POLLOUT;
  fds.revents = 0;
  clear_last_error();
  int result = error_wrapper(::poll(&fds, 1, -1), ec);
  if (result >= 0)
    ec = error_code_type();
  return result;
}

} // namespace socket_ops
