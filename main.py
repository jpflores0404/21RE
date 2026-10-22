"""2R1E: Two Rooms, One Exit - single-player movement prototype.

Run with:  python3 main.py
"""
import curses
import os


def main():
    # curses waits ~1s after Esc to see if it starts an escape sequence.
    # This must be set before curses initializes to make Esc feel instant.
    os.environ.setdefault("ESCDELAY", "25")

    from client.app import run

    try:
        # wrapper restores the terminal even if the game crashes.
        curses.wrapper(run)
    except KeyboardInterrupt:
        pass
    print("Thanks for playing 2R1E.")


if __name__ == "__main__":
    main()
