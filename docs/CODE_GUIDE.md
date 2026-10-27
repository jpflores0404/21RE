# Beginner guide to the inherited code

This explains commit `6bfedc8`, not the proposed authoritative-server implementation. No game code was changed by the October 23 review. Read a section beside the real file, predict an example, run a relevant check, then explain it to your partner. Reading this guide is not evidence of understanding by itself.

## First vocabulary

A **module** is a Python file. A **function** is a named operation. Its **parameters** are the values it receives, and its **return value** is the answer it gives its caller. A **class** groups data and related functions; an **instance** is one object created from it. A **method** is a function on such an object. `self` means that object. `__init__` initializes it and returns `None`.

A **dictionary** maps keys to values, such as `{"walk": False}`. A **list** keeps values in order. A **set** keeps unique values, useful for finished tasks. A **tuple** groups fixed-position values such as `(x, y)`. **State** means values that change during play, such as position and completed tasks. A **callback** is a function passed to another function to call later.

An **exception** reports a failure. `raise` signals it; `try/except` handles selected failures; `finally` runs cleanup. An **import** loads another module, including its top-level code. This explains why importing a drawing constant in a test can fail on Windows before any test runs.

Coordinates use `x` for column and `y` for row, starting at zero in the upper-left. Right adds 1 to x, down adds 1 to y. `None` means no value. `True`/`False` are yes/no values. Strings hold text. JSON is a text format for structured values, not a guarantee that received data is valid for the game.

For screen functions below, `stdscr` is the main curses screen, `win` is a drawing area inside it, `key` is an integer key code, and `attr` is a drawing style. Drawing functions generally return `None` and change terminal output. A `task` is the dictionary loaded from a room objects JSON file. A `link` is either local loopback or a peer connection. There is no authoritative server in this baseline.

## Follow one action first

1. `main.main()` uses `curses.wrapper` to open the terminal safely.
2. `client.app.run()` shows the menu and loads the Records Wing.
3. `run_game()` reads a key, then `handle_key()` chooses an action.
4. Movement calls `RoomSession.move()` -> `try_move()`. It changes position locally if the tile permits it.
5. E calls `RoomSession.interact()` -> a `Result`. A result can contain text, an art name, a task, or an escape flag.
6. `use()` opens a local mini-game. If it returns true, it updates `Team` and sends a completion claim.
7. The partner's background reader queues that JSON message. Their main loop calls `handle_messages()` -> `Team.apply()`.
8. `draw()` redraws the map, progress and dialogue using local state.

Important limit: the partner trusts the completion claim. `Team.apply()` does not validate the answer, location or identity. See the audit before presenting this as the target networking design.

## Entry and rule files

### `main.py`

`main()` takes no parameters. It sets the default Esc delay, imports the application, runs it inside `curses.wrapper`, handles keyboard interruption and prints a farewell. It returns `None`. The wrapper restores the terminal after normal exit or an exception; it does not prevent gameplay exceptions. The file's old "single-player movement prototype" docstring is stale relative to its host/join menu.

### `game/tiles.py`

No functions. `SPAWN` is `@`, `FLOOR` is `.`, and `TILES` is the shared legend. Each tile record provides a name, whether it is walkable, a visual style, and optional drawing/bump/inspection text. A locked door has `walk=False`; it does not automatically become floor when a task completes. A dictionary entry describing a door is not a door-unlocking system.

### `game/game_map.py`

This file loads tile rows and defines movement without terminal or network imports.

