"""Screens for starting a LAN game: Host (wait for a partner) and Join (type the host's IP)."""
import curses

from client.network import PORT, check_for_partner, join, my_ip, start_hosting
from client.screens import ask_text, show_message
from client.ui import (BACK_KEYS, CYAN, GREEN, SCREEN_H, SCREEN_W, color, draw_box, draw_key_bar,
                       frame, write_center)


def host_game(stdscr):
    """Wait for the other player to join. Returns a NetLink, or None if cancelled."""
    try:
        server = start_hosting()
    except OSError as error:
        show_message(stdscr, "HOST GAME", [f"Could not open port {PORT}.", str(error),
                                           "Is another copy of the game already hosting?"])
        return None

    ip = my_ip()
    dots = 0
    stdscr.timeout(0)  # don't wait for keys; check_for_partner() already waits a little
    try:
        while True:
            win = frame(stdscr)
            if win:
                draw_box(win, 0, 0, SCREEN_H - 1, SCREEN_W, "HOST LAN GAME", color(CYAN))
                write_center(win, 5, "You are Player 1 (Gallery Room).")
                write_center(win, 8, "Tell your partner to choose 'Join LAN game' and type:")
                write_center(win, 10, f"  {ip}  ", color(GREEN) | curses.A_BOLD | curses.A_REVERSE)
                write_center(win, 13, "Waiting for Player 2" + "." * (dots % 4) + "   ")
                write_center(win, 16, "Both computers must be on the same Wi-Fi / network.",
                             curses.A_DIM)
                write_center(win, 17, "Testing alone? Open a second terminal and join 127.0.0.1",
                             curses.A_DIM)
                draw_key_bar(win, SCREEN_H - 1, [("Q", "Cancel")])
                stdscr.refresh()
            dots += 1

            link = check_for_partner(server)
            if link:
                return link
            if stdscr.getch() in BACK_KEYS:
                return None
    finally:
        server.close()
        stdscr.timeout(-1)


def join_game(stdscr):
    """Ask for the host's IP and connect. Returns a NetLink, or None."""
    ip = ask_text(stdscr, "JOIN LAN GAME", "Host's IP address (empty = this computer):", 40)
    if ip is None:
        return None
    ip = ip or "127.0.0.1"

    win = frame(stdscr)
    if win:
        write_center(win, 11, f"Connecting to {ip}...")
        stdscr.refresh()
    try:
        return join(ip)
    except OSError as error:
        show_message(stdscr, "JOIN LAN GAME", [f"Could not connect to {ip}.", str(error),
                                               "Check the IP address and that the host is waiting."])
        return None
