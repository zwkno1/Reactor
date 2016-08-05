#ifndef ERROR_H
#define ERROR_H

#include <errno.h>
#include <netdb.h>

namespace detail {

namespace error {

enum basic_errors
{
  /// Permission denied.
  access_denied = EACCES,

  /// Address family not supported by protocol.
  address_family_not_supported = EAFNOSUPPORT,

  /// Address already in use.
  address_in_use = EADDRINUSE,

  /// Transport endpoint is already connected.
  already_connected = EISCONN,

  /// Operation already in progress.
  already_started = EALREADY,

  /// Broken pipe.
  broken_pipe = EPIPE,

  /// A connection has been aborted.
  connection_aborted = ECONNABORTED,

  /// Connection refused.
  connection_refused = ECONNREFUSED,

  /// Connection reset by peer.
  connection_reset = ECONNRESET,

  /// Bad file descriptor.
  bad_descriptor = EBADF,

  /// Bad address.
  fault = EFAULT,

  /// No route to host.
  host_unreachable = EHOSTUNREACH,

  /// Operation now in progress.
  in_progress = EINPROGRESS,

  /// Interrupted system call.
  interrupted = EINTR,

  /// Invalid argument.
  invalid_argument = EINVAL,

  /// Message too long.
  message_size = EMSGSIZE,

  /// The name was too long.
  name_too_long = ENAMETOOLONG,

  /// Network is down.
  network_down = ENETDOWN,

  /// Network dropped connection on reset.
  network_reset = ENETRESET,

  /// Network is unreachable.
  network_unreachable = ENETUNREACH,

  /// Too many open files.
  no_descriptors = EMFILE,

  /// No buffer space available.
  no_buffer_space = ENOBUFS,

  /// Cannot allocate memory.
  no_memory = ENOMEM,

  /// Operation not permitted.
  no_permission = EPERM,

  /// Protocol not available.
  no_protocol_option = ENOPROTOOPT,

  /// No such device.
  no_such_device = ENODEV,

  /// Transport endpoint is not connected.
  not_connected = ENOTCONN,

  /// Socket operation on non-socket.
  not_socket = ENOTSOCK,

  /// Operation cancelled.
  operation_aborted = ECANCELED,

  /// Operation not supported.
  operation_not_supported = EOPNOTSUPP,

  /// Cannot send after transport endpoint shutdown.
  shut_down = ESHUTDOWN,

  /// Connection timed out.
  timed_out = ETIMEDOUT,

  /// Resource temporarily unavailable.
  try_again = EAGAIN,

  /// The socket is marked non-blocking and the requested operation would block.
  would_block = EWOULDBLOCK
};

enum netdb_errors
{
  /// Host not found (authoritative.
  host_not_found = HOST_NOT_FOUND,

  /// Host not found (non-authoritative.
  host_not_found_try_again = TRY_AGAIN,

  /// The query is valid but does not have associated address data.
  no_data = NO_DATA,

  /// A non-recoverable error occurred.
  no_recovery = NO_RECOVERY
};

enum addrinfo_errors
{
  /// The service is not supported for the given socket type.
  service_not_found = EAI_SERVICE,

  /// The socket type is not supported.
  socket_type_not_supported = EAI_SOCKTYPE
};

enum misc_errors
{
  /// Already open.
  already_open = 1,

  /// End of file or stream.
  eof,

  /// Element not found.
  not_found,

  /// The descriptor cannot fit into the select system call's fd_set.
  fd_set_failure
};

} // namespace error

} // namespace detail

#endif // ERROR_H
