# Maps 1 and 2 milestone evidence report

Prepared November 2, 2026, Asia/Manila. Draft for student review, not a signed professor submission. Implementation began under John's October 31 approval of [the milestone plan](../plans/2026-10-31_MAPS_1_2_MILESTONE.md). Final verification below was rerun November 2. Developer A: John Francis Espiritu. Developer B, section, student reviewers and instructor remarks remain pending.

## Summary

The new C17 application has six rooms across three maps, menu/tutorial, WASD/arrows, facing/collision, object interactions, private discovered notes and short socket messages. Maps 1 and 2 have complete cooperative puzzle chains. Map 3 is a walkable preview with sealed unfinished controls. The inherited Python prototype and earlier C movement checkpoint are preserved.

Map 1 combines Gallery painting numbers with the Archive ordering clue and the feasible bracketed Three Blanks suggestion. The cabinet uses 246; its private instruction and the equations produce hatch answer 259. Map 2 has the uniquely correct statue, a passage that really opens, and split receiver/dial calibration at 2/3/4. Both players must activate their exits before each shared transition. The linked Google Doc was used as the puzzle reference; it was not edited.

## Important decisions and scope

One executable runs either a player UI or the authoritative server. Host launches an owned background server, and both players connect through actual TCP sockets. There are three processes and two player terminals. One nonblocking server event loop owns game state. Clients send intentions; server-side checks control identity, stage, position, ownership and prerequisites. Each player receives a permitted view, including their own clues. No threads or external backend are needed.

The approved source dependency is pinned cJSON 1.7.19 with its MIT license and nesting limit 8. No system dependency was installed. Native MSVC builds Windows locally; existing Zig cross-compiles a static Linux x86_64 musl release. Native Linux `cc` and Windows build scripts are included. Existing Windows executables remain local and all `.exe` files are excluded from GitHub.

John named Maps 1–2 the 50% milestone. This is a scope label, not a measured fraction of engineering effort or instructor acceptance. Full Map 3, gas, timed final activation and campaign victory are outside this approval.

## Work and authorship

Codex and subagents supplied the approved C implementation, tests and explanatory drafts. Independent component work covered game/content, networking, interface and terminal verification; a fresh read-only reviewer checked the integrated code. John approved the scope. No student code authorship, individual hours, partner review, teach-back or professor sign-off is inferred from these artifacts.

| Category | Evidence |
|---|---|
| Inherited baseline | Git `7f49abd`; Python prototype, old C movement source and executables preserved |
| New C rules/content | `src/game.c`, `src/content.c`, `tests/test_game.c`; [rules note](../features/campaign_rules.md) |
| New sockets/server/protocol | `src/net.c`, `src/server.c`, `src/protocol.c`; [network note](../features/campaign_network.md) |
| New terminal/client/entry/builds | `src/platform.c`, `src/client.c`, `src/main.c`, build scripts; [interface note](../features/campaign_interface.md) |
| Shared headers/configuration | [contracts note](../features/campaign_contracts.md) |
| Controlled terminal helpers | [Windows note](../features/campaign_terminal_checks.md), [Linux note](../features/campaign_linux_terminal_checks.md) |
| Student understanding demonstrated | Pending for both developers |

## Verification actually run

