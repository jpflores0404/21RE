#include "game.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#define NATIVE(s) ((SOCKET)(s))
typedef int AddressSize;
#else
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <ifaddrs.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>
#define NATIVE(s) ((int)(s))
typedef socklen_t AddressSize;
#endif

static const char *last_error = "Check the address, port and other computer.";

const char *net_error(void)
{
    return last_error;
}

static void socket_error(void)
{
#ifdef _WIN32
    int error = WSAGetLastError();
    if (error == WSAEADDRINUSE) last_error = "That IP and port are already in use. Choose another port.";
    else if (error == WSAEADDRNOTAVAIL) last_error = "That IP is not assigned to this computer. Choose a local address.";
    else if (error == WSAEACCES) last_error = "Permission denied. Try an unused port from 1024 to 65535.";
    else if (error == WSAECONNREFUSED) last_error = "Connection refused. Check that Host is waiting on the same IP and port.";
#else
    int error = errno;
    if (error == EADDRINUSE) last_error = "That IP and port are already in use. Choose another port.";
    else if (error == EADDRNOTAVAIL) last_error = "That IP is not assigned to this computer. Choose a local address.";
    else if (error == EACCES) last_error = "Permission denied. Try an unused port from 1024 to 65535.";
    else if (error == ECONNREFUSED) last_error = "Connection refused. Check that Host is waiting on the same IP and port.";
#endif
    else last_error = "Connection failed. Check the selected IP, port, network and firewall.";
}

int valid_address(const char *text)
{
    struct in_addr address;
    if (!text || inet_pton(AF_INET, text, &address) != 1) return 0;
    unsigned int first = ntohl(address.s_addr) >> 24;
    return first > 0 && first < 224;
}

int parse_port(const char *text, unsigned short *port)
{
    unsigned int value = 0;
    if (!text || !port || !text[0]) return 0;
    for (size_t i = 0; text[i]; ++i) {
        if (i >= 5 || text[i] < '0' || text[i] > '9') return 0;
        value = value * 10 + (unsigned int)(text[i] - '0');
    }
    if (value == 0 || value > 65535) return 0;
    *port = (unsigned short)value;
    return 1;
}

static void list_address(char *text, size_t capacity, struct in_addr address)
{
    char ip[INET_ADDRSTRLEN];
    if (!inet_ntop(AF_INET, &address, ip, sizeof ip) || !valid_address(ip)) return;
    size_t used = strlen(text);
    snprintf(text + used, capacity - used, "  %s%s\n", ip,
             (ntohl(address.s_addr) >> 24) == 127 ? " (this computer only)" : "");
}

void local_addresses(char *text, size_t capacity)
{
    if (!text || capacity == 0) return;
    text[0] = '\0';
#ifdef _WIN32
    ULONG size = 0;
    if (GetIpAddrTable(NULL, &size, FALSE) != ERROR_INSUFFICIENT_BUFFER) return;
    MIB_IPADDRTABLE *table = malloc(size);
    if (!table) return;
    if (GetIpAddrTable(table, &size, FALSE) == NO_ERROR) {
        for (DWORD i = 0; i < table->dwNumEntries; ++i) {
            struct in_addr address;
            address.s_addr = table->table[i].dwAddr;
            list_address(text, capacity, address);
        }
    }
    free(table);
#else
    struct ifaddrs *addresses;
    if (getifaddrs(&addresses) != 0) return;
    for (struct ifaddrs *item = addresses; item; item = item->ifa_next) {
        if (item->ifa_addr && item->ifa_addr->sa_family == AF_INET)
            list_address(text, capacity, ((struct sockaddr_in *)item->ifa_addr)->sin_addr);
    }
    freeifaddrs(addresses);
#endif
}

static int retry_error(void)
{
#ifdef _WIN32
    int error = WSAGetLastError();
    return error == WSAEWOULDBLOCK || error == WSAEINPROGRESS || error == WSAEINTR;
#else
    return errno == EAGAIN || errno == EWOULDBLOCK || errno == EINPROGRESS || errno == EINTR;
#endif
}

