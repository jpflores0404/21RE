# Controlled native Windows campaign terminal checks

Date: October 31, 2026, Asia/Manila. This verification helper supports the approved Maps 1 and 2 milestone. It does not modify game implementation, install software, or bypass operating-system policy.

## Actual result

Final rerun, November 2: the ordinary native MSVC rebuild launches successfully without any policy change or bypass. `python tests/terminal_campaign_probe.py` now records `status: passed` with ten completed groups: menu/tutorial; Host background server with socket Player 2; movement/blocked facing; E inspection; I notes; live socket updates during notes; T messages; Q and terminal restoration; owned-server port cleanup; and Ctrl+C with restoration. The final socket campaign also passes separately against the native Windows binary. The earlier blocked run below is retained as history and no longer describes the final executable.

Running `python tests/terminal_campaign_probe.py` attempted to launch the native `submissions/game/2r1e.exe` inside a separate hidden Windows console. Windows rejected process creation with **WinError 4551**, reporting "An Application Control policy has blocked this file". The helper recorded `status: blocked` and an empty `checks` array in ignored `submissions/game/native-ui-probe.json`.

No native game menu, movement, tutorial, Host/Join, dialog, resize, or cleanup assertion executed in that run. A successful C build and passing pure rule checks do not establish native Windows UI acceptance. The policy block must be resolved by an authorized operator before this native probe or human Windows play can pass. No workaround, renamed executable, policy change, or alternate execution bypass was attempted.

The Python helper itself executed its hidden worker, typed Win32 declarations, console setup and error/result handling. This is evidence that the game launch was blocked, not that the game crashed or that a gameplay test failed.

## Intended scope of the reusable helper

The probe uses one actual Windows player UI as Host and a second standard-library socket peer. Its intended assertions are main menu and Tutorial/Escape navigation; owned background server startup; Player 2 assignment; movement and blocked movement/facing; E clue inspection; I notes; incoming chat updates while notes remain open; outgoing T messages; Q returning to the menu; terminal mode and original-screen restoration; closed host port after owned child cleanup; and a separate Ctrl+C shutdown.

The helper is not a two-human or two-GUI-terminal playtest. It does not currently test the Join menu with a second UI, puzzle text/dial dialogs, full campaign completion, resizing, physical LAN or Linux rendering. Separate integration/UI evidence must cover those conditions. Its intended assertions remain unverified until a run gets past Application Control.

## Files and every function

`tests/terminal_campaign_probe.py` is new standard-library Python verification code. It is launched by a developer after the C executable has been built. `docs/features/campaign_terminal_checks.md` records its honest scope and actual result. The generated result JSON belongs to the already ignored local build directory, not the source release.

- `Coord`, `Rect`, `BufferInfo`, `Character`, `KeyEvent`, `Event` and `InputRecord` contain no custom methods. Their `_fields_` declarations describe Windows binary layouts used by ctypes; they are not game classes or wrappers around gameplay state.
- `Console.__init__()` binds typed kernel APIs, allocates a console if needed, hides its window, opens console input, and saves the incoming input mode. API failures raise an assertion/error before gameplay checks. It changes only the helper's console environment.
- `Console.open_handle(name)` opens CONIN$ or the active CONOUT$, checks the invalid-handle sentinel, and returns a handle owned by the caller. `lines` and `close` release those owned handles.
- `Console.mode()` reads and returns the console input flags through an output pointer. It changes no input flags itself and fails on a rejected API query.
- `Console.lines()` opens the currently active output buffer, reads its viewport dimensions and each row, returns a list of strings, and closes its temporary handle in `finally`. It observes the real game output rather than inventing a view from server state.
- `Console.wait(predicate)` polls screen rows with a five-second bound. It returns the satisfying rows or raises an assertion with short screen context. The brief waits avoid busy polling and do not impose a game frame rate.
- `Console.text(text)` passes a substring predicate to `wait`. Its small lambda accepts a row list and returns whether the expected text is present.
- `Console.keys(text)` creates actual key-down INPUT_RECORD values with repeat one, translated characters and virtual key codes, then writes the array to the console input queue. It asserts the accepted count. It does not call a game movement function directly.
- `Console.close()` closes the input handle and detaches the helper console. It is not the game cleanup function.
- `Peer.__init__(port)` opens a real localhost TCP connection with a finite timeout and initializes a receive buffer. `Peer.next(predicate)` retains fragmented bytes until a newline, parses complete JSON messages, and returns the first accepted message. It raises on EOF/timeout or invalid JSON. `Peer.send(value)` serializes a dictionary and sends newline-framed JSON. This small test peer does not represent a second visible terminal.
- `marker(lines)` searches screen rows for `@` and returns its screen coordinates, or `None`. It does not calculate an expected player position from game state.
- `worker(binary, result_path)` owns the controlled sequence and the C process. It records each completed assertion group only after success. A policy-blocked OSError becomes `blocked`; other exceptions become `failed`. In cleanup it requests peer quit, attempts graceful UI exit, and terminates only its own child if the bounded wait fails. It writes a JSON result even on failure. Intended socket/message predicates are small lambdas selecting welcome, started view, chat or session-end messages.
- `main()` rejects non-Windows runs, dispatches worker mode, or starts a separate hidden console worker using CREATE_NEW_CONSOLE and SW_HIDE. It prints the result and returns the worker status. It never starts a visible test window. The separate worker avoids changing the invoking shell's console settings.

## Concept and trace

A native console event is supplied by Windows to the program. This probe injects such an event through the documented console API, waits for the real rendered state, and compares it with the expected result. For example, D at a visible marker should move it one cell right; it does not simulate that move inside the test. A server message sent while the notes view is open should appear through the real UI/network loop.

The first attempted native C process did not start because of policy, so that trace was not executed. Both developers' source review, hands-on changes and teach-back remain pending. Do not mark the intended checks as completed by reading this note.
