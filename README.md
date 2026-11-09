# 2R1E: Two Rooms, One Exit

A two-player terminal escape game in C17. Explore separate areas, share clues by speaking to your partner, and solve three maps. There is no in-game chat.

## Run on Linux

Use a terminal with at least 80 columns and 30 rows on each computer. The included executable is for Linux x86_64.

From the repository directory, run:

```sh
chmod +x 2R1E
./2R1E
```

To build from source, install GCC and the standard Linux development headers, then run:

```sh
gcc -std=c17 -O2 -Wall -Wextra -Werror main.c maps.c movement.c puzzles.c display.c network.c terminal.c -o 2R1E
./2R1E
```

## Connect

Both players need the same game version and a network that allows connections between their computers.

1. Player 1 chooses **Host game**, enters their reachable local IPv4 address from the addresses shown, and chooses an unused TCP port from `1024` to `65535`.
2. Player 1 tells Player 2 the address and port and waits for the connection.
3. Player 2 chooses **Join game** and enters that same host address and port.

The host's firewall must allow incoming TCP connections on the chosen port. For two terminals on the same Linux computer, use `127.0.0.1` and a matching port. For separate computers, use the host's LAN address. If either player disconnects, start a new session.

## Controls

| Key | Action |
|---|---|
| W / A / S / D | Move and face that direction |
| E | Inspect or use the object directly ahead |
| Enter | Confirm a menu choice or submit an answer |
| Backspace | Delete a character while typing |
| Escape | Cancel text entry or connection setup |
| Q | Leave gameplay or quit from the menu |

Stand beside an object, face it, and press E. When `Answer>` appears, type the answer and press Enter. Movement keys and Q become text while typing. Wrong answers can be retried.

Both players must use their own `E` exit object to advance on each map. After winning, press Enter on each terminal to return to the menu.

## Answer keys: spoilers

Complete these steps in order. Player 1 is Host; Player 2 is Join.

| Map | Player | Object | Answer or action |
|---|---|---|---|
| 1 | 2 | N, Three Blanks | Enter `light passage watcher` |
| 1 | 2 | K, cabinet | Enter `246` |
| 1 | 1 | H, hatch | Enter `259` |
| 1 | Both | Their own E exit | Face it and press E |
| 2 | 1 | B, statue | Face it and press E to open the passage |
| 2 | 2 | D, dial panel, or T, test console | Enter `2 3 4` with spaces |
| 2 | Both | Their own E exit | Face it and press E |
| 3 | 1 | 4, White Return valve | Face it and press E to obtain clearance |
| 3 | 1 | A, authorization terminal | Face it and press E after clearance |
| 3 | 2 | S, mechanical safety release | Face it and press E after clearance |
| 3 | Both | Their own E exit | Face it and press E to win |

Map 3 authorization and safety release can be completed in either order after clearance.