int net_start(void)
{
#ifdef _WIN32
    WSADATA data;
    return WSAStartup(MAKEWORD(2, 2), &data) == 0;
#else
    return 1;
#endif
}

void net_stop(void)
{
#ifdef _WIN32
    WSACleanup();
#endif
}

void socket_close(Socket socket)
{
    if (socket == NO_SOCKET) return;
#ifdef _WIN32
    closesocket(NATIVE(socket));
#else
    close(NATIVE(socket));
#endif
}

static int nonblocking(Socket socket)
{
#ifdef _WIN32
    u_long enabled = 1;
    return ioctlsocket(NATIVE(socket), FIONBIO, &enabled) == 0;
#else
    int flags = fcntl(NATIVE(socket), F_GETFL, 0);
    return flags >= 0 && fcntl(NATIVE(socket), F_SETFL, flags | O_NONBLOCK) == 0;
#endif
}

Socket socket_listen(const char *host, unsigned short port)
{
    struct sockaddr_in address = {0};
    if (!valid_address(host) || !port) {
        last_error = "Enter a valid local IPv4 address and a port from 1 to 65535.";
        return NO_SOCKET;
    }
    Socket listener = (Socket)socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    int enabled = 1;
    if (listener == NO_SOCKET) { socket_error(); return NO_SOCKET; }
    address.sin_family = AF_INET;
    inet_pton(AF_INET, host, &address.sin_addr);
    address.sin_port = htons(port);
#ifdef _WIN32
    int option = SO_EXCLUSIVEADDRUSE;
#else
    int option = SO_REUSEADDR;
#endif
    if (setsockopt(NATIVE(listener), SOL_SOCKET, option, (const char *)&enabled, sizeof enabled) != 0 ||
        bind(NATIVE(listener), (struct sockaddr *)&address, sizeof address) != 0 ||
        listen(NATIVE(listener), 1) != 0 || !nonblocking(listener)) {
        socket_error();
        socket_close(listener);
        return NO_SOCKET;
    }
    return listener;
}

Socket socket_accept(Socket listener)
{
    Socket accepted = (Socket)accept(NATIVE(listener), NULL, NULL);
    if (accepted != NO_SOCKET && !nonblocking(accepted)) {
        socket_close(accepted);
        return NO_SOCKET;
    }
    return accepted;
}

Socket socket_connect(const char *host, unsigned short port)
{
    struct sockaddr_in address = {0};
    fd_set writable;
    struct timeval wait = {2, 0};
    int error = 0;
    AddressSize size = (AddressSize)sizeof error;
    Socket connection;

    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    if (!valid_address(host) || !port) {
        last_error = "Enter the host's IPv4 address and a port from 1 to 65535.";
        return NO_SOCKET;
    }
    inet_pton(AF_INET, host, &address.sin_addr);
    connection = (Socket)socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (connection == NO_SOCKET) { socket_error(); return NO_SOCKET; }
    if (!nonblocking(connection)) goto failed;
    if (connect(NATIVE(connection), (struct sockaddr *)&address, sizeof address) == 0) return connection;
    if (!retry_error()) goto failed;
#ifndef _WIN32
    if (connection < 0 || connection >= FD_SETSIZE) goto failed;
#endif
    FD_ZERO(&writable);
    FD_SET(NATIVE(connection), &writable);
#ifdef _WIN32
    int ready = select(0, NULL, &writable, NULL, &wait);
#else
    int ready = select(NATIVE(connection) + 1, NULL, &writable, NULL, &wait);
#endif
    if (ready > 0) {
        if (getsockopt(NATIVE(connection), SOL_SOCKET, SO_ERROR, (char *)&error, &size) == 0) {
            if (error == 0) return connection;
#ifdef _WIN32
            WSASetLastError(error);
#else
            errno = error;
#endif
        }
    } else if (ready == 0) {
        last_error = "Connection timed out. Check the host's IP, port, network and firewall.";
        socket_close(connection);
        return NO_SOCKET;
    }
failed:
    socket_error();
    socket_close(connection);
    return NO_SOCKET;
}

