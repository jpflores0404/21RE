#ifndef _WIN32
#define _POSIX_C_SOURCE 200809L
#endif
#include "net.h"
#include <limits.h>
#include <string.h>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#define NATIVE(s) ((SOCKET)(s))
typedef int SocketLength;
static int retry_error(void) { int e = WSAGetLastError(); return e == WSAEWOULDBLOCK || e == WSAEINTR || e == WSAEINPROGRESS; }
#else
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>
#define NATIVE(s) ((int)(s))
typedef socklen_t SocketLength;
static int retry_error(void) { return errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR || errno == EINPROGRESS; }
#endif

int net_init(void)
{
#ifdef _WIN32
    WSADATA data;
    return WSAStartup(MAKEWORD(2, 2), &data) == 0;
#else
    return 1;
#endif
}

void net_cleanup(void)
{
#ifdef _WIN32
    WSACleanup();
#endif
}

void net_close(Socket socket)
{
    if (socket == NO_SOCKET) return;
#ifdef _WIN32
    closesocket(NATIVE(socket));
#else
    close(NATIVE(socket));
#endif
}

int net_nonblocking(Socket socket)
{
#ifdef _WIN32
    u_long enabled = 1;
    return ioctlsocket(NATIVE(socket), FIONBIO, &enabled) == 0;
#else
    int flags = fcntl(NATIVE(socket), F_GETFL, 0);
    return flags >= 0 && fcntl(NATIVE(socket), F_SETFL, flags | O_NONBLOCK) == 0;
#endif
}

static int address_for(const char *address, int port, struct sockaddr_in *out)
{
    memset(out, 0, sizeof *out);
    if (port < 1 || port > 65535) return 0;
    out->sin_family = AF_INET;
    out->sin_port = htons((unsigned short)port);
    return inet_pton(AF_INET, address, &out->sin_addr) == 1;
}

Socket net_listen(const char *address, int port)
{
    struct sockaddr_in endpoint;
    Socket connection;
    if (!address_for(address, port, &endpoint)) return NO_SOCKET;
    connection = (Socket)socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (connection == NO_SOCKET) return NO_SOCKET;
#ifdef _WIN32
    {
        int enabled = 1;
        setsockopt(NATIVE(connection), SOL_SOCKET, SO_EXCLUSIVEADDRUSE, (const char *)&enabled, sizeof enabled);
    }
#else
    {
        int enabled = 1;
        setsockopt(NATIVE(connection), SOL_SOCKET, SO_REUSEADDR, &enabled, sizeof enabled);
    }
#endif
    if (bind(NATIVE(connection), (const struct sockaddr *)&endpoint, sizeof endpoint) != 0 ||
        listen(NATIVE(connection), 4) != 0 || !net_nonblocking(connection)) {
        net_close(connection);
        return NO_SOCKET;
    }
    return connection;
}

Socket net_accept(Socket listener)
{
    Socket accepted = (Socket)accept(NATIVE(listener), NULL, NULL);
    if (accepted != NO_SOCKET && !net_nonblocking(accepted)) { net_close(accepted); return NO_SOCKET; }
    return accepted;
}

Socket net_connect(const char *address, int port, int timeout_ms)
{
    struct sockaddr_in endpoint;
    Socket connection;
    int result;
    if (!address_for(address, port, &endpoint)) return NO_SOCKET;
    connection = (Socket)socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (connection == NO_SOCKET || !net_nonblocking(connection)) { net_close(connection); return NO_SOCKET; }
    result = connect(NATIVE(connection), (const struct sockaddr *)&endpoint, sizeof endpoint);
    if (result != 0) {
        int want = 1, readable = 0, writable = 0, error = 0;
        SocketLength size = (SocketLength)sizeof error;
        if (!retry_error() || net_wait(&connection, &want, 1, timeout_ms, &readable, &writable) < 0 || !writable ||
            getsockopt(NATIVE(connection), SOL_SOCKET, SO_ERROR, (char *)&error, &size) != 0 || error != 0) {
            net_close(connection); return NO_SOCKET;
        }
    }
    return connection;
}

int net_read(Socket socket, char *buffer, size_t size)
{
    int count;
    if (size == 0 || size > INT_MAX) return -2;
    count = (int)recv(NATIVE(socket), buffer, (int)size, 0);
    return count >= 0 ? count : (retry_error() ? -1 : -2);
}

int net_write(Socket socket, const char *buffer, size_t size)
{
    int count;
    if (size == 0 || size > INT_MAX) return -2;
#ifdef _WIN32
    count = send(NATIVE(socket), buffer, (int)size, 0);
#else
    count = (int)send(NATIVE(socket), buffer, size, MSG_NOSIGNAL);
#endif
    return count >= 0 ? count : (retry_error() ? -1 : -2);
}

int net_wait(Socket *sockets, const int *want_write, int count, int timeout_ms, int *readable, int *writable)
{
    fd_set reads, writes;
    struct timeval timeout;
    int i, largest = 0, result;
    if (count < 1 || count > FD_SETSIZE || timeout_ms < 0) return -1;
    FD_ZERO(&reads); FD_ZERO(&writes);
    for (i = 0; i < count; ++i) {
        readable[i] = writable[i] = 0;
        if (sockets[i] == NO_SOCKET) continue;
#ifndef _WIN32
        if (sockets[i] < 0 || sockets[i] >= FD_SETSIZE) return -1;
#endif
        FD_SET(NATIVE(sockets[i]), &reads);
        if (want_write[i]) FD_SET(NATIVE(sockets[i]), &writes);
#ifndef _WIN32
        if (NATIVE(sockets[i]) > largest) largest = NATIVE(sockets[i]);
#endif
    }
    timeout.tv_sec = timeout_ms / 1000;
    timeout.tv_usec = (timeout_ms % 1000) * 1000;
    result = select(largest + 1, &reads, &writes, NULL, &timeout);
    if (result < 0) return retry_error() ? 0 : -1;
    for (i = 0; i < count; ++i) if (sockets[i] != NO_SOCKET) {
        readable[i] = FD_ISSET(NATIVE(sockets[i]), &reads) != 0;
        writable[i] = FD_ISSET(NATIVE(sockets[i]), &writes) != 0;
    }
    return result;
}