| Function/class | Inputs -> result | What happens and what can fail |
|---|---|---|
| `MapError` | A specialized `ValueError` | Names a bad map problem; adds no methods |
| `GameMap.__init__(rows)` | List of map strings -> initialized map | Pads short rows with solid void, scans every character, finds exactly one spawn, replaces `@` with floor, stores rows/width/height. Raises `MapError` for empty map, unknown tile, missing or duplicate spawn |
| `GameMap.tile(x, y)` | Coordinates -> tile dictionary or `None` | Checks bounds before indexing. Returns `None` outside the rectangle |
| `GameMap.can_walk(x, y)` | Coordinates -> bool | Requires a real tile with `walk=True`; changes no state |
| `load_map(path)` | Text file path -> `GameMap` | Reads UTF-8, strips trailing whitespace, splits rows and invokes validation. File errors and map errors propagate |
| `Player.__init__(x, y)` | Starting coordinates -> player | Stores position, faces down, sets step counter to zero |
| `try_move(game_map, player, direction)` | Map, player, direction string -> `(moved, message)` | Rejects unknown direction. Otherwise changes facing first, computes one adjacent tile, rejects boundary/solid tiles, or changes position and increments steps. Blocked movement still turns the player |

Worked trace: in `tests/test_movement.py`'s small map, player `(1,1)` moves right to `(2,1)`, steps becomes 1. Another right targets a pedestal and fails, so position stays `(2,1)` but facing stays right. This facing is what selects an object for E/F.

### `game/level.py`

A **level** contains rooms, and each room contains a tile map and object metadata. `MAPS_DIR` is resolved relative to this source file, so changing the terminal's working directory does not change asset lookup.

| Function | Inputs -> result | State and important behavior |
|---|---|---|
| `Room.__init__(player_id, info, game_map, objects)` | Player number, metadata, map, object list -> room | Stores name/introduction/objective, filters task objects, builds `at[(x,y)] = object` for fast facing lookup; one large object can occupy multiple tiles |
| `Level.__init__(title, rooms)` | Title and player-to-room dictionary -> level | Holds the supplied title and rooms |
| `Level.all_tasks()` | No extra input -> task list | Combines all rooms' task lists; no mutation |
| `Level.task_ids()` | No extra input -> ID list | Extracts each task's `id` |
| `Level.task_title(task_id)` | ID -> title or original ID | Searches tasks for a display name; fallback avoids losing a readable message for an unknown ID, but does not validate that ID |
| `read_json(path)` | File path -> decoded JSON value | Reads UTF-8 and parses JSON; missing/bad file raises errors |
| `load_level(level_id, maps_dir=MAPS_DIR)` | Folder name and optional base path -> `Level` | Loads manifest, then every room's map/objects, checks objects/tasks, converts room keys to integers. Currently loads both players' secrets in each client |
| `check_objects(objects, game_map)` | Object list and map -> `None` or `ValueError` | Checks known kind, container item, task type/title/answer, solid coordinates and overlapping objects. Not a full arbitrary-JSON schema validator; missing fields can raise other errors |
| `check_tasks(level)` | Loaded level -> `None` or `ValueError` | Rejects duplicate task IDs and nonexistent `needs_task`; does not detect dependency cycles or validate network input |

`Room.at` is an index, not another game object. Looking up `(41,8)` and `(42,8)` can return the same desk dictionary. That is why opening either side opens the same container ID.

### `game/session.py`

This file holds one player's room, position, items, clues and opened containers. It provides rules for E and F. It is currently called inside a client.

| Function | Inputs -> result | State and important behavior |
|---|---|---|
| `Result.__init__(title, lines, art=None, task=None, escape=False)` | Display/action fields -> result | Copies text lines and stores optional artwork/task/escape request. Does not draw or send anything |
| `RoomSession.__init__(room)` | Room -> session | Creates player at spawn; initializes empty items/clues lists and opened-ID set |
| `move(direction)` | Direction -> movement tuple | Delegates to `try_move`; changes the contained player only as that function allows |
| `faced_position()` | Current player -> `(x,y)` | Adds the facing direction vector to player coordinates; does not move |
| `faced_object()` | Current facing -> object dict or `None` | Uses the room's coordinate index |
| `faced_name()` | Current facing -> display name or `None` | Prefers explicit object name, otherwise a descriptive tile name |
| `add_clue(clue)` | Text -> bool | Appends only if not already in the clue list; true means newly saved |
| `interact(team)` | Current shared progress -> `Result` or `None` | Dispatches by object kind to container/task/exit behavior; ordinary examine returns text; empty object position falls back to tile description |
| `plain_tile()` | Current facing -> description result or `None` | Allows furniture without object metadata to respond; no state change |
| `open_container(obj)` | Container dict -> result | If already opened, returns after-text. Otherwise records ID and adds item once, optionally prompts F for revealed art |
| `use_task(task, team)` | Task and team -> result | Reports already done/missing item/missing prerequisite. Otherwise returns a result containing the task; this only starts a mini-game, not proves completion |
| `use_exit(obj, team)` | Exit and team -> result | Returns sealed/count text until all tasks done; then returns `escape=True`. This method itself does not add the player to escaped |
| `inspect(team)` | Team -> result or `None` | Closed container conceals details; opened container can reveal different art. Copies inspection text, adds exit progress and deduplicates clue. Rendering happens elsewhere |

