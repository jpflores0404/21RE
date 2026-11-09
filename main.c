#include "game.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>

static int request_number(const char *text, char **end, unsigned int *number)
{
    if (*text < '0' || *text > '9') return 0;
    errno = 0;
    unsigned long value = strtoul(text, end, 10);
    if (errno || value > UINT_MAX) return 0;
    *number = (unsigned int)value;
    return 1;
}

static void notice(const char *message)
{
    char screen[SCREEN_SIZE];
    snprintf(screen, sizeof screen, "%s\n\nEnter or Escape: main menu\n", message);
    terminal_draw(screen);
    while (!terminal_stopping()) {
        int key = terminal_key(20);
        if (key < 0 || key == KEY_ENTER || key == KEY_ESCAPE) return;
    }
}

static int ask_connection(int hosting, char address[TEXT_SIZE], unsigned short *port)
{
    char candidates[512] = "";
    if (hosting) local_addresses(candidates, sizeof candidates);
    for (int field = 0; field < 2; ++field) {
        char input[TEXT_SIZE] = "";
        size_t length = 0;
        const char *error = "";
        while (!terminal_stopping()) {
            char screen[1200];
            const char *title = field ? "TCP port" : hosting ? "Local IPv4 address to host on" : "Host IPv4 address";
            const char *hint = field ? "Enter the agreed port, 1-65535. Unused ports 1024-65535 are recommended."
                : hosting ? "Use an address assigned to this computer. Share it with your partner."
                : "Enter the exact address your partner shares.";
            snprintf(screen, sizeof screen, "2R1E | %s\n\n%s\n%s\n\n%s%s%s%s\n%s_\n\n%s\nEnter continues. Escape cancels.\n",
                     hosting ? "Host" : "Join", title, hint,
                     hosting && !field ? "Local IPv4 addresses:\n" : "",
                     hosting && !field ? (candidates[0] ? candidates : "  Could not list addresses. Check your network settings.\n") : "",
                     field ? "Selected host IPv4: " : "", field ? address : "", input, error);
            if (!terminal_draw(screen)) return 0;
            int key = terminal_key(20);
            if (key < 0 || key == KEY_ESCAPE) return 0;
            if (key == KEY_ENTER) {
                if (field ? parse_port(input, port) : valid_address(input)) {
                    if (!field) snprintf(address, TEXT_SIZE, "%s", input);
                    break;
                }
                error = field ? "Invalid port. Enter digits from 1 to 65535."
                              : "Invalid IPv4 address. Enter four octets from 0 to 255 for a unicast address.";
            } else if (key == KEY_BACKSPACE && length) {
                input[--length] = '\0';
            } else if (key >= 32 && key <= 126 && length < sizeof input - 1) {
                input[length++] = (char)key;
                input[length] = '\0';
            }
        }
        if (terminal_stopping()) return 0;
    }
    return 1;
}

static Socket wait_for_guest(const char *address, unsigned short port)
{
    Socket listener = socket_listen(address, port);
    if (listener == NO_SOCKET) { notice(net_error()); return NO_SOCKET; }
    char screen[512];
    snprintf(screen, sizeof screen,
             "2R1E\n\nWaiting for Player 2\n\nHost IPv4: %s\nTCP port: %u\n\n"
             "Tell your partner to enter this IP and port in Join.\n"
             "Use a LAN address for separate computers; loopback is local only.\n\nQ or Escape: cancel\n",
             address, (unsigned int)port);
    if (!terminal_draw(screen)) { socket_close(listener); return NO_SOCKET; }
    Socket guest = NO_SOCKET;
    while (!terminal_stopping()) {
        guest = socket_accept(listener);
        if (guest != NO_SOCKET) break;
        int key = terminal_key(20);
        if (key < 0 || key == 'q' || key == 'Q' || key == KEY_ESCAPE) break;
    }
    socket_close(listener);
    return guest;
}

