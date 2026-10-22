# Architecture

**Task 1.11 · Owner: M1 · Due: Oct 28**

```
               +------------------------------+
  keyboard --> |  client/   (what you see)    |
               |  menu, game screen, tasks,   |
               |  journal, close-ups, lobby   |
               +------+---------------+-------+
                      | calls         | send()/receive()
                      v               v
      +-------------------------+   +--------------------------+
      |  game/  (rules only)    |   | client/network.py        |
      |  tiles, map, movement,  |   | LocalLink  (1 computer)  |
      |  level loader, session  |   | NetLink    (LAN, TCP)    |
      |  (E / F), team (tasks)  |   +------------+-------------+
      +-----------+-------------+                |
                  | reads                        | JSON lines
                  v                              v
      +-------------------------+        partner's computer
      | assets/                 |
      | maps/*.txt, *.json, art |
      +-------------------------+
```

Rules we agreed on:
1. `game/` never imports `curses` or `socket`, so it can be unit tested.
2. Level content (maps, objects, clues, answers) lives in `assets/` as text
   and JSON, not in Python code.
3. Each mini-game is one function that returns True (done) or False (left).
