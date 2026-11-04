#include "demo.h"

#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#define NATIVE(s) ((SOCKET)(s))
#else
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>
#define NATIVE(s) ((int)(s))
#endif

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

void close_socket(Socket socket)
{
    if (socket == NO_SOCKET) return;
#ifdef _WIN32
    closesocket(NATIVE(socket));
#else
    close(NATIVE(socket));
#endif
}

/* A stalled peer should end the demo rather than freeze its partner. */
static int limit_send_wait(Socket socket)
{
#ifdef _WIN32
    DWORD timeout = 2000;
#else
    struct timeval timeout = {2, 0};
#endif
    return setsockopt(NATIVE(socket), SOL_SOCKET, SO_SNDTIMEO,
                      (const char *)&timeout, sizeof timeout) == 0;
}

Socket host_player(void)
{
    struct sockaddr_in address = {0};
    Socket listener = (Socket)socket(AF_INET, SOCK_STREAM, 0);
    Socket partner;
    if (listener == NO_SOCKET) return NO_SOCKET;

    address.sin_family = AF_INET;
    address.sin_port = htons(PORT);
    address.sin_addr.s_addr = htonl(INADDR_ANY);
#ifndef _WIN32
    /* Permit a new demo immediately after closing the previous one. */
    int reuse = 1;
    setsockopt(NATIVE(listener), SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof reuse);
#endif
    if (bind(NATIVE(listener), (struct sockaddr *)&address, sizeof address) != 0 ||
        listen(NATIVE(listener), 1) != 0) {
        close_socket(listener);
        return NO_SOCKET;
    }

    printf("Waiting for Player 2 on TCP port %d...\n", PORT);
    fflush(stdout);
    partner = (Socket)accept(NATIVE(listener), NULL, NULL);
    close_socket(listener);
    if (partner != NO_SOCKET && !limit_send_wait(partner)) {
        close_socket(partner);
        return NO_SOCKET;
    }
    return partner;
}

Socket join_host(const char *host)
{
    struct sockaddr_in address = {0};
    Socket partner;

    address.sin_family = AF_INET;
    address.sin_port = htons(PORT);
    if (inet_pton(AF_INET, host, &address.sin_addr) != 1) return NO_SOCKET;

    partner = (Socket)socket(AF_INET, SOCK_STREAM, 0);
    if (partner == NO_SOCKET) return NO_SOCKET;
    if (!limit_send_wait(partner) ||
        connect(NATIVE(partner), (struct sockaddr *)&address, sizeof address) != 0) {
        close_socket(partner);
        return NO_SOCKET;
    }
    return partner;
}

int send_text(Socket socket, const char *text)
{
    int left = (int)strlen(text);
    while (left > 0) {
#ifdef _WIN32
        int sent = send(NATIVE(socket), text, left, 0);
#else
        int sent = (int)send(NATIVE(socket), text, (size_t)left, MSG_NOSIGNAL);
#endif
        if (sent <= 0) return 0;
        text += sent;
        left -= sent;
    }
    return 1;
}

/* Return 1 for a line, 0 for no line yet, or -1 when the connection ends. */
int receive_line(Socket socket, Line *line)
{
    fd_set readers;
    struct timeval wait = {0, 20000};
    char byte;

#ifndef _WIN32
    if (socket >= FD_SETSIZE) return -1;
#endif
    FD_ZERO(&readers);
    FD_SET(NATIVE(socket), &readers);
#ifdef _WIN32
    int ready = select(0, &readers, NULL, NULL, &wait);
#else
    int ready = select(NATIVE(socket) + 1, &readers, NULL, NULL, &wait);
#endif
    if (ready <= 0) return ready;
    if (recv(NATIVE(socket), &byte, 1, 0) != 1) return -1;
    return add_character(line, (unsigned char)byte);
}
