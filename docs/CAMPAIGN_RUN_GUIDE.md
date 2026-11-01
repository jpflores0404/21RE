# Maps 1 and 2 milestone run guide

This is the October 31 approved C milestone: working Maps 1 and 2, six room layouts, socket communication and a walkable unfinished Map 3. It is not the completed three-map campaign. Use the [current verification report](progress_reports/2026-11-02_maps_1_2_milestone.md) for actual tested behavior and remaining limits.

## Windows local play

November 2 verification: the final native MSVC build passes rule/protocol checks, the full two-socket campaign and controlled Windows console checks. Earlier builds were blocked by Application Control with error 4551; the final ordinary rebuild launches without any policy change or bypass. Two-human acceptance remains pending.

Open two PowerShell terminals at the repository root. In each, run:

```powershell
& ./submissions/game/2r1e.exe
```

In terminal A, choose Host. In terminal B, choose Join and use 127.0.0.1 with the same port. Default port is 5050. The host starts one background server copy of the same executable; only two player terminals need to be visible. The host's server is cleaned up when the session ends.

Windows binaries remain local in ignored `submissions/`. Build from source using `scripts/build_game_windows.ps1` when changing the code or using another Windows machine. The earlier `movement_demo.exe` remains a separate learning checkpoint.

## Linux distribution

The Linux x86_64 executable is `releases/linux-x86_64/2r1e`. On a 64-bit Intel/AMD Linux system:

```sh
chmod +x releases/linux-x86_64/2r1e
./releases/linux-x86_64/2r1e
```

Open the program in two terminals and use the same Host/Join sequence. If you download the binary alone, run `chmod +x 2r1e` and `./2r1e` from its download folder. Build from source with `sh scripts/build_game_linux.sh` on a Linux system with a C compiler.

For controlled server tests, a standalone headless server is available:

```sh
./releases/linux-x86_64/2r1e --server 127.0.0.1 5050 classroom-test
```

The final argument identifies that server instance for host startup verification; it is not an authentication system. Normal users should use the menu's Host option rather than start both kinds of host on the same port.

## Gameplay

- WASD moves and changes facing. A blocked move still turns toward the object.
- E reads or interacts with the object directly ahead.
- I shows the current player's discovered notes.
- Enter confirms input. Escape cancels a dialog.
- T sends a short player message over the actual socket connection.
- Q outside text entry leaves the session. A typed q inside an answer or message remains text.

Read Tutorial before hosting. The ordering note's Three Blanks interaction offers named choices, avoiding ambiguous exact-word riddle grading. Follow the displayed controls on each puzzle dialog. Wrong answers remain retryable; notes and completed stages are preserved when canceling.

Both players must activate their own enabled exit controls to advance. In Map 2, the receiver operator sees calibration feedback that the dial operator does not see; describe it to the partner or send a message. On entering Map 3, the milestone is complete but the campaign is not won. Explore the preview; its final controls remain sealed.

## Localhost versus LAN

127.0.0.1 connects two programs on one computer. For a later LAN test, explicitly bind the host to a suitable local interface or 0.0.0.0 and have the partner join that computer's LAN IPv4 address. The computers must permit traffic to the chosen port. Do not label a same-computer test as two-machine LAN evidence.

If startup fails, check the address, port and whether another host already owns the port. A third player is rejected. Disconnecting a player ends the session clearly; automatic reconnection and saved sessions are not included. Resize a small terminal to at least 80x24 for the full interface; socket events must continue while a resize notice is displayed.

## Source and learning

The authoritative server owns room positions, stage flags and puzzle outcomes. Clients send intentions such as moving or submitting an answer. The server checks identity, position, ownership and prerequisites before producing each permitted player view. cJSON parses messages; parsing alone does not authorize an action.

See `features/campaign_rules.md`, `features/campaign_network.md`, `features/campaign_interface.md` and `features/campaign_contracts.md` for source explanations. AI assistance is declared. Student review, teach-back and professor acceptance must be recorded separately from automated checks.
