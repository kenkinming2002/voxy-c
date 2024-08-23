#ifndef SOCKET_H
#define SOCKET_H

#include <libnet/message.h>

#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <stdlib.h>
#include <string.h>
#include <errno.h>

struct socket
{
  int fd;

  char *send_buf;
  size_t send_n;

  char *recv_buf;
  size_t recv_n;
};

/// Create a socket from a file desriptor.
///
/// This takes care of putting the socket into non-blocking mode and setting up
/// internal buffer.
static struct socket socket_create_from_fd(int fd)
{
  fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK);
  return (struct socket){
    .fd = fd,
    .send_buf = NULL,
    .send_n = 0,
    .recv_buf = NULL,
    .recv_n = 0,
  };
}

/// Destroy socket.
static void socket_destroy(struct socket socket)
{
  close(socket.fd);
  free(socket.send_buf);
  free(socket.recv_buf);
}

/// Append message to socket internal send buffer.
///
/// This function does not block.
static void socket_enqueue_message(struct socket *socket, const struct libnet_message *message)
{
  const char *buf = (const char *)message;
  size_t n = sizeof(struct libnet_message) + message->size;

  socket->send_buf = realloc(socket->send_buf, socket->send_n + n);
  memcpy(&socket->send_buf[socket->send_n], buf, n);
  socket->send_n += n;
}

/// Dequeue message from socket internal receive buffer.
///
/// The argument n keep track of the number of bytes currently consumed. This
/// function does not block.
static const struct libnet_message *socket_dequeue_message(struct socket *socket, size_t *n)
{
  if(socket->recv_n < (*n) + sizeof(struct libnet_message))
    return NULL;

  const struct libnet_message *message = (const struct libnet_message *)&socket->recv_buf[*n];
  if(socket->recv_n < (*n) + sizeof(struct libnet_message) + message->size)
    return NULL;

  *n += sizeof(struct libnet_message);
  *n += message->size;
  return message;
}

/// Called when all the messages in the socket internal receive buffer has been
/// dequeued.
///
/// This shrinks the socket internal receive buffer from the front to remove all
/// data that have been consumed.
static void socket_dequeue_message_end(struct socket *socket, size_t n)
{
  // Shrink the buffer from the front. Is it possible for an malloc
  // implementation to allow shrinking an allocated chunk in the front
  // without copying, by simply splitting it off and marking it as free?
  // Either way, there is no API in C that would allow this.
  if(n != 0)
  {
    memmove(&socket->recv_buf[0], &socket->recv_buf[n], socket->recv_n - n);
    socket->recv_n -= n;
    socket->recv_buf = realloc(socket->recv_buf, socket->recv_n);
  }
}

/// Try to send data from the socket internal send buffer.
///
/// This function does not block. Upon success, 0 is returned. Otherwise, -1 is
/// returned and errno is set to indicate the error.
static int socket_update_try_send(struct socket *socket)
{
  if(socket->send_n == 0)
    return 0;

  char *buf = socket->send_buf;
  size_t n = socket->send_n;
  while(n > 0)
  {
    ssize_t result;
    if((result = send(socket->fd, buf, n, MSG_NOSIGNAL)) == -1)
      break;

    buf += result;
    n -= result;
  }

  memmove(&socket->send_buf[0], &socket->send_buf[socket->send_n - n], n);
  socket->send_n = n;
  socket->send_buf = realloc(socket->send_buf, socket->send_n);

  if(n != 0 && errno != EAGAIN && errno != EWOULDBLOCK)
    return -1;

  return 0;
}

/// Try to recv data to the socket internal recv buffer.
///
/// This function does not block. Upon success, 0 is returned. Otherwise, -1 is
/// returned and errno is set to indicate the error.
static int socket_update_try_recv(struct socket *socket, int *connection_closed)
{
  char buf[4096];
  ssize_t n;
  while((n = recv(socket->fd, buf, sizeof buf, 0)) > 0)
  {
    size_t old_n = socket->recv_n;
    socket->recv_n += n;
    socket->recv_buf = realloc(socket->recv_buf, socket->recv_n);
    memcpy(&socket->recv_buf[old_n], buf, n);
  }

  if(n == 0)
    *connection_closed = 1;

  if(n == -1 && errno != EAGAIN && errno != EWOULDBLOCK)
    return -1;

  return 0;
}

#endif // SOCKET_H