static void sync_controls(Controls *controls, unsigned int *pending, int old_map, const View *view)
{
    if (old_map != view->map || view->finished) {
        *controls = (Controls){0};
        *pending = 0;
    }
    if (*pending && view->reply >= *pending) {
        if (view->prompt != PROMPT_NONE) {
            controls->mode = MODE_ANSWER;
            controls->prompt = view->prompt;
            controls->length = 0;
            controls->text[0] = '\0';
        }
        *pending = 0;
    }
}

static int usable_key(int key, const Controls *controls, int finished)
{
    int width, height;
    if (!terminal_size(&width, &height)) return -1;
    if (finished && key == KEY_ENTER) return key;
    if (width >= 80 && height >= 30) return key;
    if (key == KEY_ESCAPE || (controls->mode == MODE_PLAY && (key == 'q' || key == 'Q')))
        return key;
    return 0;
}

static void flush_and_close(Peer *peer)
{
    for (int turn = 0; turn < 50 && (peer->queued || peer->latest_size); ++turn) {
        if (!peer_pump(peer)) break;
        terminal_key(10);
    }
    peer_close(peer);
}

static void host_game(Socket socket)
{
    Game game;
    Peer peer;
    Controls controls = {0};
    View current = {0}, previous = {0}, guest = {0}, previous_guest = {0};
    unsigned int pending = 0;
    int connected = 1, dirty = 1, last_width = 0, last_height = 0;
    start_game(&game);
    peer_open(&peer, socket);
    while (!terminal_stopping()) {
        int old_map = current.map;
        make_view(&game, 0, &current);
        sync_controls(&controls, &pending, old_map, &current);
        make_view(&game, 1, &guest);
        if (connected && memcmp(&guest, &previous_guest, sizeof guest) != 0) {
            if (!peer_screen(&peer, &guest)) break;
            previous_guest = guest;
        }
        int width, height;
        if (!terminal_size(&width, &height)) break;
        if (dirty || memcmp(&current, &previous, sizeof current) != 0 ||
            width != last_width || height != last_height) {
            if (!show_view(&current, &controls)) break;
            previous = current;
            last_width = width;
            last_height = height;
            dirty = 0;
        }
        if (connected && !peer_pump(&peer)) {
            connected = 0;
            peer_close(&peer);
            if (!game.finished) {
                notice("Player 2 disconnected. Start a new Host and Join session.");
                break;
            }
        }
        char line[TEXT_SIZE];
        int received;
        while (connected && (received = peer_line(&peer, line)) != 0) {
            if (received < 0 || strcmp(line, "quit") == 0) {
                connected = 0;
                peer_close(&peer);
                break;
            }
            if (!game.finished && line[0] == '0' + game.map && line[1] == ' ') {
                char *end;
                unsigned int request;
                if (request_number(line + 2, &end, &request) && *end == ' ' &&
                    request > game.players[1].reply) {
                    apply_action(&game, 1, end + 1);
                    game.players[1].reply = request;
                }
            }
        }
        if (!connected && !game.finished) {
            notice("Player 2 left the session.");
            break;
        }
        int key = usable_key(terminal_key(10), &controls, game.finished);
        if (key < 0) break;
        if (game.finished) {
            if (current.finished && (key == KEY_ENTER || key == 'q' || key == 'Q')) break;
            continue;
        }
        char command[TEXT_SIZE];
        if (key == KEY_ESCAPE) pending = 0;
        int action = control_key(&controls, key, command);
        if (action < 0) break;
        if (action > 0) {
            pending = command[1] == '\0' && (command[0] == 'e' || command[0] == 'E');
            game.players[0].reply = 1;
            apply_action(&game, 0, command);
        }
        if (key) dirty = 1;
    }
    flush_and_close(&peer);
}

