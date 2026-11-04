# 2R1E 50 classroom demo

Two players explore separate rooms, exchange clues, and solve the puzzles in Maps 1 and 2. The host runs the game rules and plays as Player 1. Player 2 joins over one TCP connection on port 5050.

## Run on Linux

Copy `2r1e50` to each school PC. The supplied executable is for 64-bit Intel or AMD Linux. Open a terminal in the folder containing the file and run these commands on each PC.

```sh
chmod +x 2r1e50
./2r1e50
```

### Build it yourself

With GCC already available, open a terminal inside `2R1E_50` and run:

```sh
gcc -std=c17 -Wall -Wextra -Wpedantic -O2 main.c game.c net.c terminal.c -o 2r1e50
./2r1e50
```

The four C files build into one executable. The compiler uses the system C library and socket interfaces. Running the supplied binary requires no compiler, Python, or additional game files.

### Connect two school PCs

1. Connect both PCs to the same school LAN. On the host PC, run `hostname -I` or `ip -4 addr` and note the IPv4 address on its active Wi-Fi or Ethernet connection.
2. Run `./2r1e50` on the host PC and select **1. Host game**. It listens on all local IPv4 interfaces on TCP port `5050`.
3. Run `./2r1e50` on the second PC and select **2. Join game**. Enter the host PC’s actual LAN IPv4 address. The program uses port `5050` automatically.
4. Send `say hello` from one PC and a reply from the other. Then explore the two rooms.

For a same-PC rehearsal, open two terminals and enter `127.0.0.1` in the Join menu. If the school PCs cannot connect, confirm the IP address and ask school IT whether that network allows device-to-device traffic on TCP `5050`. Press Ctrl+C to cancel a Host that is still waiting for a connection.

## Play

Enter each command and press **Enter**. A screen update starts with the map and player number, then shows the room, clue, chat, and controls. Use a terminal about 80 columns wide for readable clue lines.

| Command | Action |
|---|---|
| `w`, `a`, `s`, `d` | Move one tile. Walking toward an object turns you to face it. |
| `e` | Inspect the tile you face or use your exit. |
| `answer TEXT` | Submit an answer at the puzzle you face. |
| `say TEXT` | Send a short message to your partner. |
| `set A B C` and `test` | As Player 2 on Map 2, face the calibration controls, enter three values from 0 to 4, and test them. |
| `q` | End the session and return to the menu. |

Both players must use their own exit to progress. The demo finishes after Map 2. You can revisit an object to reread its clue. The terminal retains earlier screens in its scrollback.

## Windows build for local practice

From `2R1E_50` in an x64 Visual Studio Developer Command Prompt:

```bat
mkdir ..\submissions\2R1E_50
cl /nologo /std:c17 /W4 /D_CRT_SECURE_NO_WARNINGS main.c game.c net.c terminal.c /Fo..\submissions\2R1E_50\ /Fe..\submissions\2R1E_50\2r1e50.exe ws2_32.lib
..\submissions\2R1E_50\2r1e50.exe
```

Use the same Host and Join steps in two Windows terminals. The Windows output belongs in the local `submissions` folder.
