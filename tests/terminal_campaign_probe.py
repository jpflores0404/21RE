"""Controlled native Windows console probe, separate from human acceptance.

The coordinator creates a hidden console worker. Windows Application Control
failures are reported as blocked rather than bypassed or counted as passes.
"""
import ctypes as c
from ctypes import wintypes as w
import json
import os
from pathlib import Path
import signal
import socket
import subprocess
import sys
import time


class Coord(c.Structure):
    _fields_ = [("x", w.SHORT), ("y", w.SHORT)]


class Rect(c.Structure):
    _fields_ = [("left", w.SHORT), ("top", w.SHORT), ("right", w.SHORT), ("bottom", w.SHORT)]


class BufferInfo(c.Structure):
    _fields_ = [("size", Coord), ("cursor", Coord), ("attributes", w.WORD),
                ("window", Rect), ("maximum", Coord)]


class Character(c.Union):
    _fields_ = [("unicode", w.WCHAR), ("ascii", c.c_char)]


class KeyEvent(c.Structure):
    _fields_ = [("down", w.BOOL), ("repeat", w.WORD), ("virtual", w.WORD),
                ("scan", w.WORD), ("character", Character), ("controls", w.DWORD)]


class Event(c.Union):
    _fields_ = [("key", KeyEvent), ("padding", c.c_byte * 16)]


class InputRecord(c.Structure):
    _fields_ = [("kind", w.WORD), ("event", Event)]


class Console:
    def __init__(self):
        self.kernel = c.WinDLL("kernel32", use_last_error=True)
        k = self.kernel
        k.GetConsoleWindow.restype = w.HWND
        k.CreateFileW.restype = w.HANDLE
        k.CreateFileW.argtypes = [w.LPCWSTR, w.DWORD, w.DWORD, w.LPVOID, w.DWORD, w.DWORD, w.HANDLE]
        k.GetConsoleMode.argtypes = [w.HANDLE, c.POINTER(w.DWORD)]
        k.CloseHandle.argtypes = [w.HANDLE]
        k.GetConsoleScreenBufferInfo.argtypes = [w.HANDLE, c.POINTER(BufferInfo)]
        k.ReadConsoleOutputCharacterW.argtypes = [w.HANDLE, w.LPWSTR, w.DWORD, Coord, c.POINTER(w.DWORD)]
        k.WriteConsoleInputW.argtypes = [w.HANDLE, c.POINTER(InputRecord), w.DWORD, c.POINTER(w.DWORD)]
        if not k.GetConsoleWindow():
            assert k.AllocConsole(), c.get_last_error()
        user = c.WinDLL("user32", use_last_error=True)
        user.ShowWindow.argtypes = [w.HWND, c.c_int]
        user.ShowWindow(k.GetConsoleWindow(), 0)
        self.input = self.open_handle("CONIN$")
        self.original_mode = self.mode()

    def open_handle(self, name):
        handle = self.kernel.CreateFileW(name, 0xC0000000, 3, None, 3, 0, None)
        assert handle != c.c_void_p(-1).value, c.get_last_error()
        return handle

    def mode(self):
        result = w.DWORD()
        assert self.kernel.GetConsoleMode(self.input, c.byref(result)), c.get_last_error()
        return result.value

    def lines(self):
        handle = self.open_handle("CONOUT$")
        try:
            info = BufferInfo()
            assert self.kernel.GetConsoleScreenBufferInfo(handle, c.byref(info)), c.get_last_error()
            width = info.window.right - info.window.left + 1
            lines = []
            for y in range(info.window.top, info.window.bottom + 1):
                text = c.create_unicode_buffer(width + 1)
                count = w.DWORD()
                assert self.kernel.ReadConsoleOutputCharacterW(handle, text, width,
                    Coord(info.window.left, y), c.byref(count)), c.get_last_error()
                lines.append(text.value)
            return lines
        finally:
            self.kernel.CloseHandle(handle)

    def wait(self, predicate):
        deadline = time.monotonic() + 5
        while time.monotonic() < deadline:
            lines = self.lines()
            if predicate(lines):
                return lines
            time.sleep(0.04)
        raise AssertionError("Console condition timed out: " + repr(lines[:8]))

    def text(self, text):
        return self.wait(lambda lines: text in "\n".join(lines))

    def keys(self, text):
        records = (InputRecord * len(text))()
        for record, character in zip(records, text):
            record.kind = 1
            record.event.key.down = 1
            record.event.key.repeat = 1
            record.event.key.virtual = ord(character.upper())
            record.event.key.character.unicode = character
        written = w.DWORD()
        assert self.kernel.WriteConsoleInputW(self.input, records, len(text), c.byref(written))
        assert written.value == len(text)

    def close(self):
        self.kernel.CloseHandle(self.input)
        self.kernel.FreeConsole()


class Peer:
    def __init__(self, port):
        self.socket = socket.create_connection(("127.0.0.1", port), timeout=3)
        self.socket.settimeout(3)
        self.buffer = b""

    def next(self, predicate):
        while True:
            while b"\n" not in self.buffer:
                part = self.socket.recv(16384)
                assert part, "Unexpected server EOF"
                self.buffer += part
            line, self.buffer = self.buffer.split(b"\n", 1)
            value = json.loads(line)
            if predicate(value):
                return value

    def send(self, value):
        self.socket.sendall(json.dumps(value).encode() + b"\n")


