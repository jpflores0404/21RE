"""The in-game screen: map, task list, dialogue box, and the main game loop.

Screen layout (80 x 24):
    row 0        status bar with the TOTAL TASKS bar
    rows 1-15    map (left, 50 wide)  |  player panel and task list (right)
    rows 16-21   dialogue box
    row 22       hint line ("! Wiring Panel  [E] Use  [F] Inspect")
    row 23       controls bar
"""
import curses
import time

from client.dialogue import Dialogue
from client.menu import show_help
from client.screens import ask_text, show_inspect, show_journal, show_message, show_win
from client.sprites import get_sprite
from client.tasks import run_task
from client.ui import (BACK_KEYS, BAR, CYAN, DOWN_KEYS, ENTER_KEYS, GREEN, LEFT_KEYS, RIGHT_KEYS,
                       SCREEN_W, STYLE_COLORS, UP_KEYS, YELLOW, color, draw_box, draw_key_bar,
                       frame, progress_bar, write)
from game.session import RoomSession
from game.team import Team

MAP_X, MAP_Y, MAP_W, MAP_H = 1, 1, 50, 15
PANEL_X, PANEL_Y, PANEL_W, PANEL_H = 52, 1, 28, 15
DIALOG_Y, DIALOG_H = 16, 6
HINT_Y, KEYS_Y = 22, 23

PLAYER_COLORS = {1: CYAN, 2: YELLOW}


def key_to_direction(key):
    if key in UP_KEYS:
        return "up"
    if key in DOWN_KEYS:
        return "down"
    if key in LEFT_KEYS:
        return "left"
    if key in RIGHT_KEYS:
        return "right"
    return None


def is_key(key, letter):
    return key in (ord(letter.lower()), ord(letter.upper()))


class Game:
    """Everything that changes while one game is being played."""

    def __init__(self, level, player_id, link, local):
        self.level = level
        self.link = link
        self.local = local            # True: both players share this computer
        self.player_id = player_id    # whose room we are looking at right now
        self.team = Team(level.task_ids())
        self.start_time = time.time()

        # On one computer we keep both rooms. In a LAN game, only our own room
        # is here; the partner's room lives on the partner's computer.
        player_ids = [1, 2] if local else [player_id]
        self.sessions = {pid: RoomSession(level.rooms[pid]) for pid in player_ids}

        room = level.rooms[player_id]
        self.dialogue = Dialogue(room.name, [room.intro, "Objective: " + room.objective])
        self.waiting = []   # dialogues to show after the current one closes
        self.notice = ""    # short message on the hint line

    def session(self):
        return self.sessions[self.player_id]

    def partner_id(self):
        return 2 if self.player_id == 1 else 1

    def has_escaped(self):
        return self.player_id in self.team.escaped


def run_game(stdscr, level, player_id, link, local):
    """Play one game until the team wins, the partner leaves, or the player quits."""
    game = Game(level, player_id, link, local)

    while True:
        if not handle_messages(stdscr, game):
            return  # the partner left
        if game.team.has_won():
            show_win(stdscr, time.time() - game.start_time)
            return
        if game.dialogue is None and game.waiting:
            game.dialogue = game.waiting.pop(0)

        draw(stdscr, game)
        stdscr.timeout(100)  # wait at most 0.1 s for a key, then check the network again
        key = stdscr.getch()
        if key == -1 or key == curses.KEY_RESIZE:
            continue

        if game.dialogue:
            # While a dialogue is open, keys only turn the page or close it.
            if key in BACK_KEYS:
                game.dialogue = None
            elif key in ENTER_KEYS:
                if game.dialogue.is_last_page():
                    game.dialogue = None
                else:
                    game.dialogue.next_page()
            continue

        if key in BACK_KEYS:
            return
        handle_key(stdscr, game, key)