Try this trace: `open_container(curator_desk)` changes `opened` and `items`, then `inspect()` adds its clue once. Repeating either should not duplicate the item or clue. Compare `tests.test_interaction`.

### `game/team.py`

Each peer maintains a separate `Team`; these are not two references to one shared object across computers. Sending events is how the baseline tries to keep the copies consistent.

| Function | Inputs -> result | State and limits |
|---|---|---|
| `Team.__init__(task_ids)` | Expected task IDs -> team | Copies IDs; starts empty `done` and `escaped` sets |
| `finish(task_id)` | ID -> `None` | Adds any supplied value to the set. No player/prerequisite validation; unhashable values fail |
| `is_done(task_id)` | ID -> bool | Set membership check |
| `done_count()` | No extra input -> integer | Counts expected IDs in `done`; unknown IDs do not raise the displayed count |
| `all_done()` | No extra input -> bool | Compares count with expected task total |
| `escape(player_id)` | Player ID -> `None` | Adds ID without validating eligibility |
| `has_won()` | No extra input -> bool | Requires all expected tasks and IDs 1 and 2 in escaped |
| `apply(message)` | Peer message -> `None` | Directly reads `type` and task/escape fields, calls `finish`/`escape`; unknown types do nothing. Missing/wrong fields can raise errors |

Sets make repeated completion claims idempotent, meaning repeating the same claim has the same final result. Idempotence does not establish that the first claim was legal.

## Client files

### `client/app.py`

`run(stdscr)` initializes curses and colors, repeats the main menu, obtains a local/host/join link and fixed player number, loads a fresh Records Wing and calls `run_game`. Its `finally` closes the link even after game failure. It returns `None` when Quit is selected. `START_LEVEL` fixes the single level; the loader's ability to read another folder does not implement a campaign transition.

### `client/network.py`

A **socket** is a communication endpoint. TCP delivers an ordered byte stream. Newline framing tells the receiver where one JSON message ends. A background **thread** runs while the UI handles keys. `queue.Queue` passes its messages to the UI safely; it does not validate their game meaning.

| Function | Inputs -> result | State and failure behavior |
|---|---|---|
| `LocalLink.__init__()` | None -> local link | Initializes a pending-message list |
| `LocalLink.send(message)` | Any message -> `None` | Appends the same object to pending; no encoding or network |
| `LocalLink.receive()` | None -> list | Returns all pending messages and replaces pending with a fresh empty list |
| `LocalLink.close()` | None -> `None` | No resource to close |
| `NetLink.__init__(sock)` | Connected socket -> link | Stores socket, creates queue and starts daemon reader thread; daemon means it does not keep process alive by itself |
| `NetLink.read_loop()` | Stored socket -> `None` | Reads text lines using a file wrapper, parses JSON, enqueues values. OSError/invalid JSON stops reading; then enqueues `bye`. Valid JSON has no schema or size validation |
| `NetLink.send(message)` | JSON-compatible value -> `None` | Serializes, appends newline, encodes UTF-8 and uses `sendall`; socket failure queues `bye`. Sending can block; serialization errors are not caught here |
| `NetLink.receive()` | None -> list | Drains currently queued values with no wait for future input |
| `NetLink.close()` | None -> `None` | Attempts bidirectional socket shutdown, tolerates shutdown OSError, then closes it |
| `start_hosting(port=5050)` | Port -> listening socket | Creates TCP listener on all interfaces, enables address reuse, uses backlog 1 and 0.2-second accept timeout; bind/listen errors propagate |
| `check_for_partner(server)` | Listener -> `NetLink` or `None` | Accepts a connection, returns None on timeout, removes accepted-socket timeout and starts NetLink |
| `join(ip, port=5050)` | Host address/port -> `NetLink` | Connects with five-second timeout, clears timeout on success, starts reader; connection OSError propagates |
| `my_ip()` | None -> guessed address string | Connects a UDP socket to a public destination to choose local interface, sends no payload, returns its local address, falls back to loopback on error, closes socket in `finally` |

