"""Controlled two-PTY Linux UI check. Uses script(1), Python stdlib and optional WSL."""
import argparse
import os
from pathlib import Path
import re
import shlex
import socket
import subprocess
import threading
import time


class Terminal:
    def __init__(self, binary, wsl):
        self.wsl = wsl
        self.output = b""
        command = (
            "stty rows 32 cols 100; before=$(stty -g); "
            "printf '\\nPROBE_TTY=%s\\n' \"$(tty)\"; "
            + shlex.quote(binary)
            + "; code=$?; after=$(stty -g); "
            "if [ \"$before\" = \"$after\" ]; then echo PROBE_RESTORED; "
            "else echo PROBE_NOT_RESTORED; fi; exit $code"
        )
        args = self.command(["script", "-q", "-e", "-c", command, "/dev/null"])
        self.process = subprocess.Popen(
            args, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
            creationflags=subprocess.CREATE_NO_WINDOW if os.name == "nt" else 0,
        )
        self.reader = threading.Thread(target=self.read, daemon=True)
        self.reader.start()

    def command(self, args):
        return ["wsl", "-d", self.wsl, "--exec", *args] if self.wsl else args

    def read(self):
        while True:
            data = os.read(self.process.stdout.fileno(), 16384)
            if not data:
                return
            self.output = (self.output + data)[-262144:]

    def screen(self):
        return self.output.rsplit(b"\x1b[H\x1b[2J", 1)[-1].decode("ascii", errors="replace")

    def wait(self, predicate, label):
        deadline = time.monotonic() + 8
        while time.monotonic() < deadline:
            screen = self.screen()
            if predicate(screen):
                return screen
            if self.process.poll() is not None:
                break
            time.sleep(0.03)
        raise AssertionError(f"{label}:\n{self.screen()}")

    def text(self, expected):
        return self.wait(lambda screen: expected in screen, expected)

    def keys(self, keys):
        if isinstance(keys, str):
            keys = keys.encode("ascii")
        for key in re.findall(rb"\x1b\[[ABCD]|.", keys, re.DOTALL):
            self.process.stdin.write(key)
            self.process.stdin.flush()
            time.sleep(0.06)

    def resize(self, rows, columns):
        tty = re.search(rb"PROBE_TTY=(/dev/pts/[0-9]+)", self.output)
        assert tty, self.output[:1000]
        subprocess.run(self.command(["stty", "-F", tty[1].decode(), "rows", str(rows), "cols", str(columns)]),
                       check=True, capture_output=True, creationflags=subprocess.CREATE_NO_WINDOW if os.name == "nt" else 0)

    def finish(self):
        code = self.process.wait(timeout=8)
        self.reader.join(timeout=2)
        assert code == 0, (code, self.output[-2000:])
        assert b"PROBE_RESTORED" in self.output, self.output[-1500:]

    def close(self):
        if self.process.poll() is None:
            try:
                self.keys(b"\x1bq\nq")
                self.process.wait(timeout=5)
            except (OSError, subprocess.TimeoutExpired):
                self.process.terminate()
                self.process.wait(timeout=5)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("binary", type=Path)
    parser.add_argument("--wsl", metavar="DISTRO")
    args = parser.parse_args()
    binary = str(args.binary.resolve())
    if args.wsl:
        binary = "/mnt/host/" + binary[0].lower() + binary[2:].replace("\\", "/")
    with socket.socket() as reservation:
        reservation.bind(("127.0.0.1", 0))
        port = reservation.getsockname()[1]
    terminals = []
    checks = []
    try:
        host = Terminal(binary, args.wsl)
        terminals.append(host)
        host.text("> Host game")
        host.keys("ss\n")
        host.text("TUTORIAL")
        host.keys(b"\x1b")
        host.text("> Tutorial")
        host.keys(b"\x1b[A\x1b[A\n")
        host.text("Host bind IPv4")
        host.keys("\n")
        host.text("TCP port")
        host.keys(b"\x7f" * 4 + str(port).encode() + b"\n")
        host.text("Player 1")
        checks.append("menu, tutorial, Host and background server")

        join = Terminal(binary, args.wsl)
        terminals.append(join)
        join.text("> Host game")
        join.keys("s\n")
        join.text("Join host IPv4")
        join.keys("\n")
        join.text("TCP port")
        join.keys(b"\x7f" * 4 + str(port).encode() + b"\n")
        join.text("Player 2")
        host.text("Explore")
        checks.append("second actual UI Join and both-player ready")

        host.keys("d")
        host.wait(lambda screen: "#.@" in screen and "Facing East" in screen, "right move")
        host.keys("aw")
        host.wait(lambda screen: "#@." in screen and "Facing North" in screen, "wall collision/facing")
        host.keys("ddddse")
        host.text("Eye painting")
        host.keys("i")
        host.text("DISCOVERED NOTES")
        host.text("number 6")
        join.keys("tHello from terminal two\n")
        host.text("Hello from terminal two")
        assert "DISCOVERED NOTES" in host.screen()
        host.keys(b"\x1b")
        host.keys("tHello from terminal one\n")
        join.text("Hello from terminal one")
        checks.append("movement, collision, E inspection, I journal and bidirectional chat during journal")

        join.keys("dddddse")
        join.text("Answer:")
        join.keys("6 5 4\n")
        join.text("Try again")
        join.keys("e")
        join.text("Answer:")
        join.keys("1 2 3\n")
        join.text("Ordering note reconstructed")
        checks.append("word answer dialog, wrong-answer retry and correct submission")

        host.resize(15, 60)
        host.text("Enlarge terminal")
        join.keys("tStill connected while small\n")
        host.resize(32, 100)
        host.text("Still connected while small")
        checks.append("small-window warning, network pumping and resize recovery")

        host.keys("q")
        host.text("return to menu")
        join.text("return to menu")
        for terminal in terminals:
            terminal.keys("\nq")
            terminal.finish()
        if args.wsl:
            # WSL's Windows forwarding relay can briefly outlive the Linux listener.
            table = subprocess.check_output(host.command(["cat", "/proc/net/tcp"]),
                                            creationflags=subprocess.CREATE_NO_WINDOW)
            for row in table.decode().splitlines()[1:]:
                fields = row.split()
                assert not (fields[1].endswith(f":{port:04X}") and fields[3] == "0A"), "Linux server still listening"
        else:
            with socket.socket() as probe:
                probe.settimeout(1)
                assert probe.connect_ex(("127.0.0.1", port)) != 0, "Host server still listening"
        checks.append("Q disconnect notifications, owned-server cleanup and terminal-mode restoration")
        for check in checks:
            print("PASS:", check)
        print(f"Linux terminal checks: {len(checks)} groups passed")
    finally:
        for terminal in terminals:
            terminal.close()


if __name__ == "__main__":
    main()
