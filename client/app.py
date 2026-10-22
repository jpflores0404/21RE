"""The top-level flow: menu -> game -> menu."""
import curses

from client.game_screen import run_game
from client.lobby import host_game, join_game
from client.menu import main_menu, show_help
from client.network import LocalLink
from client.ui import init_colors
from game.level import load_level

START_LEVEL = "records_wing"


def run(stdscr):
    curses.curs_set(0)    # hide the text cursor
    stdscr.keypad(True)   # turn arrow keys into curses.KEY_UP etc.
    init_colors()

    while True:
        choice = main_menu(stdscr)
        link = None
        if choice == "local":
            link, player_id = LocalLink(), 1
        elif choice == "host":
            link, player_id = host_game(stdscr), 1
        elif choice == "join":
            link, player_id = join_game(stdscr), 2
        elif choice == "help":
            show_help(stdscr)
        elif choice == "quit":
            return

        if link:
            # Load a fresh copy of the level for every new game.
            local = isinstance(link, LocalLink)
            try:
                run_game(stdscr, load_level(START_LEVEL), player_id, link, local)
            finally:
                link.close()
