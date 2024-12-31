#ifndef SOCKET_H
#define SOCKET_H

#include <libnet/message.h>

#include <openssl/err.h>
#include <openssl/ssl.h>

#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/// SSL socket.
///
/// The implementation is largely inspired by code from
/// https://github.com/darrenjs/openssl_examples with few minor differences. The
/// following diagram depicts the data flow:
///     ----------                 -------                  -----------
///     |        | ---> rbio ----> |     | ---> mrbio ----> |         |
///     | Socket |                 | SSL |                  | Program |
///     |        | <--- wbio <---- |     | <--- mwbio <---- |         |
///     ----------                 -------                  -----------
/// The m in mrbio and mwbio stands for message since they are the buffers where
/// we dequeue/enqueue messages from/to respectively.
///
/// One question may be why are we using so many memory BIO objects as
/// intermediate buffer. That is because the other way lies madness.
///
/// As with the common saying in computer science:
///     - Every problem can be solved with an additional level of indirection.
///
/// In OpenSSL land (at leaast if you are trying to do non-blocking io):
///     - Every problem can be solved by an additional intermediate buffer.
struct ssl_socket
{
  int fd;

  SSL *ssl;

  BIO *rbio;
  BIO *wbio;

  BIO *mrbio;
  BIO *mwbio;
};

/// Create a SSL socket from given file desriptor and ssl context.
///
/// The given file descriptor should be obtained from a call to accept() and set
/// to non-blocking mode by the caller.
static int ssl_socket_create(struct ssl_socket *socket, int fd, SSL_CTX *ssl_ctx)
{
  socket->fd = fd;

  if(!(socket->ssl = SSL_new(ssl_ctx)))
  {
    fprintf(stderr, "libnet: Error: Failed to create SSL socket: %s\n",  ERR_error_string(ERR_get_error(), NULL));
    goto err_close_fd;
  }

  if(!(socket->rbio = BIO_new(BIO_s_mem())))
  {
    fprintf(stderr, "libnet: Error: Failed to create BIO object for read buffer: %s\n",  ERR_error_string(ERR_get_error(), NULL));
    goto err_free_ssl;
  }

  if(!(socket->wbio = BIO_new(BIO_s_mem())))
  {
    fprintf(stderr, "libnet: Error: Failed to create BIO object for write buffer: %s\n",  ERR_error_string(ERR_get_error(), NULL));
    goto err_free_rbio;
  }

  if(!(socket->mrbio = BIO_new(BIO_s_mem())))
  {
    fprintf(stderr, "libnet: Error: Failed to create BIO object for message read buffer: %s\n",  ERR_error_string(ERR_get_error(), NULL));
    goto err_free_wbio;
  }

  if(!(socket->mwbio = BIO_new(BIO_s_mem())))
  {
    fprintf(stderr, "libnet: Error: Failed to create BIO object for message write buffer: %s\n",  ERR_error_string(ERR_get_error(), NULL));
    goto err_free_mrbio;
  }

  SSL_set_bio(socket->ssl, socket->rbio, socket->wbio);
  return 0;

err_free_mrbio:
  BIO_free(socket->mrbio);
err_free_wbio:
  BIO_free(socket->wbio);
err_free_rbio:
  BIO_free(socket->rbio);
err_free_ssl:
  SSL_free(socket->ssl);
err_close_fd:
  close(socket->fd);
  return -1;
}

/// Create a SSL server socket from given file desriptor and ssl context.
///
/// The given file descriptor should be obtained from a call to accept() and set
/// to non-blocking mode by the caller.
static int ssl_socket_accept(struct ssl_socket *socket, int fd, SSL_CTX *ssl_ctx)
{
  if(ssl_socket_create(socket, fd, ssl_ctx) != 0)
    return -1;

  SSL_set_accept_state(socket->ssl);
  return 0;
}

/// Create a SSL client socket from given file desriptor and ssl context.
///
/// The given file descriptor should be obtained from a call to connect() and set
/// to non-blocking mode by the caller.
static int ssl_socket_connect(struct ssl_socket *socket, int fd, SSL_CTX *ssl_ctx)
{
  if(ssl_socket_create(socket, fd, ssl_ctx) != 0)
    return -1;

  SSL_set_connect_state(socket->ssl);
  return 0;
}

/// Destroy a SSL socket.
static void ssl_socket_destroy(struct ssl_socket *socket)
{
  BIO_free(socket->mwbio);
  BIO_free(socket->mrbio);
  SSL_free(socket->ssl);
  close(socket->fd);
}

/// CHeck if an SSL socket have pending data to send.
static bool ssl_socket_want_send(struct ssl_socket *socket)
{
  BUF_MEM *mem;
  BIO_get_mem_ptr(socket->wbio, &mem);
  return mem->length != 0;
}