def handle_messages(stdscr, game):
    """Apply messages from the partner. Returns False if the partner has left."""
    for message in game.link.receive():
        kind = message["type"]
        if kind == "bye":
            show_message(stdscr, "PARTNER LEFT", ["Your partner left the game.",
                                                  "Returning to the menu."])
            return False

        game.team.apply(message)  # tasks and escapes

        if kind == "task" and not game.local:
            game.notice = f"Your partner finished: {game.level.task_title(message['id'])}"
        elif kind == "escaped" and not game.local:
            game.notice = "Your partner went through the exit! Join them!"
        elif kind == "clue" and message["to"] in game.sessions:
            game.sessions[message["to"]].add_clue("From partner: " + message["text"])
            if message["to"] == game.player_id:
                game.waiting.append(Dialogue("Your partner shared a clue", [message["text"]]))
        elif kind == "chat":
            game.waiting.append(Dialogue(f"Player {message['from']} says", [message["text"]]))
    return True


def handle_key(stdscr, game, key):
    session = game.session()
    game.notice = ""

    if is_key(key, "h"):
        show_help(stdscr)
    elif key == 9 and game.local:  # Tab
        switch_player(game)
    elif game.has_escaped():
        game.notice = "You escaped! Waiting for your partner..."
    elif key_to_direction(key):
        moved, message = session.move(key_to_direction(key))
        if not moved and not session.faced_name():
            game.notice = message
    elif is_key(key, "e"):
        use(stdscr, game, session.interact(game.team))
    elif is_key(key, "f"):
        result = session.inspect(game.team)
        if result is None:
            game.notice = "There's nothing in front of you to inspect."
        elif result.art:
            show_inspect(stdscr, result)
        else:
            game.dialogue = Dialogue(result.title, result.lines)
    elif is_key(key, "i"):
        clue = show_journal(stdscr, session, f"Player {game.partner_id()}")
        if clue:
            game.link.send({"type": "clue", "to": game.partner_id(), "text": clue})
            game.notice = "Clue sent to your partner."
    elif is_key(key, "t"):
        chat(stdscr, game)


def use(stdscr, game, result):
    """Show what pressing E did: text, a task mini-game, or walking out the exit."""
    if result is None:
        game.notice = "There's nothing in front of you to use."
    elif result.task:
        task = result.task
        if run_task(stdscr, task):
            game.team.finish(task["id"])
            game.link.send({"type": "task", "id": task["id"]})
            game.notice = f"Task complete: {task['title']}"
    elif result.escape:
        game.team.escape(game.player_id)
        game.link.send({"type": "escaped", "player": game.player_id})
        if game.local and not game.team.has_won():
            game.dialogue = Dialogue("Exit", [f"Player {game.player_id} escaped!",
                                              f"Now Player {game.partner_id()} must reach the exit."])
            game.player_id = game.partner_id()
        else:
            game.dialogue = Dialogue("Exit", ["You step through the exit...",
                                              "Waiting for your partner to escape too."])
    else:
        game.dialogue = Dialogue(result.title, result.lines)


def switch_player(game):
    """Tab on one computer: hand the keyboard to the other player."""
    other = game.partner_id()
    if other in game.team.escaped:
        game.notice = f"Player {other} has already escaped."
        return
    game.player_id = other
    room = game.level.rooms[other]
    game.dialogue = Dialogue("Switch player", [f"Now playing as Player {other} in the {room.name}."])


def chat(stdscr, game):
    if game.local:
        game.notice = "Chat is for LAN games. On one computer, just talk to each other!"
        return
    text = ask_text(stdscr, "CHAT", "Message to your partner:")
    if text:
        game.link.send({"type": "chat", "from": game.player_id, "text": text})
        game.notice = "Message sent."


# ---- Drawing ----------------------------------------------------------------

def draw(stdscr, game):
    win = frame(stdscr)
    if win is None:
        return
    draw_status(win, game)
    draw_map(win, game)
    draw_panel(win, game)
    draw_dialogue(win, game)
    draw_hint(win, game)

    keys = [("WASD", "Move"), ("E", "Use"), ("F", "Inspect"), ("I", "Journal")]
    keys += [("Tab", "Switch")] if game.local else [("T", "Chat")]
    keys += [("H", "Help"), ("Q", "Menu")]
    draw_key_bar(win, KEYS_Y, keys)
    stdscr.refresh()


def draw_status(win, game):
    team = game.team
    total = len(team.task_ids)
    mode = "LOCAL" if game.local else "LAN"
    left = f" TOTAL TASKS {progress_bar(team.done_count(), total, 20)} {team.done_count()}/{total}"
    right = f"{mode}  Player {game.player_id} "
    write(win, 0, 0, left.ljust(SCREEN_W - len(right)) + right, color(BAR) | curses.A_BOLD)