static void join_game(Socket socket)
{
    Peer peer;
    Controls controls = {0};
    View current = {0}, incoming = {0};
    unsigned int pending = 0, request = 0;
    int assembling = 0, connected = 1, dirty = 1;
    int last_width = 0, last_height = 0;
    size_t used = 0;
    peer_open(&peer, socket);
    while (!terminal_stopping()) {
        if (connected && !peer_pump(&peer)) {
            connected = 0;
            peer_close(&peer);
            if (!current.finished) {
                notice("Host disconnected. Start a new Host and Join session.");
                break;
            }
        }
        char line[TEXT_SIZE];
        int received;
        while (connected && (received = peer_line(&peer, line)) != 0) {
            if (received < 0) { connected = 0; break; }
            if (!assembling) {
                char *end;
                incoming = (View){0};
                if (strlen(line) < 12 || strncmp(line, "VIEW ", 5) ||
                    line[5] < '1' || line[5] > '3' || line[6] != ' ' ||
                    line[7] < '0' || line[7] > '4' || line[8] != ' ' ||
                    line[9] < '0' || line[9] > '1' || line[10] != ' ' ||
                    !request_number(line + 11, &end, &incoming.reply) || *end) {
                    connected = 0;
                    break;
                }
                incoming.map = line[5] - '0';
                incoming.prompt = line[7] - '0';
                incoming.finished = line[9] - '0';
                assembling = 1;
                used = 0;
            } else if (strcmp(line, ".") == 0) {
                int old_map = current.map;
                current = incoming;
                sync_controls(&controls, &pending, old_map, &current);
                assembling = 0;
                dirty = 1;
            } else {
                size_t length = strlen(line);
                if (used + length + 2 > sizeof incoming.screen) { connected = 0; break; }
                memcpy(incoming.screen + used, line, length);
                used += length;
                incoming.screen[used++] = '\n';
                incoming.screen[used] = '\0';
            }
        }
        if (!connected && !current.finished) {
            notice("The connection ended or sent an invalid screen.");
            break;
        }
        int width, height;
        if (!terminal_size(&width, &height)) break;
        if (dirty || width != last_width || height != last_height) {
            if (!show_view(&current, &controls)) break;
            last_width = width;
            last_height = height;
            dirty = 0;
        }
        int key = usable_key(terminal_key(10), &controls, current.finished);
        if (key < 0) break;
        if (current.finished) {
            if (key == KEY_ENTER || key == 'q' || key == 'Q') break;
            continue;
        }
        char command[TEXT_SIZE];
        if (key == KEY_ESCAPE) pending = 0;
        int action = control_key(&controls, key, command);
        if (action < 0) break;
        if (action > 0 && current.map) {
            char message[TEXT_SIZE + 24];
            if (request == UINT_MAX) break;
            ++request;
            snprintf(message, sizeof message, "%d %u %s\n", current.map, request, command);
            if (!peer_queue(&peer, message)) break;
            pending = command[1] == '\0' && (command[0] == 'e' || command[0] == 'E') ? request : 0;
        }
        if (key) dirty = 1;
    }
    flush_and_close(&peer);
}

int main(void)
{
    if (!terminal_open()) {
        terminal_close();
        fputs("Open 2R1E in an interactive terminal.\n", stderr);
        return 1;
    }
    int selected = 1, previous = 0;
    while (!terminal_stopping()) {
        if (selected != previous) {
            char menu[512];
            snprintf(menu, sizeof menu, "2R1E\n\n%c 1. Host game\n%c 2. Join game\n%c 3. Quit\n\n"
                     "Choose 1, 2, or 3, then Enter.\nHost chooses the local IP and TCP port. Join enters the same values.\n",
                     selected == 1 ? '>' : ' ', selected == 2 ? '>' : ' ', selected == 3 ? '>' : ' ');
            if (!terminal_draw(menu)) break;
            previous = selected;
        }
        int key = terminal_key(20);
        if (key < 0 || key == 'q' || key == 'Q') break;
        if (key >= '1' && key <= '3') selected = key - '0';
        if (key != KEY_ENTER) continue;
        if (selected == 3) break;
        char address[TEXT_SIZE];
        unsigned short port;
        if (ask_connection(selected == 1, address, &port)) {
            Socket socket = selected == 1 ? wait_for_guest(address, port) : socket_connect(address, port);
            if (socket != NO_SOCKET) {
                if (selected == 1) host_game(socket);
                else join_game(socket);
            } else if (selected == 2 && !terminal_stopping()) notice(net_error());
        }
        previous = 0;
    }
    terminal_close();
    return 0;
}
