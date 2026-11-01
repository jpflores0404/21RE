"""Exercise the real authoritative server with two socket clients. Standard library only."""
import collections
import json
import os
from pathlib import Path
import socket
import subprocess
import sys
import time

USE_WSL = False


def server_command(binary, port, session):
    if USE_WSL:
        windows = str(binary)
        linux = "/mnt/host/" + windows[0].lower() + windows[2:].replace("\\", "/")
        return ["wsl", "-d", "docker-desktop", "--", linux, "--server", "127.0.0.1", str(port), session]
    return [str(binary), "--server", "127.0.0.1", str(port), session]


class Peer:
    def __init__(self, port):
        self.socket = socket.create_connection(("127.0.0.1", port), timeout=3)
        self.socket.settimeout(3)
        self.buffer = b""
        self.view = None
        welcome = self.next("welcome")
        assert welcome["session"] == "integration", welcome
        self.player = welcome["player"]

    def next(self, kind):
        while True:
            while b"\n" not in self.buffer:
                data = self.socket.recv(16384)
                if not data:
                    raise AssertionError("Unexpected server EOF")
                self.buffer += data
            line, self.buffer = self.buffer.split(b"\n", 1)
            message = json.loads(line)
            if message.get("type") == kind or isinstance(kind, tuple) and message.get("type") in kind:
                return message
            assert message.get("type") not in ("error", "session_end"), message

    def send(self, action):
        self.socket.sendall(json.dumps(action, separators=(",", ":")).encode() + b"\n")

    def close(self):
        self.socket.close()


def start_server(binary):
    with socket.socket() as reservation:
        reservation.bind(("127.0.0.1", 0))
        port = reservation.getsockname()[1]
    flags = subprocess.CREATE_NO_WINDOW if os.name == "nt" else 0
    process = subprocess.Popen(server_command(binary, port, "integration"),
                               stdout=subprocess.PIPE, stderr=subprocess.PIPE, creationflags=flags)
    deadline = time.monotonic() + 5
    while time.monotonic() < deadline:
        if process.poll() is not None:
            raise AssertionError(process.communicate())
        try:
            first = Peer(port)
            return process, port, first
        except (ConnectionRefusedError, TimeoutError):
            time.sleep(0.03)
    process.terminate()
    raise AssertionError("Server did not become ready")


def update(peers):
    for peer in peers:
        peer.view = peer.next("view")


def act(peers, player, kind, **fields):
    peer = peers[player]
    action = {"type": kind}
    if kind not in ("ready", "quit", "chat"):
        action["map"] = peer.view["map"]
    action.update(fields)
    peer.send(action)
    update(peers)


def approach(peers, player, symbol):
    """Find a walkable path to face one actual visible object, then use real moves."""
    peer = peers[player]
    rows = peer.view["rows"]
    positions = [(x, y) for y, row in enumerate(rows) for x, tile in enumerate(row) if tile == symbol]
    assert len(positions) == 1, (symbol, positions, rows)
    target_x, target_y = positions[0]
    directions = [(0, -1, "up"), (1, 0, "right"), (0, 1, "down"), (-1, 0, "left")]
    start = (peer.view["x"], peer.view["y"])
    queue = collections.deque([start])
    paths = {start: []}
    chosen = None
    while queue:
        x, y = queue.popleft()
        for dx, dy, direction in directions:
            if (x + dx, y + dy) == (target_x, target_y):
                chosen = paths[(x, y)] + [direction]
                break
        if chosen is not None:
            break
        for dx, dy, direction in directions:
            nx, ny = x + dx, y + dy
            if 0 <= ny < len(rows) and 0 <= nx < len(rows[ny]) and rows[ny][nx] in ".@* " and (nx, ny) not in paths:
                paths[(nx, ny)] = paths[(x, y)] + [direction]
                queue.append((nx, ny))
    assert chosen is not None, (symbol, "unreachable")
    for direction in chosen:
        act(peers, player, "move", direction=direction)
    act(peers, player, "interact")


