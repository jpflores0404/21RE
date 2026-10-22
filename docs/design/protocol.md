# Network Protocol Draft

**Task 1.10 · Owner: M2 · Due: Oct 28**

**Transport:** TCP on port 5050. Each message is one line of JSON ending in `\n`.
**Model:** the host listens and the joiner connects. After that both sides
are equal (peer to peer). Each computer runs the rules for its **own** room and
tells the partner about the few things that are **shared**.

| Message | Example | When |
|---|---|---|
| task | `{"type":"task","id":"a_download"}` | A player finishes a task |
| clue | `{"type":"clue","to":2,"text":"Painting 'The Key' is marked 4."}` | Shared from the journal |
| chat | `{"type":"chat","from":1,"text":"what's your order?"}` | T key |
| escaped | `{"type":"escaped","player":1}` | A player walks out of the open exit |
| bye | `{"type":"bye"}` | Created locally when the connection closes |

Why not send every move? The partner never sees the other room, so positions
do not need to be shared. Only the TOTAL TASKS bar, clues, chat and escapes
are shared. That keeps the network code small.

Design decisions:
- A background thread reads from the socket into a queue. The game loop checks
  the queue every 0.1 s, so the screen never freezes.
- One-computer mode uses a fake "LocalLink" with the same `send()` /
  `receive()`, so the game loop works the same way in both modes.
- If the connection drops, the reader thread queues `bye`, and the game shows
  "Your partner left" and goes back to the menu.
