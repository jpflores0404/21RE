# Explaining the C movement demo on Linux and Windows

Prepared October 28, 2026 for the 2R1E Operating Systems presentation. Read this beside `src/movement_demo.c`. This guide explains the actual local prototype, including its limitations. It does not claim that networking, campaign puzzles, or developer understanding have been completed.

## 1 What the program does

One C process draws an `@` player inside a bordered terminal. W moves up, A left, S down, D right, and Q quits. Enter is unnecessary. The program reads the current terminal dimensions, keeps the player within the playable region, and redraws after a move or resize. A small window pauses movement and displays an enlargement message.

There is one maintained source file containing both platform implementations. `_WIN32` selects the Windows code during compilation. Linux uses the other branch. The compiled Windows program does not contain a second running Linux program, and the Linux executable does not call Windows APIs. They share the same movement and layout functions.

The existing Windows executable is not a Linux executable. Linux must compile the C source to produce a Linux binary. Source portability means the source can be built for different environments; it does not mean the same executable runs on every operating system.

| Operation | Windows branch | Linux branch |
|---|---|---|
| Read input | Structured console events and key-down character | Noncanonical terminal input bytes |
| Wait for input | `WaitForSingleObject` on an input handle | `select` on standard input |
| Draw characters | Write directly into a console screen buffer | Write cursor escape sequences and text |
| Read dimensions | `GetConsoleScreenBufferInfo` | `ioctl` with `TIOCGWINSZ` |
| Preserve previous view | A separately created console buffer | Alternate-screen control sequence |
| Request shutdown | Console callback and Interlocked flag | Signal handler and `sig_atomic_t` flag |
| Update player | Shared C functions | The same shared C functions |

### First explanation to give the professor

"Our prototype separates movement rules from terminal operations. Both platforms use the same player coordinates and boundary checks. Windows receives console event records, while Linux receives terminal input bytes after we change the terminal mode. The program changes the player's position and redraws the screen only when necessary."

## 2 Build and run

### Linux commands

With a C compiler installed, run from the repository root:

```sh
sh scripts/run_movement_linux.sh --self-test
sh scripts/run_movement_linux.sh
```

The script compiles the same `src/movement_demo.c` as C17. The first command then runs the pure movement checks; the second starts the interactive program. Use an actual terminal with compatible cursor and alternate-screen escape sequences. A typical xterm-compatible Linux terminal is the intended environment.

Equivalent direct commands are:

```sh
mkdir -p submissions/movement-demo
cc -std=c17 -Wall -Wextra -Wpedantic -Werror src/movement_demo.c -o submissions/movement-demo/movement_demo
./submissions/movement-demo/movement_demo --self-test
./submissions/movement-demo/movement_demo
```

`cc` is the system's C compiler command. `-std=c17` chooses the language standard. The warning flags request diagnostics, and `-Werror` makes warnings fail the build. `-o` names the output executable. There is no ncurses link flag because this demo does not use ncurses. The POSIX functions used here are normally provided through the system C library.

The launcher uses quoted paths so a repository folder containing spaces works. It checks that it is running on Linux and that `cc` exists. `set -eu` stops on failed commands or unset shell variables. It finds the repository relative to the script, creates an ignored output folder, compiles, and uses `exec` to replace the launcher with the demo, forwarding its arguments. It does not install a compiler.

### Windows commands

The previously built native executable can be launched in PowerShell:

```powershell
& ./submissions/movement-demo/movement_demo.exe --self-test
& ./submissions/movement-demo/movement_demo.exe
```

To rebuild in an MSVC Developer Command Prompt, use:

```bat
cl /nologo /std:c17 /W4 /WX /TC src\movement_demo.c /Fosubmissions\movement-demo\movement_demo.obj /Fesubmissions\movement-demo\movement_demo.exe
```

Create the output folder first. The exact compiler setup command for this computer is in `docs/MOVEMENT_DEMO.md`. `/TC` explicitly selects C; `.c` is the source file, `.obj` the compiled object, and `.exe` the linked Windows program. MSVC supplies its normal runtime and default Windows import libraries; the demo has no separate Winsock dependency because it has no sockets.

### Verification status

The Windows C17 build, 14 shared movement checks, and controlled native Windows console checks passed on October 27. The console checks covered WASD, boundaries, resizing, tiny-window behavior, Q, Ctrl+C, and restoration.