def campaign(binary):
    process, port, first = start_server(binary)
    second = None
    try:
        second = Peer(port)
        peers = [first, second]
        first.send({"type": "ready"})
        second.send({"type": "ready"})
        for peer in peers:
            while True:
                peer.view = peer.next("view")
                if peer.view["started"]:
                    break
        assert [peer.player for peer in peers] == [1, 2]
        flags = subprocess.CREATE_NO_WINDOW if os.name == "nt" else 0
        busy = subprocess.run(server_command(binary, port, "busy"),
                              capture_output=True, timeout=3, creationflags=flags)
        assert busy.returncode != 0, "Busy port was not rejected"
        third = socket.create_connection(("127.0.0.1", port), timeout=3)
        third.settimeout(3)
        assert json.loads(third.makefile("rb").readline())["type"] == "error"
        third.close()

        for malformed in [b'[]\n', b'{"type":"ready","player":2}\n',
                          b'{"type":"victory"}\n', b'{"type":"ready","type":"quit"}\n']:
            first.socket.sendall(malformed)
            assert first.next("error")["message"]
        before = (first.view["x"], first.view["y"])
        first.send({"type": "move", "map": 3, "direction": "right"})
        update(peers)
        assert (first.view["x"], first.view["y"]) == before

        fragmented = b'{"type":"chat","text":"fragmented hello"}\n'
        first.socket.sendall(fragmented[:7])
        time.sleep(0.04)
        first.socket.sendall(fragmented[7:])
        update(peers)
        assert "fragmented hello" in second.view["chat"]
        first.socket.sendall(b'{"type":"chat","text":"one"}\n{"type":"chat","text":"two"}\n')
        update(peers)
        update(peers)
        assert "two" in second.view["chat"]

        approach(peers, 0, "A")
        assert "6" in " ".join(first.view["notes"])
        assert "6" not in " ".join(second.view["notes"])
        approach(peers, 0, "B")
        approach(peers, 0, "C")
        approach(peers, 0, "D")
        approach(peers, 1, "N")
        act(peers, 1, "submit", object="order_note", answer="1 2 3")
        approach(peers, 1, "K")
        act(peers, 1, "submit", object="cabinet", answer="000")
        assert first.view["map"] == 1
        act(peers, 1, "submit", object="cabinet", answer="246")
        assert "Z" in " ".join(second.view["notes"])
        assert "Z-X-Y" not in " ".join(first.view["notes"])
        approach(peers, 0, "H")
        act(peers, 0, "submit", object="hatch", answer="1259")
        act(peers, 0, "submit", object="hatch", answer="259")
        approach(peers, 0, "E")
        assert first.view["map"] == 1
        approach(peers, 1, "E")
        assert all(peer.view["map"] == 2 for peer in peers)

        approach(peers, 1, "B")
        approach(peers, 0, "B")
        assert first.view["rows"][5][20] == "."
        approach(peers, 0, "R")
        approach(peers, 1, "D")
        for dial, value in enumerate([2, 3, 4]):
            act(peers, 1, "dial", dial=dial, value=value)
        approach(peers, 1, "T")
        act(peers, 1, "test")
        approach(peers, 1, "E")
        assert second.view["map"] == 2
        approach(peers, 0, "E")
        assert all(peer.view["map"] == 3 and peer.view["milestone_complete"] for peer in peers)
        assert all("victory" not in peer.view for peer in peers)
        approach(peers, 0, "E")
        assert first.view["map"] == 3
        first.send({"type": "quit"})
        assert second.next("session_end")["message"]
        process.wait(timeout=3)
        assert process.returncode == 0
        print("Real two-client campaign, private notes, framing, invalid input, busy port, third join and quit checks passed.")
    finally:
        first.close()
        if second:
            second.close()
        if process.poll() is None:
            process.terminate()
            process.wait(timeout=3)


def lifecycle_faults(binary):
    for fault in ("disconnect", "oversized"):
        process, port, first = start_server(binary)
        second = None
        try:
            second = Peer(port)
            if fault == "disconnect":
                first.close()
            else:
                first.socket.sendall(b"x" * 1025)
            assert second.next("session_end")["message"]
            process.wait(timeout=3)
            assert process.returncode == 0
        finally:
            first.close()
            if second:
                second.close()
            if process.poll() is None:
                process.terminate()
                process.wait(timeout=3)
    print("Abrupt disconnect and oversized incomplete line checks passed.")


def slow_peer(binary):
    """Keep one peer responsive while the other stops draining its socket."""
    process, port, first = start_server(binary)
    second = None
    try:
        second = Peer(port)
        second.socket.setsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF, 1024)
        first.send({"type": "ready"})
        second.send({"type": "ready"})
        for peer in (first, second):
            while not peer.next("view")["started"]:
                pass
        deadline = time.monotonic() + 20
        for _ in range(10000):
            first.send({"type": "ready"})
            try:
                response = first.next(("view", "session_end"))
            except AssertionError as error:
                # Capacity failure may close the initiating connection before notification.
                if str(error) != "Unexpected server EOF":
                    raise
                break
            if response["type"] == "session_end":
                break
            assert time.monotonic() < deadline, "Slow peer blocked the referee"
        else:
            raise AssertionError("Slow peer never reached the bounded output limit")
        process.wait(timeout=3)
        assert process.returncode == 0
        print("Slow-reader output capacity and responsive-peer termination checks passed.")
    finally:
        first.close()
        if second:
            second.close()
        if process.poll() is None:
            process.terminate()
            process.wait(timeout=3)


if __name__ == "__main__":
    if len(sys.argv) == 3 and sys.argv[1] == "--wsl":
        USE_WSL = True
        binary_path = sys.argv[2]
    elif len(sys.argv) == 2:
        binary_path = sys.argv[1]
    else:
        raise SystemExit("Usage: python tests/integration_campaign.py [--wsl] PATH_TO_EXECUTABLE")
    executable = Path(binary_path).resolve()
    campaign(executable)
    lifecycle_faults(executable)
    slow_peer(executable)