void peer_open(Peer *peer, Socket socket)
{
    memset(peer, 0, sizeof *peer);
    peer->socket = socket;
}

void peer_close(Peer *peer)
{
    socket_close(peer->socket);
    peer_open(peer, NO_SOCKET);
}

int peer_queue(Peer *peer, const char *text)
{
    if (text == NULL) return 0;
    size_t length = strlen(text);
    size_t remaining = peer->queued - peer->sent;
    if (length > sizeof peer->outgoing - remaining) return 0;
    memmove(peer->outgoing, peer->outgoing + peer->sent, remaining);
    memcpy(peer->outgoing + remaining, text, length);
    peer->queued = remaining + length;
    peer->sent = 0;
    return 1;
}

int peer_screen(Peer *peer, const View *view)
{
    char frame[SCREEN_SIZE + 64];
    int length = snprintf(frame, sizeof frame, "VIEW %d %d %d %u\n%s.\n",
                          view->map, view->prompt, view->finished, view->reply, view->screen);
    if (length < 0 || length >= (int)sizeof frame) return 0;

    /* Finish a started frame before switching to the newest screen. */
    if (peer->sent > 0 && peer->sent < peer->queued) {
        memcpy(peer->latest, frame, (size_t)length);
        peer->latest_size = (size_t)length;
    } else {
        memcpy(peer->outgoing, frame, (size_t)length);
        peer->queued = (size_t)length;
        peer->sent = 0;
        peer->latest_size = 0;
    }
    return 1;
}

int peer_pump(Peer *peer)
{
    if (peer->socket == NO_SOCKET) return 0;
    for (int turn = 0; turn < 4; ++turn) {
        if (peer->sent == peer->queued) {
            peer->sent = 0;
            peer->queued = peer->latest_size;
            memcpy(peer->outgoing, peer->latest, peer->latest_size);
            peer->latest_size = 0;
        }
        if (peer->queued == 0) break;
        int length = (int)(peer->queued - peer->sent);
#ifdef _WIN32
        int sent = send(NATIVE(peer->socket), peer->outgoing + peer->sent, length, 0);
#else
        int sent = (int)send(NATIVE(peer->socket), peer->outgoing + peer->sent, (size_t)length, MSG_NOSIGNAL);
#endif
        if (sent < 0 && retry_error()) break;
        if (sent <= 0) {
            socket_close(peer->socket);
            peer->socket = NO_SOCKET;
            return peer->received != 0;
        }
        peer->sent += (size_t)sent;
    }

    for (int turn = 0; turn < 4; ++turn) {
        size_t available = sizeof peer->incoming - peer->received;
        if (available == 0) break;
        int got = (int)recv(NATIVE(peer->socket), peer->incoming + peer->received, (int)available, 0);
        if (got < 0 && retry_error()) break;
        if (got <= 0) {
            socket_close(peer->socket);
            peer->socket = NO_SOCKET;
            return peer->received != 0;
        }
        peer->received += (size_t)got;
    }
    return 1;
}

int peer_line(Peer *peer, char line[TEXT_SIZE])
{
    if (line == NULL) return -1;
    char *newline = memchr(peer->incoming, '\n', peer->received);
    size_t length = newline == NULL ? peer->received : (size_t)(newline - peer->incoming);
    if (length >= TEXT_SIZE) return -1;
    for (size_t i = 0; i < length; ++i) {
        unsigned char byte = (unsigned char)peer->incoming[i];
        if (byte < 32 || byte > 126) return -1;
    }
    if (newline == NULL) return 0;
    memcpy(line, peer->incoming, length);
    line[length] = '\0';
    peer->received -= length + 1;
    memmove(peer->incoming, newline + 1, peer->received);
    return 1;
}