| Final check, November 2 | Actual result and limit |
|---|---|
| `scripts/build_game_windows.ps1` | Native MSVC C17, `/W4 /WX`, build and self-tests pass. 3207 rule assertions and 31 protocol checks. Tile assertions contribute to the large count. |
| Existing Zig 0.17.0, `cc -target x86_64-linux-musl -static -std=c17 -Wall -Wextra -Wpedantic -Werror -O2 -s -DCJSON_NESTING_LIMIT=8`, explicit source list and `-lm` | Static Linux x86_64 binary rebuilt; `--self-test` under WSL passes the same 3207 + 31 checks. This is cross-compilation, not a native `cc` build in that WSL distribution. |
| `python tests/integration_campaign.py submissions/game/2r1e.exe` | Full native Windows server campaign through Maps 1–2 into Map 3 with two socket clients passes. |
| `python tests/integration_campaign.py --wsl releases/linux-x86_64/2r1e` | Same checks against the actual Linux server through forwarded localhost pass. |
| Socket fault checks in both preceding runs | Private views, wrong/stale/forged input, malformed/duplicate fields, fragmentation/coalescing, busy port, third player, quit, abrupt disconnect, oversized incomplete line and bounded slow-reader termination pass. |
| `python tests/terminal_campaign_probe.py` | Ten native Windows console groups pass, including menu/tutorial, Host with socket partner, movement/facing, E/I/T, updates during notes, Q/Ctrl+C restoration and owned-server cleanup. Partner is a socket helper, not a second native Windows UI. |
| `python tests/linux_terminal_campaign_probe.py releases/linux-x86_64/2r1e --wsl docker-desktop` | Six groups pass with two actual Linux UI clients in pseudo-terminals: Host/Join, tutorial/arrows, movement/inspection/notes/chat, word-dialog retry/success, resize with network updates, Q/peer notification/listener cleanup and terminal-mode restoration. Controlled hidden tests, not human play. |
| Independent integrated source review | No blocking findings. The review checked authority, privacy, framing, queues, lifecycle, progression and readability. |

Earlier Windows executables were rejected by Application Control with error 4551. The final ordinary MSVC rebuild launches and passes the native checks without changing or bypassing policy. That earlier failure is historical, not the final status.

Linux release SHA-256: `b96e1bda2b1aacd824dc11bf4eeade4a6e9a6a45bf4e2ee9e7fb74e0db9604e1`.

Final staged review: the Linux binary has executable mode 100755, no Windows executable/object is staged, Python verification helpers compile, and whitespace checking passes for project-owned changes. Unmodified upstream cJSON retains a trailing space and license-end blank line reported by Git's whitespace check; those upstream files were preserved rather than silently edited.

## Acceptance evidence against the eleven plan conditions

Seven conditions have direct technical evidence: two socket clients complete the intended campaign; six layouts and passage reachability validate; prerequisite/retry rules hold; both-player transitions and preview stop hold; modal updates/resize/shutdown have controlled evidence; both builds and relevant checks pass; and release staging excludes Windows outputs and preserves the Linux executable mode.

Four conditions remain partial: both platforms' complete menu/Host/Join UI matrix, the complete answer/dial interaction matrix through actual UI, deterministic forced short-write/retry scheduling, and both students' learning/review requirement. The Windows Join menu and dial controls have code review and socket-level evidence, but not a complete native UI campaign run. Slow-reader backpressure is exercised, while every partial-send schedule is not forced. These are seven demonstrated technical conditions out of eleven, with four partial conditions, not seven professor-approved items or a grading percentage.

## Corrections, deviations and remaining issues

Integration review corrected a view refresh bug caused by comparing reused allocation addresses, preserved partially sent frames during shutdown, separated long puzzle dialogs from map rendering, and guarded input before the first valid view. These corrections stayed within the approved architecture and behavior.

No consequential scope deviation was made. Main-menu Tutorial currently needs Escape and reopening to refresh after a resize. Physical two-computer Linux LAN, human puzzle fairness/playtime, sanitizer runs, full UI campaign acceptance and student teach-back are unverified. The Linux release targets x86_64 Intel/AMD systems, not ARM.

## Demonstration and learning checklist for the students

Run the local Windows executable in two terminals. Choose Host in one, Join in the other, and keep 127.0.0.1 and matching ports. Show movement, facing, E inspection, I private notes and T messages. Solve both map chains and show that Map 3 remains unfinished. See [the run guide](../CAMPAIGN_RUN_GUIDE.md).

Before submission, both developers should trace a valid move and a rejected cabinet action, explain TCP framing and partial writes, locate the platform-specific keyboard reader, and explain why a client cannot declare puzzle success. Record actual presenter/reviewer names and results. Review these generated notes as reference material, then prepare the explanation in your own words.

Next check-in: November 4, 2026 under the confirmed grading calendar. Suggested next work is student review and two-person acceptance of this milestone. Full Map 3 requires a separately reviewed scope and approval. Signatures and instructor remarks remain pending.
