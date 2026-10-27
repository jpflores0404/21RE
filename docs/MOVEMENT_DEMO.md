# C movement demo

Approved by John Francis Espiritu on October 27, 2026: AI may write this first small demo and explanatory notes. This is a local movement prototype, not the networked game. The inherited Python application is unchanged.

## Run on Windows

Open PowerShell in the repository folder and run:

```powershell
& ./submissions/movement-demo/movement_demo.exe
```

The executable is a local build in the already ignored `submissions/` directory. It is not a tracked source file. Rebuild from `src/movement_demo.c` when sharing source or moving computers.

Maximize Windows Terminal before running for a fullscreen-sized demonstration. The program fills its current terminal viewport; it does not change the desktop display mode. Use a real interactive terminal rather than an editor output panel. Use at least 20 columns by 6 rows; 80 by 24 or larger is recommended.

- W moves up, A left, S down and D right. Uppercase also works. Enter is unnecessary.
- `@` is the player; `#` marks the movement boundary. One input event moves one cell. Holding a key uses the terminal's normal repeat behavior.
- Q exits. Ctrl+C also requests a clean exit. The previous Windows screen buffer and input mode are restored.
- Resizing changes the available play area. The player is clamped inside the new bounds. A very small terminal pauses movement and displays an enlargement prompt; Q still works.

There are no map files, puzzles, interaction keys, multiplayer connections or timers in this demo. The header and border reserve a small amount of terminal space. The final bottom-right cell is intentionally left unwritten to avoid unwanted scrolling on POSIX terminals.

## Rebuild with the existing Windows compiler

The machine has an MSVC compiler even though Visual Studio Installer reports an incomplete Build Tools installation. This exact invocation succeeded locally from PowerShell:

```powershell
New-Item -ItemType Directory -Force submissions/movement-demo | Out-Null
cmd /d /s /c '"C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvars64.bat" >nul && cl /nologo /std:c17 /W4 /WX /TC src\movement_demo.c /Fosubmissions\movement-demo\movement_demo.obj /Fesubmissions\movement-demo\movement_demo.exe'
& ./submissions/movement-demo/movement_demo.exe --self-test
```

The versioned compiler path belongs to this computer. On another Windows computer with MSVC installed, use its Developer Command Prompt and run the `cl` command above, or change the setup path to its actual installation. No compiler was installed for this task. `/TC` selects C, `/std:c17` selects the language standard, `/W4` enables warnings and `/WX` treats warnings as errors.

## Planned Linux build check

The Linux branch is already in `src/movement_demo.c`; build it on Linux rather than running the Windows executable. The compiler selects the POSIX code automatically. A new convenience launcher can compile and run it from any working directory:

```sh
sh scripts/run_movement_linux.sh --self-test
sh scripts/run_movement_linux.sh
```

See [the detailed Linux and Windows comparison](LINUX_WINDOWS_MOVEMENT_GUIDE.md) for headers, every function, input modes, byte/event processing, drawing, cleanup, traces and oral-defense practice. Use WASD: Linux escape-sequence keys are not decoded and can accidentally be interpreted as movement letters. Linux suspend/resume and SIGQUIT cleanup are also outside this narrow demo.

The POSIX source branch is included, but Linux compilation and interactive behavior have not been verified here. On a Linux machine with a C compiler, run from the repository root:

```sh
mkdir -p submissions/movement-demo
cc -std=c17 -Wall -Wextra -Wpedantic -Werror src/movement_demo.c -o submissions/movement-demo/movement_demo
./submissions/movement-demo/movement_demo --self-test
./submissions/movement-demo/movement_demo
```

These Linux commands are proposed instructions, not evidence of a completed Linux test. No terminal library is required; the Linux branch uses terminal modes, escape sequences, `select`, signals and terminal-size queries.

## Check before the October 28 demonstration

1. Run `--self-test`; the expected output is `Movement self-test passed (14 checks).`
2. Run interactively. Demonstrate W, A, S, D without Enter and explain how x/y change.
3. Walk to all four edges. The player must stop inside the border.
4. Shrink and expand the terminal. The player must remain visible or the enlargement prompt must appear.
5. Press Q and confirm ordinary terminal input and the earlier screen return. Repeat once with Ctrl+C.
6. Record the actual presenter, terminal, result and screenshot. Do not mark these manual checks complete just because automated movement checks passed.

## Verification recorded October 27

- Native Windows build succeeded using MSVC 14.50.35717, C17, `/W4 /WX`, with no compiler warnings.
- The executable's 14 pure movement checks passed. They cover all directions, uppercase input, invalid input, each boundary, resize clamping, too-small dimensions and enlargement without unintended movement.
- A controlled hidden native Windows console probe injected actual WASD/Q events and read the screen buffer. It confirmed movement, all four boundaries, shrinking to 40x12, pausing in a 10x4 viewport, enlargement, Q and Ctrl+C exits, and restoration of input modes and the original screen. This is automated console evidence, not a human playtest in Windows Terminal.
- Invalid arguments and redirected/noninteractive execution were rejected with exit code 1 and readable messages.
- Human terminal acceptance and Linux builds are separate checks. See the feature note for their current status and the code walkthrough.

See [the feature explanation](features/movement_demo.md) for every function, the input-to-output trace, OS concepts and pending student teach-back.