`listen(1)` is a connection backlog, not proof of a fully implemented two-player session policy. `my_ip()` also is not an internet service call; however, on an isolated LAN its route-based guess may return loopback or the wrong interface. Confirm the actual LAN IP in network settings for the demo.

Try this: send half a JSON line, then the rest and newline. The line reader waits until the terminator. Sending two lines at once produces two values. Passing `[]` still produces a decoded value, so the receiver must separately check the expected message dictionary.

### `client/lobby.py`

| Function | Inputs -> result | Behavior |
|---|---|---|
| `host_game(stdscr)` | Screen -> connected `NetLink` or `None` | Handles port-open error, draws guessed IP, repeatedly accepts with a timeout, permits cancel, then always closes listener and restores key wait mode |
| `join_game(stdscr)` | Screen -> `NetLink` or `None` | Gets typed IP, substitutes localhost if blank, displays connection status, handles connection errors with a readable message |

These are Host/Join screens, not an assigned-ID/readiness protocol. Player number is chosen in `app.run`.

### `client/dialogue.py`

| Function | Inputs -> result | Behavior |
|---|---|---|
| `Dialogue.__init__(title, lines)` | Title/text list -> dialogue | Wraps to 74 columns, splits into three-line pages, ensures one empty page if needed, starts at page zero |
| `lines()` | Current page -> line list | Returns page content without changing it |
| `is_last_page()` | Current state -> bool | Compares page index to final index |
| `next_page()` | None -> `None` | Increments index; caller must avoid advancing beyond last page |

### `client/menu.py`

| Function | Inputs -> result | Behavior |
|---|---|---|
| `main_menu(stdscr)` | Screen -> menu ID string | Waits for keys, wraps selected index on up/down, returns selected action or quit |
| `draw_menu(win, selected)` | Window/index -> `None` | Draws title, menu highlight, two character sprites and controls |
| `show_help(stdscr)` | Screen -> `None` | Draws control/help text and blocks until a non-resize key. Does not process peer updates while open |

`MENU_ITEMS` pairs visible labels with IDs used by `app.run`. `HOW_TO_PLAY` is display content, not the source of game rules.

### `client/game_screen.py`

This is the main input/rendering coordinator. It is one of the largest files; learn its input functions before its drawing functions.

