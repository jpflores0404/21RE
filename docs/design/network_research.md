# Network Research

**Task 1.6 · Owner: M2 · Due: Oct 26**

| Option | Pros | Cons | Decision |
|---|---|---|---|
| TCP sockets (`socket`) | Built in, reliable, ordered | Must handle disconnects | **Chosen** |
| UDP | Fast | Packets can be lost or reordered; we would re-invent TCP | No |
| HTTP server (`http.server`) | Familiar | Polling; heavier than we need | No |
| Third-party (websockets, etc.) | Easy API | Needs `pip install` | No |

Findings:
- `socket.create_connection((ip, 5050), timeout=5)` gives a clear error if
  the host is wrong.
- `SO_REUSEADDR` lets the host restart quickly on the same port.
- `accept()` with a 0.2 s timeout lets the "waiting for player" screen still
  react to the Q key.
- School Wi-Fi may block device-to-device traffic. Backup plan: a phone
  hotspot, or two terminals on one laptop with `127.0.0.1`.
- To show the host's IP: open a UDP socket "to" 8.8.8.8 and read
  `getsockname()`. No data is actually sent.