static int dispatch_socket_error(ssize_t n, const char *message)
{
  if(n == -1 && errno != EAGAIN && errno != EWOULDBLOCK)
  {
    fprintf(stderr, "libnet: Error: %s: %s\n", message, strerror(errno));
    return -1;
  }
  return 0;
}

/// Try to recv data from socket.
static int ssl_socket_try_recv(struct ssl_socket *socket, bool *connection_closed)
{
  char buf[4096];
  ssize_t n;
  while((n = recv(socket->fd, buf, sizeof buf, 0)) > 0)
    BIO_write(socket->rbio, buf, n);

  if(n == 0)
    *connection_closed = 1;

  return dispatch_socket_error(n, "Failed to recv data to socket");
}

/// Try to send data to socket.
static int ssl_socket_try_send(struct ssl_socket *socket)
{
  char *buf;
  size_t len = BIO_get_mem_data(socket->wbio, &buf);
  int pos = BIO_tell(socket->wbio);
  if(len == 0)
    return 0;

  ssize_t n;
  while(len > 0 && (n = send(socket->fd, buf, len, MSG_NOSIGNAL)) > 0)
  {
    buf += n;
    len -= n;
    pos += n;
  }

  BIO_seek(socket->wbio, pos);
  return dispatch_socket_error(n, "Failed to send data to socket");
}

static int dispatch_ssl_error(SSL *ssl, int n, const char *message)
{
  switch(SSL_get_error(ssl, n))
  {
  case SSL_ERROR_NONE:
  case SSL_ERROR_WANT_READ:
  case SSL_ERROR_WANT_WRITE:
    return 0;
  case SSL_ERROR_SYSCALL:
    fprintf(stderr, "libnet: Error: %s: System Error:%s\n", message, strerror(errno));
    return -1;
  case SSL_ERROR_SSL:
    fprintf(stderr, "libnet: Error: %s: SSL Error: %s\n", message, ERR_error_string(ERR_get_error(), NULL));
    return -1;
  default:
    fprintf(stderr, "libnet: Error: %s: Unknown error\n", message);
    return -1;
  }
}

/// Decrypt data to mrbio, by repeatedly calling SSL_read().
static int ssl_socket_try_decrypt(struct ssl_socket *socket)
{
  char buf[4096];
  int n;
  while((n = SSL_read(socket->ssl, buf, sizeof buf)) > 0)
    BIO_write(socket->mrbio, buf, n);

  return dispatch_ssl_error(socket->ssl, n, "Failed to decrypt SSL message");
}

/// Encrypt data from mwbio, by repeatedly calling SSL_write().
static int ssl_socket_try_encrypt(struct ssl_socket *socket)
{
  char *buf;
  size_t len = BIO_get_mem_data(socket->mwbio, &buf);
  int pos = BIO_tell(socket->mwbio);
  if(len == 0)
    return 0;

  int n;
  while(len > 0 && (n = SSL_write(socket->ssl, buf, len)) > 0)
  {
    buf += n;
    len -= n;
    pos += n;
  }

  BIO_seek(socket->mwbio, pos);
  return dispatch_ssl_error(socket->ssl, n,  "Failed to encrypt SSL message");
}

static void ssl_socket_enqueue_message(struct ssl_socket *socket, const struct libnet_message *message)
{
  const char *buf = (const char *)message;
  size_t n = sizeof(struct libnet_message) + message->size;
  BIO_write(socket->mwbio, buf, n);
}

struct ssl_socket_message_iter
{
  BIO *bio;

  char *buf;
  size_t len;
  int pos;
};

static struct ssl_socket_message_iter ssl_socket_message_iter_begin(struct ssl_socket *socket)
{
  struct ssl_socket_message_iter iter;
  iter.bio = socket->mrbio;
  iter.len = BIO_get_mem_data(iter.bio, &iter.buf);
  iter.pos = BIO_tell(iter.bio);
  return iter;
}

static const struct libnet_message *ssl_socket_message_iter_next(struct ssl_socket_message_iter *iter)
{
  if(iter->len < sizeof(struct libnet_message))
    goto end;

  const struct libnet_message *message = (const struct libnet_message *)iter->buf;
  if(iter->len < sizeof(struct libnet_message) + message->size)
    goto end;

  iter->buf += sizeof(struct libnet_message) + message->size;
  iter->len -= sizeof(struct libnet_message) + message->size;
  iter->pos += sizeof(struct libnet_message) + message->size;
  return message;

end:
  BIO_seek(iter->bio, iter->pos);
  return NULL;
}


#endif // SOCKET_H