| Function | Inputs -> result | State and important behavior |
|---|---|---|
| `key_to_direction(key)` | Key code -> direction string or `None` | Uses shared movement key groups |
| `is_key(key, letter)` | Key and letter -> bool | Accepts upper/lower case |
| `Game.__init__(level, player_id, link, local)` | Level, current player, transport, local flag -> game state | Creates Team, wall-clock start time, one session for LAN or two for local, intro dialogue and pending notices. The supplied level still contains both rooms |
| `Game.session()` | None -> current `RoomSession` | Looks up current player |
| `Game.partner_id()` | None -> other player ID | Maps 1 to 2 and 2 to 1 |
| `Game.has_escaped()` | None -> bool | Checks current player's escaped membership |
| `run_game(stdscr, level, player_id, link, local)` | Game setup -> `None` on end | Loops over messages/win/dialogue/draw/input, polls main keys every 100 ms; exits on partner left, quit or win. Dialogue consumes navigation keys before gameplay |
| `handle_messages(stdscr, game)` | Screen/state -> bool | Drains inbox, returns false for bye after showing notice, trusts `Team.apply`, queues chat/clue dialogue and updates journals. Assumes correct message shape |
| `handle_key(stdscr, game, key)` | Key/state -> `None` | Dispatches help, local Tab, movement, E/F, journal sharing, chat; escaped player cannot act except earlier help/Tab branches |
| `use(stdscr, game, result)` | Rule result/state -> `None` | Shows notice/text, runs task, locally marks and sends completion, or locally escapes and sends escape. Local mode switches partner after first exit |
| `switch_player(game)` | State -> `None` | Tab selects other local session unless escaped, then shows explanatory dialogue |
| `chat(stdscr, game)` | Screen/state -> `None` | Local mode reports verbal play; LAN opens text input and sends nonempty text |
| `draw(stdscr, game)` | Screen/state -> `None` | Obtains sized frame, calls drawing functions and controls bar, refreshes terminal |
| `draw_status(win, game)` | Window/state -> `None` | Shows team fraction/progress, LAN/local and player number |
| `draw_map(win, game)` | Window/state -> `None` | Iterates map rows/columns, chooses tile char/style, colors done tasks/eligible exits, highlights faced tile, draws @ unless escaped. Does not change tile walkability |
| `draw_panel(win, game)` | Window/state -> `None` | Draws sprite/room, up to four local tasks, done/waiting text and item/clue counts |
| `draw_dialogue(win, game)` | Window/state -> `None` | Draws current dialogue page or objective/escape wait text; truncates ordinary objective display to width |
| `draw_hint(win, game)` | Window/state -> `None` | Draws notice first, otherwise faced-object prompt |

The 100 ms poll applies only while control is in `run_game`. A task or journal can keep control in another function. Do not say the game processes incoming changes continuously while every screen is open.

### `client/tasks.py`

Each mini-game returns `True` on completion or `False` on leaving. This makes its caller small, but currently lets a client decide success. The planned server must validate puzzle input independently.

| Function | Inputs -> result | State and important behavior |
|---|---|---|
| `run_task(stdscr, task)` | Screen/task -> bool | Looks up mini-game by task type, sets blocking input, runs it, restores input wait, shows completion if true. Unknown task type causes lookup failure |
| `task_window(stdscr, task, keys)` | Screen/task/control list -> window or `None` | Uses common size check, draws task border and controls |
| `show_task_complete(stdscr, task)` | Screen/task -> `None` | Draws completion box, pauses one second and flushes pending keys; no network application during this call |
| `play_wires(stdscr, task)` | Screen/task -> bool | Shuffles two color-ID lists; tracks selected side, cursor, picked wire and fixed set. Matching IDs join a wire; four fixed wins; Q leaves |
| `draw_wires(win, left, right, fixed, side, cursor, picked, message)` | Window and current wire state -> `None` | Draws labeled wire ends, highlight and connection paths; labels reduce reliance on color |
| `play_swipe(stdscr, task)` | Screen/task -> bool | Advances position on timed key reads, accepts a shared Enter-key group inside allowed zone, reports early/late or leaves. Controls advertise Space but Enter/E also belong to that group |
| `draw_swipe(win, position, message)` | Window/position/feedback -> `None` | Draws card/track, `=` target zone, moving marker and text |
| `play_transfer(stdscr, task)` | Screen/task -> bool | On start stores `time.time`, computes elapsed/seconds, redraws every 100 ms, returns true at full bar, Q cancels. Download/upload differ only in presentation here |
| `draw_transfer(win, uploading, progress, running, seconds)` | Window and transfer state -> `None` | Draws direction, progress, estimated time and completion text |
| `play_code(stdscr, task)` | Screen/task -> bool | Reads local `answer`, appends bounded digits, backspaces, compares on Enter; wrong clears text and retries, Q leaves. It is client validation, not server validation |
| `draw_code(win, answer, typed, message)` | Window, answer-length source, entered text, feedback -> `None` | Draws blanks/digits/keypad and error text; answer length determines number of blanks |
| `play_breaker(stdscr, task)` | Screen/task -> bool | Shuffles numbers 1-10 in 2x5 positions, tracks next required number and cursor. Wrong order resets progress; already-pressed numbers only give feedback |
| `draw_breaker(win, numbers, next_number, cursor, message)` | Window/current breaker state -> `None` | Draws buttons, selected/finished styling and next-number feedback |