def marker(lines):
    for y, line in enumerate(lines):
        if "@" in line:
            return line.index("@"), y
    return None


def worker(binary, result_path):
    console, process, peer = None, None, None
    checks = []
    result = {"status": "failed", "checks": checks}
    try:
        console = Console()
        with open("CONIN$", "rb", buffering=0) as input_file, open("CONOUT$", "wb", buffering=0) as output_file:
            process = subprocess.Popen([str(binary)], stdin=input_file, stdout=output_file, stderr=output_file)
            console.text("TWO ROOMS ONE EXIT")
            console.keys("ss\r")
            console.text("TUTORIAL")
            console.keys("\x1bww\r")
            console.text("Host bind IPv4")
            console.keys("\r")
            console.text("TCP port")
            with socket.socket() as reservation:
                reservation.bind(("127.0.0.1", 0))
                port = reservation.getsockname()[1]
            console.keys("\b" * 8 + str(port) + "\r")
            console.text("Gallery")
            peer = Peer(port)
            welcome = peer.next(lambda m: m.get("type") == "welcome")
            assert welcome["player"] == 2
            peer.send({"type": "ready"})
            peer.next(lambda m: m.get("type") == "view" and m.get("started"))
            start = marker(console.wait(lambda lines: marker(lines) is not None))
            console.keys("d")
            console.wait(lambda lines: marker(lines) == (start[0] + 1, start[1]))
            console.keys("a")
            console.wait(lambda lines: marker(lines) == start)
            console.keys("w")
            console.text("Facing North")
            assert marker(console.lines()) == start
            checks.extend(["menu and tutorial", "Host background server and second socket player", "movement and blocked facing"])
            console.keys("dddds")
            console.text("Facing South")
            console.keys("e")
            console.text("Eye painting")
            console.keys("i")
            console.text("DISCOVERED NOTES")
            peer.send({"type": "chat", "text": "Probe partner message"})
            console.text("Probe partner message")
            console.keys("\x1btProbe host message\r")
            peer.next(lambda m: m.get("type") == "view" and "Probe host message" in m.get("chat", ""))
            checks.extend(["E clue inspection", "I discovered notes", "socket updates during notes", "T message sent through sockets"])
            console.keys("q")
            console.text("return to menu")
            peer.next(lambda m: m.get("type") == "session_end")
            console.keys("\rq")
            assert process.wait(timeout=5) == 0
            assert console.mode() == console.original_mode
            assert "TWO ROOMS ONE EXIT" not in "\n".join(console.lines())
            with socket.socket() as check_socket:
                assert check_socket.connect_ex(("127.0.0.1", port)) != 0
            checks.extend(["Q exit and terminal restoration", "host server port closed after owned child cleanup"])
            signal.signal(signal.SIGINT, lambda _signal, _frame: None)
            process = subprocess.Popen([str(binary)], stdin=input_file, stdout=output_file, stderr=output_file)
            console.text("TWO ROOMS ONE EXIT")
            assert console.kernel.GenerateConsoleCtrlEvent(0, 0)
            assert process.wait(timeout=5) == 0
            assert console.mode() == console.original_mode
            checks.append("Ctrl+C exit and terminal restoration")
            result["status"] = "passed"
    except OSError as error:
        result.update(status="blocked" if getattr(error, "winerror", None) == 4551 else "failed",
                      error=str(error), winerror=getattr(error, "winerror", None))
    except Exception as error:
        result["error"] = repr(error)
    finally:
        if peer:
            try:
                peer.send({"type": "quit"})
            except OSError:
                pass
            peer.socket.close()
        if process and process.poll() is None:
            try:
                console.keys("\x1bq\rq")
                process.wait(timeout=3)
            except (AssertionError, OSError, subprocess.TimeoutExpired):
                process.kill()
                process.wait(timeout=5)
        if console:
            console.close()
        result_path.write_text(json.dumps(result, indent=2), encoding="utf-8")
    return 0 if result["status"] == "passed" else 3 if result["status"] == "blocked" else 1


def main():
    if os.name != "nt":
        print("This controlled probe requires native Windows.")
        return 2
    if len(sys.argv) == 4 and sys.argv[1] == "--worker":
        return worker(Path(sys.argv[2]), Path(sys.argv[3]))
    root = Path(__file__).resolve().parents[1]
    binary = Path(sys.argv[1]).resolve() if len(sys.argv) == 2 else root / "submissions/game/2r1e.exe"
    result = root / "submissions/game/native-ui-probe.json"
    result.parent.mkdir(parents=True, exist_ok=True)
    startup = subprocess.STARTUPINFO()
    startup.dwFlags |= subprocess.STARTF_USESHOWWINDOW
    startup.wShowWindow = subprocess.SW_HIDE
    process = subprocess.run([sys.executable, str(Path(__file__).resolve()), "--worker", str(binary), str(result)],
                             startupinfo=startup, creationflags=subprocess.CREATE_NEW_CONSOLE, timeout=45)
    print(result.read_text(encoding="utf-8"))
    return process.returncode


if __name__ == "__main__":
    raise SystemExit(main())
