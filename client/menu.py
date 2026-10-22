"""Main menu and the How to Play screen."""
import curses

from client.sprites import get_sprite
from client.ui import (BACK_KEYS, BAR, CYAN, DOWN_KEYS, ENTER_KEYS, SCREEN_H, SCREEN_W, UP_KEYS,
                       YELLOW, color, draw_box, draw_key_bar, frame, write, write_center)

TITLE_ART = [
    r" ____  ____  _ _____ ",
    r"|___ \|  _ \/ | ____|",
    r"  __) | |_) | |  _|  ",
    r" / __/|  _ <| | |___ ",
    r"|_____|_| \_\_|_____|",
]

MENU_ITEMS = [
    ("Play on this computer (Tab switches player)", "local"),
    ("Host LAN game  (you are Player 1 - Gallery)", "host"),
    ("Join LAN game  (you are Player 2 - Archive)", "join"),
    ("How to play", "help"),
    ("Quit", "quit"),
]

HOW_TO_PLAY = [
    ("GOAL", "Two players wake up in two different rooms. Finish every task in"),
    ("", "BOTH rooms to fill the TOTAL TASKS bar. Then the exit opens and"),
    ("", "both players must walk out through it to win."),
    ("TASKS", "Your task list is on the right. Task stations glow yellow on the"),
    ("", "map. Some tasks need a clue or a task from your partner's room!"),
    ("CLUES", "Press F on things to look closely. Clues go into your journal (I)."),
    ("", "In the journal, press Enter on a clue to send it to your partner."),
    ("", ""),
    ("W A S D", "Move. Walk into something to face it."),
    ("E", "Use what you are facing (read, open, start a task)."),
    ("F", "Look closely at what you are facing."),
    ("I", "Journal: your items and clues. Share clues from here."),
    ("T", "Chat with your partner (LAN games)."),
    ("Tab", "Switch player (games on one computer)."),
    ("Q / Esc", "Close a box, leave a task, or go back to the menu."),
]


def main_menu(stdscr):
    """Show the menu and return the id of the chosen item, e.g. "local"."""
    selected = 0
    stdscr.timeout(-1)
    while True:
        win = frame(stdscr)
        if win:
            draw_menu(win, selected)
            stdscr.refresh()

        key = stdscr.getch()
        if key in UP_KEYS:
            selected = (selected - 1) % len(MENU_ITEMS)
        elif key in DOWN_KEYS:
            selected = (selected + 1) % len(MENU_ITEMS)
        elif key in ENTER_KEYS:
            return MENU_ITEMS[selected][1]
        elif key in BACK_KEYS:
            return "quit"


def draw_menu(win, selected):
    for i, line in enumerate(TITLE_ART):
        write_center(win, 2 + i, line, color(CYAN) | curses.A_BOLD)
    write_center(win, 8, "TWO ROOMS, ONE EXIT", curses.A_BOLD)
    write_center(win, 9, "a two-player escape room", curses.A_DIM)

    for i, (label, _) in enumerate(MENU_ITEMS):
        if i == selected:
            write_center(win, 12 + i, f"> {label} <", color(BAR) | curses.A_BOLD)
        else:
            write_center(win, 12 + i, label)

    # The two player characters on each side of the menu.
    for i, line in enumerate(get_sprite("right", 0)):
        write(win, 12 + i, 2, line, color(CYAN) | curses.A_BOLD)
    for i, line in enumerate(get_sprite("left", 1)):
        write(win, 12 + i, SCREEN_W - 9, line, color(YELLOW) | curses.A_BOLD)
    write(win, 18, 3, " P1 ", color(CYAN))
    write(win, 18, SCREEN_W - 8, " P2 ", color(YELLOW))

    draw_key_bar(win, SCREEN_H - 1, [("W/S", "Choose"), ("Enter", "Select"), ("Q", "Quit")])


def show_help(stdscr):
    """Show How to Play until a key is pressed."""
    stdscr.timeout(-1)
    while True:
        win = frame(stdscr)
        if win:
            draw_box(win, 0, 0, SCREEN_H - 1, SCREEN_W, "HOW TO PLAY", color(CYAN))
            for i, (label, text) in enumerate(HOW_TO_PLAY):
                write(win, 2 + i, 3, f"{label:>8}", color(YELLOW) | curses.A_BOLD)
                write(win, 2 + i, 13, text)
            draw_key_bar(win, SCREEN_H - 1, [("any key", "Back")])
            stdscr.refresh()
        if stdscr.getch() != curses.KEY_RESIZE:
            return