Random wire/breaker layout is local presentation. Future tests should use fixed fixtures/controlled randomness when checking a precise sequence. Client progress-bar elapsed time is not the required authoritative gas deadline.

### `client/screens.py`

| Function | Inputs -> result | Behavior and limits |
|---|---|---|
| `load_art(name)` | Artwork stem -> line list | Reads `assets/art/<name>.txt`; returns a missing-art message if absent. Other file/encoding errors propagate |
| `wait_for_key(stdscr, draw)` | Screen and drawing callback -> key code | Uses blocking keyboard input, redraws on resize, returns the next other key. Does not service game messages |
| `show_message(stdscr, title, lines)` | Screen/text -> `None` | Creates nested `draw(win)` callback to wrap/center text, passes it to wait_for_key |
| `show_inspect(stdscr, result)` | Screen/result -> `None` | Loads art, creates nested `draw(win)` for up to 13 art lines and wrapped details, waits for any key. Long text can be clipped; no multi-page inspection scroll |
| `show_journal(stdscr, session, partner_name)` | Screen/session -> clue string or `None` | Creates nested `draw(win)` for items/clue list; moves selection, returns chosen text on Enter or None on exit. Sending occurs in caller |
| `ask_text(stdscr, title, question, max_length=60, start_text='')` | Screen/prompt/options -> stripped text or `None` | Creates nested `draw(win)` for prompt/cursor; handles printable characters, backspace, Enter/Esc. `finally` hides cursor again |
| `show_win(stdscr, seconds)` | Screen/elapsed seconds -> `None` | Converts time to minutes/seconds; creates nested `draw(win)` for banner/time, clears buffered keys and waits for key |

Each nested `draw(win)` is a callback that reads its containing function's local values. It draws only and returns `None`. This is a small use of a **closure**, a function retaining access to surrounding variables; no extra UI framework is involved.

`ART_DIR` is just an asset path. `tests/test_interaction.py` imports it from this drawing module, pulling in curses unnecessarily for a rule test. A future approved small correction can move that path lookup to a platform-independent location.

### `client/ui.py`

| Function | Inputs -> result | Behavior |
|---|---|---|
| `init_colors()` | Terminal capability -> `None` | Initializes color pairs only when supported; attempts default background, falls back to black |
| `color(pair)` | Pair ID -> curses style or 0 | Returns no-color style if terminal lacks color |
| `write(win, y, x, text, attr=0)` | Window/position/text/style -> `None` | Rejects out-of-range row, clips left/right, skips empty text, catches curses drawing errors including lower-right boundary |
| `write_center(win, y, text, attr=0)` | Window/row/text/style -> `None` | Calculates centered x and delegates to safe write |
| `draw_box(win, y, x, h, w, title='', attr=0)` | Rectangle and optional title -> `None` | Draws ASCII border and title through write |
| `draw_key_bar(win, y, keys)` | Row and `(key,label)` pairs -> `None` | Clears row then draws keyboard help labels |
| `progress_bar(done, total, width)` | Completed/total/width -> string | Rounds filled fraction, handles zero total; caller must supply sensible bounded progress |
| `frame(stdscr)` | Main screen -> centered 80x24 subwindow or `None` | Clears screen, checks dimensions; too small shows clear resize/Q message instead of returning a drawing window |

Shared key tuples map both arrows and WASD. `ENTER_KEYS` includes E, Space and Enter; a caller using it accepts all of them unless it adds its own condition.

### `client/sprites.py`

`get_sprite(facing, frame)` returns a list of ASCII rows for the requested direction plus alternating legs. Unknown facing falls back to down. `frame % len(LEGS)` cycles the leg pose. No game state changes; the caller supplies the step/frame number.

### Package marker files

