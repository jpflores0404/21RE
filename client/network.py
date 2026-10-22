"""Sending messages between the two players.

There are two kinds of "link", and the game uses both the same way:
    LocalLink  both players on one computer (no network needed)
    NetLink    two computers on the same Wi-Fi / LAN

Every message is a small dictionary, for example:
    {"type": "task", "id": "g_wires"}                  a task was finished
    {"type": "escaped", "player": 1}                   a player left through the exit
    {"type": "clue", "to": 2, "text": "..."}           a clue shared from the journal
    {"type": "chat", "from": 1, "text": "hello"}       a chat message
    {"type": "bye"}                                    the partner left the game

Over the network each message is sent as one line of JSON text.
"""
import json
import queue
import socket
import threading

PORT = 5050


class LocalLink:
    """Both players on one computer: anything sent is received straight back."""

    def __init__(self):
        self.pending = []

    def send(self, message):
        self.pending.append(message)

    def receive(self):
        messages = self.pending
        self.pending = []
        return messages

    def close(self):
        pass


class NetLink:
    """A connection to the other player's computer."""

    def __init__(self, sock):
        self.sock = sock
        self.inbox = queue.Queue()
        # Reading waits until data arrives, so it runs in a background thread.
        # That way the game screen never freezes while waiting for the partner.
        threading.Thread(target=self.read_loop, daemon=True).start()

    def read_loop(self):
        try:
            for line in self.sock.makefile("r", encoding="utf-8"):
                self.inbox.put(json.loads(line))
        except (OSError, ValueError):
            pass
        self.inbox.put({"type": "bye"})  # the connection has closed

    def send(self, message):
        try:
            self.sock.sendall((json.dumps(message) + "\n").encode("utf-8"))
        except OSError:
            self.inbox.put({"type": "bye"})

    def receive(self):
        """Return every message that arrived since the last call (may be empty)."""
        messages = []
        while not self.inbox.empty():
            messages.append(self.inbox.get())
        return messages

    def close(self):
        try:
            self.sock.shutdown(socket.SHUT_RDWR)  # tells the partner we left
        except OSError:
            pass
        self.sock.close()


def start_hosting(port=PORT):
    """Open a port and wait for the other player. Raises OSError if the port is busy."""
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server.bind(("", port))
    server.listen(1)
    server.settimeout(0.2)  # so the waiting screen can still react to keys
    return server


def check_for_partner(server):
    """Return a NetLink if the other player has connected, otherwise None."""
    try:
        sock, _ = server.accept()
    except socket.timeout:
        return None
    sock.settimeout(None)
    return NetLink(sock)


def join(ip, port=PORT):
    """Connect to the host. Raises OSError if it cannot connect."""
    sock = socket.create_connection((ip, port), timeout=5)
    sock.settimeout(None)
    return NetLink(sock)


def my_ip():
    """Best guess of this computer's LAN address, to tell the other player."""
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    try:
        s.connect(("8.8.8.8", 80))  # nothing is sent; this only picks the network card
        return s.getsockname()[0]
    except OSError:
        return "127.0.0.1"
    finally:
        s.close()
