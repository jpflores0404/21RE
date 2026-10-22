"""Full-screen views: close-up, journal, messages, text input, and the win screen."""
import curses
import string
import textwrap
from pathlib import Path

from client.ui import (BACK_KEYS, BACKSPACE_KEYS, CYAN, DOWN_KEYS, GREEN, SCREEN_H, SCREEN_W,
                       UP_KEYS, YELLOW, color, draw_box, draw_key_bar, frame, write,
                       write_center)

ART_DIR = Path(__file__).resolve().parent.parent / "assets" / "art"


def load_art(name):
    """Read the ASCII picture assets/art/<name>.txt as a list of lines."""
    path = ART_DIR / f"{name}.txt"
    if not path.exists():
        return [f"[missing art: {name}]"]
    return path.read_text(encoding="utf-8").rstrip("\n").splitlines()


def wait_for_key(stdscr, draw):
    """Call draw(win) until the player presses a key. Returns the key."""
    stdscr.timeout(-1)  # wait for keys instead of checking every few ms
    while True:
        win = frame(stdscr)
        if win:
            draw(win)
            stdscr.refresh()
        key = stdscr.getch()
        if key != curses.KEY_RESIZE:
            return key


def show_message(stdscr, title, lines):
    """A simple box with some text. Any key closes it."""
    def draw(win):
        draw_box(win, 0, 0, SCREEN_H - 1, SCREEN_W, title, color(CYAN))
        y = 4
        for line in lines:
            for part in textwrap.wrap(line, SCREEN_W - 10) or [""]:
                write_center(win, y, part)
                y += 1
        draw_key_bar(win, SCREEN_H - 1, [("any key", "Continue")])

    wait_for_key(stdscr, draw)


def show_inspect(stdscr, result):
    """Close-up of an object: its picture, then its details."""
    art = load_art(result.art)

    def draw(win):
        draw_box(win, 0, 0, SCREEN_H - 1, SCREEN_W, f"CLOSE-UP: {result.title}", color(CYAN))
        art_x = (SCREEN_W - max(len(line) for line in art)) // 2
        y = 2
        for line in art[:13]:
            write(win, y, art_x, line, color(YELLOW))
            y += 1
        y += 1
        for line in result.lines:
            attr = color(GREEN) | curses.A_BOLD if line.startswith("*") else 0
            for part in textwrap.wrap(line, SCREEN_W - 8):
                write_center(win, y, part, attr)
                y += 1
        draw_key_bar(win, SCREEN_H - 1, [("any key", "Back to the room")])

    wait_for_key(stdscr, draw)


def show_journal(stdscr, session, partner_name):
    """List your items and clues. Returns a clue to share, or None.

    W/S picks a clue and Enter sends it to your partner, so they can read it
    in their own journal. This is how the two rooms swap information.
    """
    selected = 0

    def draw(win):
        draw_box(win, 0, 0, SCREEN_H - 1, SCREEN_W, f"JOURNAL - {session.room.name}", color(CYAN))
        write(win, 2, 3, "ITEMS: ", color(CYAN) | curses.A_BOLD)
        write(win, 2, 10, ", ".join(session.items) or "(none yet)")
        write(win, 4, 3, f"CLUES ({len(session.clues)})", color(CYAN) | curses.A_BOLD)
        if not session.clues:
            write(win, 5, 5, "(none yet - press F on things to look closely)", curses.A_DIM)

        # Show the clues starting a little above the selected one,
        # so the selection is always on screen.
        y = 5
        first = max(0, selected - 4)
        for i in range(first, len(session.clues)):
            attr = curses.A_REVERSE if i == selected else 0
            for n, part in enumerate(textwrap.wrap(session.clues[i], SCREEN_W - 10)):
                write(win, y, 5, ("> " if n == 0 else "  ") + part, attr)
                y += 1
            if y > SCREEN_H - 4:
                break
        write(win, SCREEN_H - 3, 3, f"Select a clue and press Enter to send it to {partner_name}.",
              curses.A_DIM)
        draw_key_bar(win, SCREEN_H - 1, [("W/S", "Select"), ("Enter", "Share clue"),
                                         ("Q", "Back")])

    while True:
        key = wait_for_key(stdscr, draw)
        if key in UP_KEYS and selected > 0:
            selected -= 1
        elif key in DOWN_KEYS and selected < len(session.clues) - 1:
            selected += 1
        elif key in (10, 13, curses.KEY_ENTER) and session.clues:
            return session.clues[selected]
        elif key in BACK_KEYS or key in (ord("i"), ord("I")):
            return None


def ask_text(stdscr, title, question, max_length=60, start_text=""):
    """Let the player type a line of text. Returns it, or None if they press Esc."""
    text = start_text
    curses.curs_set(1)  # show the blinking cursor while typing

    def draw(win):
        draw_box(win, 8, 4, 7, SCREEN_W - 8, title, color(CYAN))
        write(win, 10, 7, question)
        write(win, 12, 7, "> " + text + " " * (max_length - len(text)), curses.A_UNDERLINE)
        draw_key_bar(win, SCREEN_H - 1, [("Enter", "OK"), ("Esc", "Cancel")])
        win.move(12, 9 + len(text))

    try:
        while True:
            key = wait_for_key(stdscr, draw)
            if key == 27:
                return None
            if key in (10, 13, curses.KEY_ENTER):
                return text.strip()
            if key in BACKSPACE_KEYS:
                text = text[:-1]
            elif 0 <= key < 256 and chr(key) in string.printable and len(text) < max_length:
                if chr(key) not in "\t\n\r\x0b\x0c":
                    text += chr(key)
    finally:
        curses.curs_set(0)


WIN_ART = [
    r"__   __            _____                            _ _ ",
    r"\ \ / /__  _   _  | ____|___  ___ __ _ _ __   ___  __| | |",
    r" \ V / _ \| | | | |  _| / __|/ __/ _` | '_ \ / _ \/ _` | |",
    r"  | | (_) | |_| | | |___\__ \ (_| (_| | |_) |  __/ (_| |_|",
    r"  |_|\___/ \__,_| |_____|___/\___\__,_| .__/ \___|\__,_(_)",
    r"                                      |_|                 ",
]


def show_win(stdscr, seconds):
    minutes, seconds = divmod(int(seconds), 60)

    def draw(win):
        draw_box(win, 0, 0, SCREEN_H - 1, SCREEN_W, "THE RECORDS WING", color(GREEN))
        art_x = (SCREEN_W - max(len(line) for line in WIN_ART)) // 2
        for i, line in enumerate(WIN_ART):
            write(win, 4 + i, art_x, line, color(GREEN) | curses.A_BOLD)
        write_center(win, 12, "Both players made it out. Two rooms, one exit.", curses.A_BOLD)
        write_center(win, 14, f"Escape time: {minutes} min {seconds:02d} s")
        write_center(win, 16, "Thanks for playing 2R1E!", curses.A_DIM)
        draw_key_bar(win, SCREEN_H - 1, [("any key", "Back to menu")])

    curses.napms(300)
    curses.flushinp()  # ignore keys that were pressed while walking out
    wait_for_key(stdscr, draw)