`game/__init__.py`, `client/__init__.py` and `tests/__init__.py` make their folders importable Python packages. They contain only descriptive text or are empty and define no functions. They are not application entry points.

## Tests, function by function

`unittest` discovers methods whose names begin `test_`. Each assertion is a check: unequal observed/expected values fail the test. A `setUp()` creates fresh state before each test. It is not evidence of a full playthrough. Existing tests total 33, split 15/7/11.

### `tests/test_movement.py`

`MovementTests.setUp()` creates the small test map/player. `RecordsWingMapTests.setUp()` loads actual Records Wing assets. Both return `None` and reset state for independent tests.

| Test method | What its assertions prove |
|---|---|
| `test_spawn_found_and_becomes_floor` | Spawn detected and walkable after replacing @ |
| `test_move_onto_floor` | One legal step changes position and increments steps |
| `test_wall_blocks_movement` | Wall keeps position and supplies bump message |
| `test_blocked_move_still_turns_player` | Failed step still changes facing |
| `test_object_blocks_movement` | Pedestal prevents a step |
| `test_cannot_leave_the_map` | Boundary checked even on a tiny open map |
| `test_unknown_direction_rejected` | Invalid direction returns false |
| `test_unknown_tile_rejected` | Unknown character raises MapError |
| `test_missing_spawn_rejected` | Missing @ raises MapError |
| `test_two_spawns_rejected` | Duplicate @ raises MapError |
| `test_short_rows_filled_with_void` | Short row padded with unwalkable void |
| `test_both_rooms_load` | Player slots 1 and 2 exist |
| `test_rooms_are_closed_in` | No outer-edge tile is walkable |
| `test_every_floor_tile_reachable_from_spawn` | Flood fill visits all walkable tiles; `seen` prevents repeated work and `todo` holds cells still to explore |
| `test_every_object_can_be_reached` | Each object has some adjacent walkable tile; combined with floor-reachability test, players can approach it |

### `tests/test_interaction.py`

`stand(session, x, y, facing)` directly changes test-player coordinates/facing and returns None. It bypasses movement intentionally for a focused interaction test, not gameplay authority. `GalleryTests.setUp()` loads actual assets, team and gallery session.

| Test method | What its assertions prove |
|---|---|
| `test_use_painting_shows_text_without_clue` | E gives basic text; does not save F-only clue |
| `test_inspect_painting_saves_clue_once` | Repeated F returns art and avoids duplicate clue |
| `test_container_opens_once_and_gives_item` | Closed/open inspection and repeated use keep item/clue count correct |
| `test_big_object_is_the_same_from_either_tile` | Alternate desk coordinate resolves to same named object |
| `test_nothing_to_use_on_open_floor` | No interactable thing returns None |
| `test_plain_bookshelf_has_a_description` | Bare tile furniture still has description |
| `test_all_art_files_exist` | All art/reveal_art stems point at actual files; does not inspect terminal rendering |

### `tests/test_tasks.py`

`TaskTests.setUp()` loads level, team and both sessions. `ExitTests.setUp()` creates team/gallery and faces exit. `wait_for(link)` polls receive at 10 ms intervals up to 200 attempts and returns the first nonempty batch, or empty list on timeout. This helper limits waiting for asynchronous socket reads.

| Test method | What its assertions prove |
|---|---|
| `test_each_player_has_four_tasks` | Alternate design contains four tasks per room |
| `test_wiring_panel_starts_a_task` | E at wiring station returns task data |
| `test_swipe_card_needs_the_keycard` | Local requirement blocks task until item collected |
| `test_upload_waits_for_partner_download` | Team flag gates upload |
| `test_finished_task_cannot_be_started_again` | Local completed task no longer launches |
| `test_codes_match_the_clues` | Stored answers match current literals `246` and `1259`; does not check master conformity or play a keypad |
| `test_exit_is_sealed_until_all_tasks_are_done` | Initially sealed with 0/8 feedback |
| `test_exit_opens_when_all_tasks_are_done` | Manually setting all task flags allows local escape result |
| `test_team_wins_only_when_both_players_escape` | Win needs both IDs after all flags; manually sets tasks, not real gameplay |
| `test_local_link_gives_messages_back` | Local send/receive returns one batch then empty |
| `test_two_computers_can_talk` | Creates real host/join sockets on **one computer**, exchanges one task then bye on close; name does not prove two-computer testing |

