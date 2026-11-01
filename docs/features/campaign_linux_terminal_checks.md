# Controlled Linux terminal campaign checks

Actual verification: November 2, 2026, Asia/Manila. John approved the Maps 1 and 2 milestone plan. This note covers a new verification helper, not changes to the game rules. Codex authored the helper and ran it; both students' source review and teach-back remain pending.

## What ran and what passed

Command from the Windows checkout:

```powershell
python tests/linux_terminal_campaign_probe.py releases/linux-x86_64/2r1e --wsl docker-desktop
```

The helper ran the actual Linux binary in two separate Linux pseudo-terminals using the installed `script` utility. A pseudo-terminal supplies the same terminal input/output interfaces used by an interactive terminal. Both processes used the game's real UI, and Host launched its own background server. These were controlled hidden terminal sessions, not a human playtest or two visible windows presented to John.

Six assertion groups passed:

1. Main menu, Tutorial, Escape, arrow navigation back to Host, and background server launch.
2. A second actual UI selected Join and connected to the random test port. Both players became ready.
3. D moved the visible marker, a blocked W changed facing without moving, E read the Eye painting, I displayed its discovered note, and T messages reached both players. Incoming chat appeared while the host's journal stayed open.
4. The Archive word dialog accepted a wrong answer, displayed retry feedback, and then accepted `1 2 3` after reopening.
5. Shrinking the host to 60x15 displayed the resize warning. The partner sent a message while it was small; restoring 100x32 revealed that message and normal gameplay.
6. Q ended the session, the other UI received a session notice, both clients returned to the menu and exited, the Linux host listener disappeared, and both terminal input modes matched their saved values.

An exploratory Ctrl+C input made the game emit the cursor/alternate-screen restoration sequences, but also interrupted the surrounding test shell with status 130 before its terminal comparison could run. That observation is not counted as a passed signal/restoration assertion. The retained helper has no Ctrl+C acceptance claim.

No Windows execution policy was changed. The final ordinary MSVC rebuild subsequently passed the separate native Windows console checks, as recorded in `campaign_terminal_checks.md`; the earlier error 4551 is historical. This Linux check does not establish Windows behavior, physical two-computer LAN acceptance, complete campaign UI puzzle play, dial-dialog acceptance, SIGTERM/SIGQUIT/SIGTSTP behavior, student understanding, or professor acceptance. Full rule/progression and malformed-input coverage belongs to the separate C/socket checks.

## Files and every function

`tests/linux_terminal_campaign_probe.py` is new Python standard-library verification code. `docs/features/campaign_linux_terminal_checks.md` is this evidence and explanation record. No fixtures, assets, configuration, exports, game callers, or game source were changed for this helper.

| Function | Inputs and output | Execution, state and failures |
|---|---|---|
| `Terminal.__init__` | Linux executable path and optional WSL distribution; constructs one owned test session | Quotes the executable for the test shell, saves terminal settings, prints the terminal device name, runs the game and compares settings after normal exit. Starts `script` and one Python reader thread. Its process and accumulated output belong to this helper instance. Process errors propagate. |
| `Terminal.command` | Linux argument list; returns direct arguments or a WSL `--exec` wrapper | Preserves each command argument. `--exec` avoids a second shell interpreting the `script -c` command. It changes no game state. |
| `Terminal.read` | No arguments; no return value | The reader-thread callback reads real output until EOF and retains at most 262144 bytes. It observes output, never computes game rules or manufactures a screen. |
| `Terminal.screen` | No arguments; returns decoded latest rendered screen | Selects the output after the last actual clear-screen sequence. Replacement decoding tolerates diagnostics outside the ASCII game output. |
| `Terminal.wait` | Screen predicate and failure label; returns a matching screen | Polls for at most eight seconds, stopping early if the child exits. A timeout raises with the observed screen. Predicates only inspect real UI strings or the rendered marker/facing. |
| `Terminal.text` | Expected substring; returns the satisfying screen | Calls `wait` with a small lambda checking membership. A missing substring fails through the same bounded wait. |
| `Terminal.keys` | ASCII string or bytes; no return value | Writes actual terminal input and flushes it. Ordinary keys have a short interval; CSI arrow sequences are sent together so their bytes reach the game's escape-sequence reader within its deadline. Pipe failures propagate. |
| `Terminal.resize` | Row/column counts; no return value | Extracts the real `/dev/pts/N` device printed by the wrapper and calls `stty` against that device. Failure to find the device or resize it fails the test. It changes that test terminal's dimensions, not the player's machine settings globally. |
| `Terminal.finish` | No arguments; no return value | Waits for the owned process to exit successfully, joins its reader and requires the shell's `PROBE_RESTORED` marker. Nonzero status, timeout or unequal terminal modes fails. |
| `Terminal.close` | No arguments; no return value | Cleanup for `finally`: first sends cancel/quit/menu keys to a still-running owned child, then terminates that child only if the bounded graceful wait fails. It does not target unrelated game processes. |
| `main` | Optional command-line `--wsl DISTRO` plus executable path; no returned value | Reserves a random local port, creates two sessions, executes the six sequences and records each group after assertions pass. Checks the Linux listener table under WSL because the Windows forwarding relay can outlive the actual Linux listener briefly. Direct Linux runs check the closed port directly. Prints passed groups only after the entire sequence succeeds. All sessions are cleaned in `finally`. |

The script's `__main__` block calls `main`. The screen predicates inside `main` check actual marker and facing text. The host/join clients send all readiness, movement, chat and answer messages through their real C code; the Python helper never calls the rule functions or sends fabricated success messages.

## Concept and worked example

A terminal is an OS-managed input/output device with settings such as whether typed characters echo and whether input waits for Enter. A pseudo-terminal lets this test supply keys while still exercising terminal-aware C code. A Python thread here only collects output; it is distinct from the three game processes, which are two UI clients and one server.

Predict this sequence before reading the result: the Gallery begins at x=1, y=1. D requests x=2. A returns to x=1. W faces the upper wall. The observed marker remains on row 1, column 1 while the header says North. E at the Eye painting then follows key input, client action, authoritative server validation, private view and actual terminal drawing. I displays the note that the server permitted the player to discover.

For an oral defense: "We run two client processes and one server process. This controlled test types into Linux pseudo-terminals, so it checks the actual keyboard, rendering, networking and cleanup path rather than calling the game logic directly."

Both students should predict the blocked movement, trace `term_key`, `input_key`, the server validation and `draw_game`, then explain why a partner message arrives while the journal is open. Student changes, independent partner review and teach-back have not been recorded by this automated check.