For this comparison, the installed WSL distribution reported Linux but had no `cc`, `gcc`, or `clang` on its command path. Docker's Linux engine was not running. Linux compilation and Linux terminal execution therefore remain unverified. On October 28, `sh -n` accepted the launcher and running it in the available Linux environment returned its expected missing-compiler message with exit status one. These shell checks cannot establish that the C source compiles or renders correctly on Linux. The Windows executable's 14 shared movement checks passed again during this work.

## 3 Headers and libraries

A header provides declarations, types, and macros to the compiler. A library provides compiled implementations used when linking or running. Including a header does not install software, start a process, or read a key.

### Standard C headers used on both platforms

| Header | Items this source uses | Reason |
|---|---|---|
| `stdio.h` | `printf`, `fprintf`, `fputs`, `puts`, `fwrite`, `fflush`, `stdout`, `stderr` | Drawing, diagnostics, and test results |
| `stdlib.h` | `malloc`, `free`, `EXIT_SUCCESS`, `EXIT_FAILURE` | Allocate a drawing row and return a process result |
| `string.h` | `strcmp`, `strlen`, `memset`, `memcpy` | Parse the self-test argument, measure labels, and fill/copy drawing bytes |

### Windows header

`windows.h` provides the Win32 declarations used by the console adapter. `WIN32_LEAN_AND_MEAN` is defined before it to exclude some unnecessary declarations. It does not turn Windows into Linux or provide standard C portability by itself.

Types used here include `HANDLE` for an OS object reference, `DWORD` for an unsigned 32-bit value, `LONG` for a signed 32-bit value, `BOOL` for the Win32 true/false convention, `SHORT` for a 16-bit signed coordinate component, and `COORD` for X/Y coordinates. `CONSOLE_SCREEN_BUFFER_INFO`, `CONSOLE_CURSOR_INFO`, and `INPUT_RECORD` describe console state and events. A handle is not a pointer to a struct that the program should dereference or free with `free`.

### Linux headers

| Header | Items this source uses | Reason |
|---|---|---|
| `errno.h` | `errno`, `EINTR` | Distinguish interrupted input operations from other errors |
| `signal.h` | `sig_atomic_t`, `sigaction`, `sigemptyset`, `SIGINT`, `SIGTERM` | Request orderly shutdown when a handled signal arrives |
| `sys/ioctl.h` | `ioctl`, `TIOCGWINSZ`, `struct winsize` | Obtain terminal columns and rows |
| `sys/select.h` | `fd_set`, `FD_ZERO`, `FD_SET`, `select`, `struct timeval` | Wait for terminal input with a timeout |
| `termios.h` | `struct termios`, terminal flags, `tcgetattr`, `tcsetattr` | Save and change terminal input behavior |
| `unistd.h` | `STDIN_FILENO`, `STDOUT_FILENO`, `isatty`, `read`, `ssize_t` | Work with POSIX input/output descriptors |