No existing automated test plays every curses mini-game, checks the authoritative-server contract, exercises the missing campaign/hazards, or establishes human understanding.

## Assets and documentation files

| File/group | Purpose and loader |
|---|---|
| `assets/maps/records_wing/level.json` | Manifest names both rooms' map/object files, title, introduction and objective; read by load_level |
| `gallery.txt`, `archive.txt` | ASCII tile grids with one @ each; read by load_map |
| `gallery_objects.json`, `archive_objects.json` | Object coordinates/kinds/text/clues/items/task data, including current local answer keys; read by load_level |
| `assets/art/painting_watcher.txt`, `painting_key.txt`, `painting_lantern.txt` | Paintings shown by load_art when inspected |
| `archive_document.txt`, `logbook.txt` | Archive ordering clues shown as art |
| `curator_note.txt`, `archivist_note.txt` | Revealed note artwork |
| `keypad.txt`, `exit_panel.txt` | Close-up interface artwork |
| `.gitignore` | Excludes Python caches, OS metadata and submissions folder; not a runtime rule |
| `README.md`, `docs/user_manual.md` | Describe how to run/play the inherited variant; new review notice separates it from the master target |
| `docs/IMPLEMENTATION_PLAN.md` | Inherited four-role plan, historical reference; confirmed dates retained, future steps replaced by proposed learning roadmap |
| `docs/design/concept.md`, `puzzle_flow_map1.md`, `map1_sketch.txt` | Inherited task-based concept, solutions and ASCII planning sketch |
| `docs/design/architecture.md`, `protocol.md`, `network_research.md` | Inherited peer design and transport reasoning; read audit for authority limits |
| `docs/design/objects_format.md`, `tiles.md` | Meaning of current content fields/tile symbols |
| `docs/design/wireframes.md`, `ui_research.md` | Terminal layout and curses choices |
| `docs/design/map2_concept.md` | Inherited optional Boiler Floor idea, not required Observation Wing |
| `docs/test_plan.md`, `test_report.md` | Original checklist and unverified imported result claims; current checks are in audit |
| `docs/progress_reports/Week1_Progress_Report.md` through `Week4_Progress_Report.md` | Imported future-dated narratives; not actual weekly evidence |
| `docs/credits.md` | Current two-developer list, attribution and future contribution-record rules |
| `AGENTS.md`, `docs/PROJECT_MEMORY.md` | Shared explicit instructions and current approved decisions/baseline |
| `docs/MASTER_PROMPT.md`, `REPOSITORY_AUDIT.md`, `LEARNING_ROADMAP.md` | Original requirements, evidence-backed comparison, proposed small implementation steps |
| `docs/CODE_GUIDE.md` | This current-code reference; update when accepted implementation changes invalidate it |
| `docs/FEATURE_NOTE_TEMPLATE.md`, `PROGRESS_REPORT_TEMPLATE.md` | Reusable per-feature explanation and actual weekly evidence formats |

Asset names after the first full path in a row share that row's folder. JSON coordinates must match solid map tiles with reachable adjacent floor. Editing art changes presentation; editing an answer/requirement can change gameplay and test expectations, so seek the appropriate approval.

## First learning session

Concept: one movement rule changes a player's state. Simple example: `(1,1) + (1,0) = (2,1)`. Project example: step right onto floor, then right into the pedestal. Let John trace it; let his partner predict the second result; swap roles for `inspect()` and a duplicate clue. Run the narrow tests on a suitable environment and explain each assertion.

Only after that, choose one approved function to implement or improve. Both students should be able to answer: who calls it, what goes in, what comes out, what changes, what can fail, and which check proves its behavior. Use the feature template to record the actual attempt and review, not a prefilled claim of mastery.
