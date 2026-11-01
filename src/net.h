#ifndef NET_H
#define NET_H
#include <stdint.h>
#include <stddef.h>
typedef intptr_t Socket;
#define NO_SOCKET ((Socket)-1)
int net_init(void);
void net_cleanup(void);
Socket net_listen(const char *address, int port);
Socket net_connect(const char *address, int port, int timeout_ms);
Socket net_accept(Socket listener);
int net_nonblocking(Socket socket);
/* recv/send: positive bytes, zero closed, -1 would-block/interrupted, -2 fatal. */
int net_read(Socket socket, char *buffer, size_t size);
int net_write(Socket socket, const char *buffer, size_t size);
void net_close(Socket socket);
/* Read/write readiness, each socket <= FD_SETSIZE. Return -1 on failure. */
int net_wait(Socket *sockets, const int *want_write, int count, int timeout_ms, int *readable, int *writable);
#endif
