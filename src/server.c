#ifndef _WIN32
#define _POSIX_C_SOURCE 200809L
#endif
#include "app.h"
#include "game.h"
#include "net.h"
#include "platform.h"
#include "protocol.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
static volatile LONG stop_flag;
static BOOL WINAPI server_stop(DWORD event)
{
    if (event != CTRL_C_EVENT && event != CTRL_BREAK_EVENT) return FALSE;
    InterlockedExchange(&stop_flag, 1); return TRUE;
}
static int should_stop(void) { return InterlockedCompareExchange(&stop_flag, 0, 0) != 0; }
#else
static volatile sig_atomic_t stop_flag;
static void server_stop(int event) { (void)event; stop_flag = 1; }
static int should_stop(void) { return stop_flag != 0; }
#endif

typedef struct {
    Socket socket;
    char input[ACTION_LIMIT + 1];
    size_t input_length;
    char output[OUTPUT_LIMIT];
    size_t output_length;
} Connection;

static int queue(Connection *connection, const cJSON *message)
{
    char frame[VIEW_LIMIT + 2];
    int count = protocol_print(message, frame, sizeof frame);
    if (!count || connection->output_length + (size_t)count > sizeof connection->output) return 0;
    memcpy(connection->output + connection->output_length, frame, (size_t)count);
    connection->output_length += (size_t)count;
    return 1;
}

static int notice(Connection *connection, const char *type, const char *message)
{
    cJSON *json = cJSON_CreateObject();
    int ok;
    if (json == NULL) return 0;
    if (!cJSON_AddStringToObject(json, "type", type) || !cJSON_AddStringToObject(json, "message", message)) {
        cJSON_Delete(json); return 0;
    }
    ok = queue(connection, json); cJSON_Delete(json); return ok;
}

static int views(Connection *connections, const Game *game)
{
    int player;
    for (player = 0; player < 2; ++player) if (connections[player].socket != NO_SOCKET) {
        cJSON *view = game_view(game, player);
        int ok = view != NULL && queue(&connections[player], view);
        cJSON_Delete(view);
        if (!ok) return 0;
    }
    return 1;
}

static void close_connection(Connection *connection)
{
    net_close(connection->socket);
    memset(connection, 0, sizeof *connection);
    connection->socket = NO_SOCKET;
}

static void end_session(Connection *connections, const char *message)
{
    int player;
    for (player = 0; player < 2; ++player) if (connections[player].socket != NO_SOCKET) {
        /* Preserve any already-started frame before the final notification. */
        if (!notice(&connections[player], "session_end", message)) close_connection(&connections[player]);
    }
}

static int consume(Connection *connections, int player, Game *game)
{
    Connection *connection = &connections[player];
    int turn;
    for (turn = 0; turn < 16; ++turn) {
        char *newline = memchr(connection->input, '\n', connection->input_length);
        size_t length;
        cJSON *action;
        const cJSON *type;
        if (newline == NULL) return connection->input_length <= ACTION_LIMIT ? 1 : 0;
        length = (size_t)(newline - connection->input);
        action = protocol_action(connection->input, length);
        memmove(connection->input, newline + 1, connection->input_length - length - 1);
        connection->input_length -= length + 1;
        if (action == NULL) {
            if (!notice(connection, "error", "Invalid or unsupported action; state unchanged.")) return 0;
            continue;
        }
        type = cJSON_GetObjectItemCaseSensitive(action, "type");
        if (strcmp(type->valuestring, "quit") == 0) { cJSON_Delete(action); return 0; }
        game_action(game, player, action);
        cJSON_Delete(action);
        if (!views(connections, game)) return 0;
    }
    return 1;
}

static void reject_third(Socket socket)
{
    static const char message[] = "{\"type\":\"error\",\"message\":\"Session already has two players.\"}\n";
    size_t offset = 0;
    uint64_t deadline = platform_millis() + 100;
    while (offset < sizeof message - 1 && platform_millis() < deadline) {
        int written = net_write(socket, message + offset, sizeof message - 1 - offset);
        if (written > 0) offset += (size_t)written;
        else if (written == -1) platform_sleep(2);
        else break;
    }
    net_close(socket);
}