def draw_map(win, game):
    session = game.session()
    room = session.room
    game_map = room.game_map
    left = MAP_X + (MAP_W - game_map.width) // 2
    top = MAP_Y + (MAP_H - game_map.height) // 2
    highlight = session.faced_position() if session.faced_name() else None

    for y in range(game_map.height):
        for x in range(game_map.width):
            tile = game_map.tile(x, y)
            char = tile.get("show", game_map.rows[y][x])
            attr = color(STYLE_COLORS.get(tile["style"], 0))
            if tile["style"] == "floor":
                attr |= curses.A_DIM

            # Task stations glow yellow until done, then turn green (like Among Us).
            obj = room.at.get((x, y))
            if obj and obj["kind"] == "task":
                done = game.team.is_done(obj["id"])
                attr = color(GREEN) if done else color(YELLOW) | curses.A_BOLD
            elif obj and obj["kind"] == "exit" and game.team.all_done():
                attr = color(GREEN) | curses.A_BOLD

            if (x, y) == highlight:
                attr |= curses.A_REVERSE
            write(win, top + y, left + x, char, attr)

    if not game.has_escaped():
        player = session.player
        write(win, top + player.y, left + player.x, "@",
              color(PLAYER_COLORS[game.player_id]) | curses.A_BOLD)


def draw_panel(win, game):
    session = game.session()
    player = session.player
    x = PANEL_X
    pcolor = color(PLAYER_COLORS[game.player_id])
    draw_box(win, PANEL_Y, x, PANEL_H, PANEL_W, f"PLAYER {game.player_id}", pcolor)

    for i, line in enumerate(get_sprite(player.facing, player.steps)):
        write(win, PANEL_Y + 1 + i, x + (PANEL_W - len(line)) // 2, line, pcolor | curses.A_BOLD)
    name = session.room.name
    write(win, 7, x + (PANEL_W - len(name)) // 2, name, curses.A_BOLD)

    write(win, 9, x + 2, "YOUR TASKS", color(CYAN) | curses.A_BOLD)
    for i, task in enumerate(session.room.tasks[:4]):
        if game.team.is_done(task["id"]):
            write(win, 10 + i, x + 2, "[x] " + task["title"], color(GREEN))
        else:
            waiting = "needs_task" in task and not game.team.is_done(task["needs_task"])
            write(win, 10 + i, x + 2, "[ ] " + task["title"],
                  curses.A_DIM if waiting else color(YELLOW))
    write(win, 14, x + 2, f"Items {len(session.items)}   Clues {len(session.clues)}", curses.A_DIM)


def draw_dialogue(win, game):
    dialogue = game.dialogue
    title = dialogue.title if dialogue else game.session().room.name
    draw_box(win, DIALOG_Y, 0, DIALOG_H, SCREEN_W, title, color(CYAN))

    if dialogue is None:
        if game.has_escaped():
            lines = ["You escaped! Waiting for your partner to reach the exit..."]
        else:
            lines = ["Objective: " + game.session().room.objective,
                     "Walk into things to face them. E = use, F = look closely, I = journal."]
        for i, line in enumerate(lines):
            write(win, DIALOG_Y + 1 + i, 3, line[:SCREEN_W - 6], curses.A_DIM)
        return

    for i, line in enumerate(dialogue.lines()):
        attr = color(GREEN) | curses.A_BOLD if line.startswith("*") else 0
        write(win, DIALOG_Y + 1 + i, 3, line, attr)
    prompt = "[E] Close" if dialogue.is_last_page() else "[E] Next >"
    write(win, DIALOG_Y + DIALOG_H - 2, SCREEN_W - len(prompt) - 3, prompt,
          color(YELLOW) | curses.A_BOLD)


def draw_hint(win, game):
    session = game.session()
    name = session.faced_name()
    if game.notice:
        write(win, HINT_Y, 2, game.notice, color(GREEN) | curses.A_BOLD)
    elif name and not game.has_escaped():
        label = f" ! {name} "
        write(win, HINT_Y, 1, label, color(YELLOW) | curses.A_BOLD | curses.A_REVERSE)
        write(win, HINT_Y, 2 + len(label), "[E] Use   [F] Inspect", color(YELLOW))
