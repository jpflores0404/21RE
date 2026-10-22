# Tile Legend

**Task 1.4 · Owners: M3 + M1 · Due: Oct 26**

Every map is a plain text file. One character = one tile.

| Char | Tile | Walkable | Colour | Notes |
|---|---|---|---|---|
| `#` | Wall | no | white | Outer walls and inner walls |
| `.` | Floor | yes | dim | |
| `,` | Carpet | yes | magenta | Decoration only |
| `/` | Doorway | yes | dim | Open gap between two areas, drawn as floor |
| `+` | Locked door | no | red | |
| `=` | Exit door | no | red / green | Turns green when every task is done |
| `P` | Painting | no | yellow | On the north wall of the gallery |
| `\|` | Bookshelf | no | yellow | |
| `T` | Reading table | no | yellow | |
| `D` | Desk | no | yellow | |
| `C` | Cabinet | no | yellow | |
| `b` | Bench | no | yellow | |
| `o` | Pedestal | no | yellow | |
| `%` | Control panel | no | yellow | **Task station** (wiring, terminals, fuse box) |
| `@` | Spawn | — | — | Exactly one per map; becomes floor when loaded |
| ` ` | Void | no | — | Fills short rows |

Rules:
- Exactly one `@` per map.
- The outer edge of every map must be solid tiles only.
- Every floor tile must be reachable from `@`.
- Objects (paintings, tasks, ...) must sit on a **solid** tile with a floor
  tile next to it, so the player can stand there and face it.