The Linux headers are OS interfaces beyond ISO C. Being declared in system headers and supplied by libc does not make their behavior part of portable ISO C. See the library sections of the [Linux terminal manual](https://man7.org/linux/man-pages/man3/termios.3.html) and [select manual](https://man7.org/linux/man-pages/man2/select.2.html).

## 4 Preprocessor and data fundamentals

The file starts by defining `_POSIX_C_SOURCE` as `200809L` only outside Windows. This feature-test macro is placed before system headers so the supported POSIX declarations are exposed. It is a request to the headers, not a test proving that Linux is installed. `_WIN32` is normally defined by a Windows-targeting compiler even for a 64-bit build.

```c
typedef struct { int x, y; } Player;
```

`struct` groups values. `typedef` gives that type the short name `Player`. `x` is a column and `y` is a row. These are integer cells, not pixels or floating-point world coordinates. The top-left coordinate is zero,zero. Increasing x moves right; increasing y moves down.

`Player player` passes a copy of the struct. `Player *player` passes a pointer through which the function can change the original. `player->x` accesses a member through a pointer; `player.x` accesses a local value. `Player next = *player` copies the current state. `*player = next` commits the accepted copy.

At file scope, `static` keeps a function or variable private to this translation unit. A translation unit is the source after its headers and preprocessor conditions have been processed. Inside a function, a static variable instead keeps its value between calls. No function-local static variable is needed for the movement rules.

An integer zero is false in C; nonzero is true. `&&` means both conditions must succeed and short-circuits the right side. `||` accepts either condition and also short-circuits. `!` reverses a truth value. `&` takes an address when used as a unary operator, but performs a bitwise AND between two values. These are different uses of the same symbol.

## 5 The common movement functions

### playable

Signature: `static int playable(int width, int height)`.

The two integer parameters are the current viewport size. It returns true only for width at least 20 and height at least 6. It changes no state. For 80 by 24, it returns true. For 19 by 24, it returns false. The minimum is a demo layout constraint, not a limit imposed by C or Linux.

### keep_inside

Signature: `static void keep_inside(Player *player, int width, int height)`.

It first returns without changes if the window is too small. Otherwise it clamps x between 1 and width minus 2, and y between 2 and height minus 2. Clamp means replacing a value outside a permitted interval with its nearest boundary. Each `if` corrects one lower or upper bound.

In an 80 by 24 viewport, row zero is the header, row one the top border, and row 23 the bottom border. Columns zero and 79 are side borders. Valid player coordinates are therefore x from 1 to 78 and y from 2 to 22. When shrinking to 20 by 6, position 78,22 becomes 18,4. The pointer parameter makes these changes visible to the caller. `void` means no value is returned.

### move_player

Signature: `static int move_player(Player *player, int key, int width, int height)`.

1. Copy the player into `next`; do not change the original yet.
2. Reject movement if the viewport is too small.
3. Use a `switch` to map W/w to y minus one, A/a to x minus one, S/s to y plus one, and D/d to x plus one.
4. The default case rejects every other integer key value.
5. Clamp the candidate to the playable interval.
6. If candidate and original are identical, return zero.
7. Otherwise copy `next` back and return one.

The return value reports whether a redraw is necessary, not whether a physical key was pressed. At x=78, D generates candidate x=79, which clamps back to 78; no move occurs. Q is recognized by `main`, so it never needs to be a movement direction. These rules are identical on Windows and Linux.

## 6 Reading keys on Windows

### Console setup

`terminal_open` obtains the inherited standard output and input handles using `GetStdHandle`. It verifies console input with `GetConsoleMode` and verifies output with `GetConsoleScreenBufferInfo`. This rejects redirected files and ordinary editor output panels before the program assumes it can draw a console.

It saves the original input mode and creates a separate screen with `CreateConsoleScreenBuffer`. The access flags allow the program to read/write the buffer, and sharing flags permit shared access. `CONSOLE_TEXTMODE_BUFFER` requests a text buffer. It hides the new buffer's cursor, registers the stop callback, and activates the new screen using `SetConsoleActiveScreenBuffer`.

The mode expression enables window events and the extended-mode flag, and clears line input, echo, and Quick Edit. A bit mask combines flags using `|`, and `& ~mask` clears selected bits while retaining others. Quick Edit is disabled so selecting text does not interfere with this demo. Low-level event reading is performed by `ReadConsoleInputA`; it is not the same input path as `scanf` or `ReadConsole` line input. [Windows console modes](https://learn.microsoft.com/en-us/windows/console/setconsolemode) describe these distinctions.

The code checks setup results one by one. On failure it returns zero; `main` still calls `terminal_close`, which uses saved flags and valid handles to clean up partial setup. `original_screen` is used to validate the original console; its local copy is not the player's map.

`INVALID_HANDLE_VALUE` is the sentinel used here for a handle that is not available. The cursor initializer `{ 25, FALSE }` requests a 25-percent cursor size and hides it; it does not set a 25-millisecond timing interval. Global saved-state variables persist for the process lifetime. `ENABLE_PROCESSED_INPUT` is retained from the incoming console mode rather than explicitly forced on, so the Ctrl+C explanation assumes the normal shell configuration.

### read_key statement by statement

```c
INPUT_RECORD record;
DWORD count;
DWORD wait_result = WaitForSingleObject(console_input, 50);
```

`record` receives one structured event. `count` receives the number actually read. The wait sleeps until the console input handle is signaled or approximately 50 milliseconds pass. It is an efficient OS wait, not an active loop constantly checking a key. A signaled input buffer can contain keyboard, resize, or other records.

```c
if (wait_result == WAIT_TIMEOUT) return 0;
if (wait_result != WAIT_OBJECT_0 ||
    !ReadConsoleInputA(console_input, &record, 1, &count) || count != 1)
    return -1;
```

A timeout means there is no key to act on this iteration. A wait failure, failed read, or unexpected record count is an input failure. The address operators let the API fill `record` and `count`. The requested length is one record, not one byte. Waiting before the read prevents the ordinary no-input case from making this path wait indefinitely. [Microsoft's input API reference](https://learn.microsoft.com/en-us/windows/console/readconsoleinput) documents the record and count parameters.

The final condition accepts `KEY_EVENT` only when `bKeyDown` is true. A key-up event is discarded. `uChar.AsciiChar` is the translated byte, so the adapter returns `'w'`, not a physical keyboard scan code. Casting to `unsigned char` prevents a byte above 127 from becoming a negative error value. The narrow A API is sufficient for ASCII WASD, but is not a complete Unicode text-entry design.

Resize events and keys without a useful character return zero. The loop checks the actual dimensions again on the next iteration. A held key may be represented with `wRepeatCount` greater than one; this prototype does not expand that count. It performs one move per accepted key-down record, so repeat feel is not guaranteed to match Linux. See the [keyboard event documentation](https://learn.microsoft.com/en-us/windows/console/low-level-console-input-functions).

## 7 Reading keys on Linux

### Descriptors and terminal mode

A POSIX file descriptor is an integer identifying an open input/output resource in the process. `STDIN_FILENO` is normally zero, and `STDOUT_FILENO` normally one. They are different from the `FILE *` streams named `stdin` and `stdout`, although they usually refer to the same underlying resources.

`terminal_open` checks input and output with `isatty`, saves the current settings with `tcgetattr`, and copies them into a local `struct termios input`. It modifies the copy and applies it using `tcsetattr` with `TCSANOW`, meaning apply now. Failed calls return zero from the wrapper and lead to cleanup.

The actual settings in this source clear `ICANON` and `ECHO`, then clear `IXON` and `ICRNL`. `ICANON` controls line-oriented input; `ECHO` causes typed characters to be echoed; `IXON` enables software flow control; `ICRNL` maps carriage return on input. The code also sets `VMIN` and `VTIME` to zero. It keeps signal processing enabled when inherited from the normal shell. This is noncanonical input, not fully raw mode. [termios flags and noncanonical read behavior](https://man7.org/linux/man-pages/man3/termios.3.html) specify the underlying controls.

The code saves state before changing it. The saved-state flag is set after the new mode has actually been applied. It then prints sequences to enter an alternate screen and hide the cursor, and flushes output. A separate section below explains drawing and those sequences.

### Why the program does not wait for Enter

In ordinary canonical terminal input, the terminal assembles a line before delivering it. Clearing `ICANON` removes that line requirement. Calling `getchar` alone would not change the terminal's line mode. Turning off `ECHO` ensures the terminal does not independently print a typed W over the game screen.

With the two read controls set to zero, a direct read is not used as the long wait. This source deliberately lets `select` perform the bounded wait, and calls `read` only after input readiness. The exact POSIX behavior of a zero-byte noncanonical read depends on the terminal settings; the prototype treats a zero result at this point as an end/error condition and exits. It assumes it is the terminal's sole input reader.

### read_key statement by statement

```c
fd_set inputs;
struct timeval timeout = { 0, 50000 };
unsigned char key;
int ready;
ssize_t count;
```

`inputs` will describe which descriptors to watch. The timeout is zero seconds plus 50,000 microseconds, or 50 milliseconds. `key` stores one byte. `ready` stores the readiness result, and `ssize_t` stores a signed byte count so minus one can represent failure.

```c
FD_ZERO(&inputs);
FD_SET(STDIN_FILENO, &inputs);
ready = select(STDIN_FILENO + 1, &inputs, NULL, NULL, &timeout);
```

The first macro empties the set. The second adds standard input. The first argument to `select` is one more than the highest descriptor watched, not a count of keyboard keys. The other two sets are unused. Sets and timeout are created again on each call because `select` can modify them. A zero return means timeout; a negative return indicates failure. [The select manual](https://man7.org/linux/man-pages/man2/select.2.html) explains readiness and return conventions.

On a negative return, `errno == EINTR` means a signal interrupted the wait. Returning zero lets `main` check the stop flag. A different error returns minus one and ends the demo through cleanup. `errno` is inspected only after a reported failure.

If input is ready, `read(STDIN_FILENO, &key, 1)` asks for one byte. A return of one becomes that unsigned byte converted to an integer. An interrupted read returns no key; other read failures return minus one. The [read interface](https://man7.org/linux/man-pages/man2/read.2.html) reports bytes read rather than returning the character directly.

### What reaches move_player

Typing lowercase D's key normally produces the byte for `'d'`. `read_key` returns that value. `main` forwards it to the movement function. No hardware driver is implemented here; the terminal and OS have already converted physical input into a byte stream.

This is not key-up/key-down event tracking, physical scan-code reading, or a Unicode text editor. Typing several bytes queues them for subsequent iterations. Holding keys is normally represented as repeated terminal bytes. The demo moves one cell for each accepted movement byte.

## 8 Drawing on each platform

### Windows position and output

`terminal_size(int *width, int *height)` calls `GetConsoleScreenBufferInfo` on the temporary output buffer. The visible window can be a region inside a larger buffer. Width is Right minus Left plus one; height is Bottom minus Top plus one. The plus one is required because both endpoints are included.

`write_at(int x, int y, const char *text, int length)` adds the visible window's Left and Top offsets to the local coordinates, stores them in a `COORD`, and calls `WriteConsoleOutputCharacterA`. The API copies exactly the requested number of characters beginning at that buffer position. The wrapper requires both API success and a matching written count. It writes character cells directly rather than printing cursor escape sequences. [Console screen buffers](https://learn.microsoft.com/en-us/windows/console/console-screen-buffers) and [WriteConsoleOutputCharacter](https://learn.microsoft.com/en-us/windows/console/writeconsoleoutputcharacter) describe these operations.

### Linux position and output

`terminal_size` calls `ioctl` with `TIOCGWINSZ` and a pointer to `struct winsize`. `ws_col` becomes width and `ws_row` becomes height. Failed queries and zero dimensions are rejected. The dimensions come from the terminal, not a fixed 80x24 constant. See [terminal window-size queries](https://man7.org/linux/man-pages/man2/TIOCGWINSZ.2const.html).

The Linux `write_at` prints `\033[%d;%dH`, with y plus one first and x plus one second, then calls `fwrite` for exactly `length` bytes. `\033` is the C octal escape for byte 27, the Escape character. Cursor positioning uses one-based row,column while the program stores zero-based x,y. Thus x=5,y=3 becomes the cursor sequence for row 4,column 6.

Setup prints `ESC[?1049h` to select an alternate screen and `ESC[?25l` to hide the cursor. Cleanup prints `ESC[?25h` to show the cursor and `ESC[?1049l` to return to the original screen. These are terminal control sequences, not Linux system calls. They require a compatible terminal emulator; printing them into a file merely stores bytes. The [xterm control-sequence reference](https://invisible-island.net/xterm/ctlseqs/ctlseqs.html) defines these operations.

### Shared render function

`render` receives a player value and the current size. It allocates one row containing exactly width bytes with `malloc`. It checks for allocation failure and returns zero if memory could not be obtained.

For each row, `memset` starts with spaces. On a playable screen, row one and the last row become horizontal borders; later interior rows receive left and right border characters. The row matching player.y receives `@` at player.x. Row zero receives the controls label, or the small-window message. The copy length is capped at width, so a narrow window does not overflow the row.

The buffer is not a null-terminated C string. That is safe because the code always supplies an explicit byte count to `write_at`. Passing this row to `printf("%s", row)` would be incorrect without an extra terminator. `const char *text` promises that `write_at` will not modify the caller's text through that pointer.

The loop writes one row at a time and stops on the first failure. It skips writing the bottom-right cell because some terminals scroll when that cell is written. It frees the allocated row exactly once and flushes standard output before reporting success. The Windows direct console writes do not use stdio buffering, but flushing is necessary for the Linux output path.

This is a simple whole-view redraw on change, not a game engine, GPU renderer, or double-buffered animation system. The Windows alternate console buffer preserves the previous screen; it is not the same thing as a frame-swapping graphics pipeline.

## 9 Signals callbacks and terminal restoration

### Linux shutdown

The source creates a `struct sigaction`, initializes it, assigns `request_stop`, clears its blocked-signal set with `sigemptyset`, and registers the action for SIGINT and SIGTERM. Ctrl+C normally produces SIGINT when the terminal's signal processing is enabled. An external `kill -TERM` sends SIGTERM. Q is an ordinary input byte, not a signal. See [sigaction](https://man7.org/linux/man-pages/man2/sigaction.2.html).

The callback's integer argument is the signal number; `(void)event` marks it intentionally unused. Its only work is setting `stop_requested = 1`. `volatile sig_atomic_t` is suitable for this simple communication with the signal handler. `volatile` is not a general replacement for atomics or locks between threads.

`stopping()` returns whether the flag is set. The normal loop eventually stops and calls `terminal_close`. It restores `original_termios`, shows the cursor, leaves the alternate screen, flushes output, and clears the saved-state flag. Restoration is best effort if the terminal has disappeared. It does not persist game state.

Cleanup is outside the signal handler because most stdio operations are not async-signal-safe. A signal could interrupt a library operation partway through updating internal state; starting another such operation inside the handler is unsafe. This implementation uses the flag to defer work. [Signal safety](https://man7.org/linux/man-pages/man7/signal-safety.7.html) explains the constraint.

### Windows shutdown

`SetConsoleCtrlHandler(request_stop, TRUE)` registers a callback. The callback returns TRUE for Ctrl+C and Ctrl+Break after setting the stop flag, and FALSE for events it does not handle. Windows may invoke console handlers outside the main execution path, so it uses Interlocked operations on the `LONG` flag. `InterlockedExchange` sets it; `InterlockedCompareExchange` reads it using an atomic operation. This is shutdown coordination, not gameplay networking. See [control-handler registration](https://learn.microsoft.com/en-us/windows/console/setconsolectrlhandler) and [Interlocked variable access](https://learn.microsoft.com/en-us/windows/win32/sync/interlocked-variable-access).

The Windows close wrapper restores the original input mode, reactivates the saved output buffer, closes only the temporary buffer it created, and unregisters the callback. It does not close standard handles owned by the terminal host. The saved flags and invalid-handle checks allow cleanup after partial startup.

### Restoration limits

No program can guarantee cleanup after forced termination, a crash, or SIGKILL. This prototype also does not handle Linux job-control suspend/resume or SIGQUIT. Ctrl+Z can suspend it without restoring the terminal first, and Ctrl+backslash can terminate it without the normal close path. For the approved demonstration, use Q or Ctrl+C. If settings are left broken, `stty sane` can recover ordinary input settings; it does not necessarily restore the alternate screen or cursor by itself.

## 10 Main loop and complete input trace

`main(int argc, char **argv)` is the entry point. `argc` counts arguments including the program name; `argv` points to argument strings. The exact single argument `--self-test` runs tests without opening a terminal. Unknown or extra arguments print usage on `stderr` and return failure. `strcmp` returns zero for equal strings, which is why the comparison uses `== 0`.

Normal startup creates an initial Player, old/current dimensions, a `dirty` flag and a successful default result. The first valid size places the player near the center. A changed size clamps the position, remembers the new dimensions, and marks the view dirty.

Dirty means the existing display no longer represents the current state. If dirty, call `render`; afterward clear the flag. Wait for input through the selected adapter. A minus-one result means failure, Q means normal exit, and movement that changes coordinates marks the next view dirty. Blocked movement or an unrelated ordinary key does not require a redraw.

The loop condition checks `stopping` each iteration. On either normal exit or a terminal error, `terminal_close` runs before the process returns its status. Error output is printed after restoration so it is visible on the original terminal. `EXIT_SUCCESS` indicates success to the shell; it is separate from `move_player` returning one to indicate a changed position.

### Trace D from input to output

Start with an 80x24 viewport and player at 10,5. On Windows, a key-down input record has ASCII `'d'`; on Linux, the terminal supplies byte `'d'`. The adapter returns that character value. `main` passes it to `move_player`, which proposes 11,5, clamps it, compares it, and commits it. The changed result sets dirty. On the next iteration `render` clears the old marker while constructing its rows and draws `@` at 11,5.

At 78,5, the same request proposes 79,5. Clamping returns 78,5, so no state change is committed and the function returns zero. That is boundary validation, not map-object collision. The prototype does not yet store a room grid or test furniture tiles.

### Timing and CPU use

The 50-millisecond wait is a timeout for input waiting. It lets the loop notice resizing or shutdown even when nobody types. It does not force a fixed 20 frames per second, guarantee a 50-millisecond response time, or update gameplay with a fixed timestep. Input may wake it sooner, and drawing/scheduling may take longer. The program avoids repeatedly redrawing an unchanged screen.

## 11 Every function at a glance

| Function | Caller and return meaning | State affected |
|---|---|---|
| `playable` | Movement, bounds, rendering and tests; true for usable dimensions | None |
| `keep_inside` | Movement, resize path and tests; no return value | Caller-owned Player coordinates |
| `move_player` | Main loop and tests; one if position changed | Player after accepted candidate |
| `terminal_open` | Main; success/failure | Saved settings, handles/modes and active display |
| `terminal_close` | Main after startup failure or loop exit; no result | Restores terminal, releases owned resource |
| `terminal_size` | Main; success/failure plus pointer outputs | Width/height; Windows cached screen information |
| `write_at` | Render; success/failure | Terminal output |
| `read_key` | Main; byte, zero for no actionable key, minus one for failure | Consumes terminal input |
| `request_stop` | OS callback; platform-specific signature | Stop flag only |
| `stopping` | Main loop condition; true after stop request | None |
| `render` | Main; success/failure | Output and temporary row allocation |
| `self_test` | Main with exact test argument; process success/failure | Local test state only |
| `main` | C runtime; process exit status | Owns the local program lifecycle |

Two versions of the platform functions appear in the text, but only one set is compiled. Matching names and signatures let the common main/render functions use either platform without knowing the OS details.

## 12 Tests and honest claims

`self_test` uses a local CHECK macro. The macro prints the failed expression and source line and increments a failure counter. Its `do { ... } while (0)` form behaves like one statement in C syntax. Unlike the standard `assert` macro, these checks do not disappear when `NDEBUG` is defined.

Inside that macro, `#condition` turns the supplied expression into text, and `__LINE__` supplies the expansion's source line number. They make a failed test explain itself. Macro expansion happens before ordinary compilation; it does not create a new runtime function named CHECK.

The 14 checks exercise up, uppercase-left, down, uppercase-right, invalid input, left border, top border, right border, bottom border, shrinking/clamping, a too-small window, enlargement without an unintended move, correction of negative coordinates, and minimum playable dimensions. They call the real shared rules. They do not inject keys or render a screen.

The prior controlled Windows console test used real OS console events and read screen contents. It supplies stronger evidence for Windows adapter behavior. It is still separate from a human judging the visible terminal or holding-key feel. Neither kind of Windows test establishes Linux compilation or Linux terminal acceptance.

### Linux checklist to run on a suitable machine

1. Compile with all listed warnings enabled and run `--self-test`.
2. Run interactively and test lowercase WASD plus uppercase equivalents.
3. Move to every edge and verify the marker stays inside.
4. Resize smaller and larger; test below the minimum size and recovery.
5. Quit with Q, then confirm the shell input, cursor and previous screen are usable.
6. Relaunch and use Ctrl+C, then a separate run with SIGTERM.
7. Record the Linux distribution, compiler version, terminal, commands and actual results.

### Known keyboard differences

Use WASD for this prototype. Linux arrow keys often send several bytes such as Escape, left bracket, capital A. The current byte reader does not decode or discard that complete sequence, so the trailing capital A may be interpreted as the A movement command. Similarly, an arrow sequence ending in D can cause right movement. Windows reads a structured non-ASCII arrow event instead. Do not claim Linux arrows are safely ignored or supported. Fixing that behavior requires treating escape sequences as a unit, not remapping the final letter.

Pasted text containing WASD can also produce moves because Linux presents terminal bytes rather than physical key identities. Unicode characters can span several bytes and are outside this control scheme. The code is suitable as a narrow WASD/Q teaching prototype; complete terminal key handling remains future work.

## 13 Presentation questions and practice

### Why not use scanf

`scanf` parses formatted input; it does not configure terminal line buffering, echo, or key-up/key-down behavior. A real-time movement prototype needs controlled terminal input plus a way to wait without freezing all other loop work. The selected wrappers provide that behavior explicitly.

### Why not use conio or ncurses

The current implementation uses no third-party terminal library. conio-style calls would not be the same Linux interface. ncurses could provide terminal abstraction but would change the dependencies and teaching surface. This small prototype directly exposes the OS operations being discussed in the course. That is a scope choice, not a claim that terminal libraries are inferior.

### Is the program multithreaded

The game loop does not create worker threads. Linux shutdown uses signals and Windows uses an OS callback, which is why shutdown flags still require care. The prototype does not demonstrate a shared-memory game simulation, distributed synchronization, or a client/server system.

### Does it access the keyboard hardware

No. It consumes events or bytes already supplied through the OS terminal interfaces. There is no keyboard driver, global key logger, or system-wide hook. Input belongs to the terminal attached to this process.

### Does fullscreen mean all display pixels

No. The program fills the current terminal viewport in character cells. Maximize the terminal for a large demonstration. It reserves a header and border and does not change the operating system's display mode.

### Practice before the presentation

Predict the result before running each example: D at 78,22 in 80x24; W at 1,2; shrinking 80x24 to 20x6; an unrelated Z; and Q. Then locate the responsible function in the actual source and explain both the returned result and changed state. Ask the partner to review the explanation.

For a small hands-on exercise, change the player marker in `render`, rebuild, and show why the movement rule remains unchanged. For a deeper exercise, draw the input path on paper for each OS without reading this guide. Explain where the program waits, where it mutates coordinates, and where it restores the terminal. Completion of this reading is not proof that either developer can perform those tasks yet.

## 14 Suggested oral explanation

"This is our first C terminal prototype. The player is a struct containing x and y coordinates. Both Windows and Linux use the same movement function: copy the current position, calculate a candidate, keep it inside the boundary, and commit it only if it changes. The terminal adapter is different on each OS. Windows reads console key-down records. Linux disables canonical line input and echo, waits with select, and reads a byte. The main loop redraws only after movement or resizing. Q or a handled interrupt takes the normal cleanup path so terminal settings are restored. We have Windows test evidence; Linux runtime acceptance is still pending."

## 15 References and files to read next

The source of truth for this guide is `src/movement_demo.c`. The practical run guide is `docs/MOVEMENT_DEMO.md`, and the per-function feature record is `docs/features/movement_demo.md`. The Linux launcher is `scripts/run_movement_linux.sh`. Refer to the linked primary API documentation above when checking a platform-specific detail. None of these documents establishes that the students have already demonstrated understanding or that the full campaign is implemented.

## 16 Return values memory and errors

Different interfaces use different success conventions. Our wrappers deliberately normalize platform operations for the caller. Do not assume that zero always means failure.

| Interface | Success | Failure or other result |
|---|---|---|
| Our `terminal_open`, `terminal_size`, `render` and `write_at` | Nonzero | Zero |
| Our `move_player` | One means position changed | Zero means no change, including a blocked move |
| Our `read_key` | Positive character byte | Zero means no actionable key; minus one means failure |
| POSIX `tcgetattr`, `tcsetattr`, `sigaction`, `ioctl` used here | Zero | Minus one; inspect errno where appropriate |
| POSIX `select` | Positive readiness count | Zero timeout; minus one error |
| POSIX `read` | Number of bytes read | Minus one error; zero requires interpretation for the resource and input mode |
| Win32 BOOL-returning calls used here | Nonzero | Zero; extended details normally through GetLastError |
| C `main` and `self_test` | EXIT_SUCCESS | EXIT_FAILURE |

`malloc((size_t)width)` converts the positive width to the unsigned size type expected for memory allocation. The result holds width bytes, not width integers. Allocation can fail and return NULL. The source checks that case and eventually calls `free(row)` after normal or failed drawing. Static strings such as the controls label are not allocated by malloc and must not be freed.

`strlen` counts bytes before a string's null terminator. `memset` writes a repeated byte, and `memcpy` copies a specified number of bytes. Neither adds a terminator automatically. `fwrite` takes an element size and element count; this source writes one-byte elements and checks the number written. `fflush(stdout)` asks the C stream to send buffered output onward; it is not a guarantee about physical display timing.

The Windows error wrapper currently reports a readable general failure rather than exposing each GetLastError code. POSIX input paths distinguish EINTR because an interrupted wait is expected during handled shutdown. Cleanup restoration calls are best effort rather than separately reported failures. Those choices are limits of this initial teaching prototype, not comprehensive production diagnostics.