int server_run(const char *address, int port, const char *session)
{
    Connection *connections = NULL;
    Socket listener = NO_SOCKET;
    Game game;
    uint64_t lobby_deadline, end_deadline = 0;
    int result = EXIT_SUCCESS, ending = 0, player;
    stop_flag = 0;
    if (session == NULL || strlen(session) > 63 || !net_init()) return EXIT_FAILURE;
#ifdef _WIN32
    SetConsoleCtrlHandler(server_stop, TRUE);
#else
    signal(SIGINT, server_stop); signal(SIGTERM, server_stop);
#endif
    listener = net_listen(address, port);
    if (listener == NO_SOCKET) { fputs("Cannot listen: invalid address, busy port, or socket failure.\n", stderr); result = EXIT_FAILURE; goto cleanup; }
    connections = calloc(2, sizeof *connections);
    if (connections == NULL) { result = EXIT_FAILURE; goto cleanup; }
    for (player = 0; player < 2; ++player) connections[player].socket = NO_SOCKET;
    game_init(&game);
    lobby_deadline = platform_millis() + 60000;
    while (1) {
        Socket sockets[3];
        int wants[3] = { 0, 0, 0 }, readable[3], writable[3], buffered = 0;
        if (!ending && (should_stop() || (!game.started && platform_millis() >= lobby_deadline))) {
            ending = 1; end_deadline = platform_millis() + 300;
            net_close(listener); listener = NO_SOCKET;
            end_session(connections, "Server stopped or lobby wait expired; host a new session.");
        }
        if (ending) {
            size_t pending = connections[0].output_length + connections[1].output_length;
            if (pending == 0 || platform_millis() >= end_deadline) break;
        }
        sockets[0] = listener;
        for (player = 0; player < 2; ++player) {
            sockets[player + 1] = connections[player].socket;
            wants[player + 1] = connections[player].output_length != 0;
            if (memchr(connections[player].input, '\n', connections[player].input_length)) buffered = 1;
        }
        if (net_wait(sockets, wants, 3, buffered ? 0 : 20, readable, writable) < 0) { result = EXIT_FAILURE; break; }
        if (!ending && listener != NO_SOCKET && readable[0]) {
            Socket accepted = net_accept(listener);
            if (accepted != NO_SOCKET) {
                int slot = connections[0].socket == NO_SOCKET ? 0 : (connections[1].socket == NO_SOCKET ? 1 : -1);
                if (slot < 0) reject_third(accepted);
                else {
                    cJSON *welcome = cJSON_CreateObject();
                    connections[slot].socket = accepted;
                    if (welcome == NULL || !cJSON_AddStringToObject(welcome, "type", "welcome") ||
                        !cJSON_AddNumberToObject(welcome, "player", slot + 1) ||
                        !cJSON_AddStringToObject(welcome, "session", session) || !queue(&connections[slot], welcome) || !views(connections, &game)) {
                        close_connection(&connections[slot]); result = EXIT_FAILURE;
                    }
                    cJSON_Delete(welcome);
                }
            }
        }
        for (player = 0; player < 2; ++player) {
            Connection *connection = &connections[player];
            int failed = 0;
            if (connection->socket == NO_SOCKET) continue;
            if (!ending && readable[player + 1]) {
                size_t available = sizeof connection->input - connection->input_length;
                int got = available == 0 ? -2 : net_read(connection->socket, connection->input + connection->input_length, available);
                if (got > 0) connection->input_length += (size_t)got;
                else if (got != -1) failed = 1;
            }
            if (!ending && !failed && !consume(connections, player, &game)) failed = 1;
            if (failed && !ending) {
                close_connection(connection);
                ending = 1; end_deadline = platform_millis() + 300;
                net_close(listener); listener = NO_SOCKET;
                end_session(connections, "A player left or exceeded protocol limits. Host a new session.");
            }
            if (connection->socket != NO_SOCKET && writable[player + 1] && connection->output_length) {
                int sent = net_write(connection->socket, connection->output, connection->output_length);
                if (sent > 0) {
                    connection->output_length -= (size_t)sent;
                    memmove(connection->output, connection->output + sent, connection->output_length);
                } else if (sent != -1) {
                    close_connection(connection);
                    if (!ending) {
                        ending = 1; end_deadline = platform_millis() + 300;
                        net_close(listener); listener = NO_SOCKET;
                        end_session(connections, "A connection failed. Host a new session.");
                    }
                }
            }
        }
    }
cleanup:
    net_close(listener);
    if (connections) { for (player = 0; player < 2; ++player) close_connection(&connections[player]); free(connections); }
#ifdef _WIN32
    SetConsoleCtrlHandler(server_stop, FALSE);
#endif
    net_cleanup();
    return result;
}
