"""Among Us style task mini-games.

Each mini-game is one function that takes the task (the dictionary from the
objects JSON file) and returns True when the player finishes it, or False if
they leave early with Q / Esc. Leaving early is fine: they can try again.

    wires     Fix Wiring      join each coloured wire to the same colour
    swipe     Swipe Card      press SPACE while the card is in the green zone
    download  Download Data   start the transfer and wait for the bar to fill
    upload    Upload Data     same as download (needs the partner's download first)
    code      Enter Code      type the right code on a keypad
    breaker   Reset Breakers  press the switches 1 to 10 in order
"""
import curses
import random
import time

from client.ui import (BACK_KEYS, BACKSPACE_KEYS, CYAN, DOWN_KEYS, ENTER_KEYS, GREEN, LEFT_KEYS,
                       MAGENTA, RED, RIGHT_KEYS, SCREEN_H, SCREEN_W, UP_KEYS, YELLOW, color,
                       draw_box, draw_key_bar, frame, progress_bar, write, write_center)


def run_task(stdscr, task):
    """Play the mini-game for this task. Returns True if it was completed."""
    games = {
        "wires": play_wires,
        "swipe": play_swipe,
        "download": play_transfer,
        "upload": play_transfer,
        "code": play_code,
        "breaker": play_breaker,
    }
    stdscr.timeout(-1)
    finished = games[task["task"]](stdscr, task)
    stdscr.timeout(-1)
    if finished:
        show_task_complete(stdscr, task)
    return finished


def task_window(stdscr, task, keys):
    """Clear the screen and draw the task's border and key bar. Returns the window."""
    win = frame(stdscr)
    if win:
        draw_box(win, 0, 0, SCREEN_H - 1, SCREEN_W, "TASK: " + task["title"], color(CYAN))
        draw_key_bar(win, SCREEN_H - 1, keys + [("Q", "Leave")])
    return win


def show_task_complete(stdscr, task):
    win = frame(stdscr)
    if win:
        draw_box(win, 7, 20, 7, 40, "", color(GREEN))
        write_center(win, 9, "TASK COMPLETE!", color(GREEN) | curses.A_BOLD)
        write_center(win, 11, task["title"])
        stdscr.refresh()
    curses.napms(1000)
    curses.flushinp()  # ignore keys pressed during the pause


# ---- Fix Wiring -------------------------------------------------------------

WIRES = [("RED", RED), ("YELLOW", YELLOW), ("GREEN", GREEN), ("PINK", MAGENTA)]


def play_wires(stdscr, task):
    left = [0, 1, 2, 3]    # wire colours (index into WIRES) from top to bottom
    right = [0, 1, 2, 3]
    random.shuffle(left)
    random.shuffle(right)
    fixed = set()          # colours that are already connected
    side = "left"          # which side the cursor is on
    cursor = 0
    picked = None          # colour picked on the left side
    message = "Pick a wire on the left."

    while len(fixed) < len(WIRES):
        win = task_window(stdscr, task, [("W/S", "Move"), ("Enter", "Pick")])
        if win:
            draw_wires(win, left, right, fixed, side, cursor, picked, message)
            stdscr.refresh()

        key = stdscr.getch()
        if key in BACK_KEYS:
            return False
        if key in UP_KEYS:
            cursor = (cursor - 1) % len(WIRES)
        elif key in DOWN_KEYS:
            cursor = (cursor + 1) % len(WIRES)
        elif key in ENTER_KEYS and side == "left":
            if left[cursor] in fixed:
                message = "That wire is already connected."
            else:
                picked = left[cursor]
                side = "right"
                message = f"Now pick the {WIRES[picked][0]} end on the right."
        elif key in ENTER_KEYS and side == "right":
            if right[cursor] == picked:
                fixed.add(picked)
                message = "Connected!"
            else:
                message = "Wrong colour! Pick a wire on the left again."
            side = "left"
            picked = None
    return True


def draw_wires(win, left, right, fixed, side, cursor, picked, message):
    write_center(win, 2, "Connect each wire to the same colour on the other side.")
    rows = [6, 9, 12, 15]
    for i in range(len(WIRES)):
        name, pair = WIRES[left[i]]
        attr = color(pair) | curses.A_BOLD
        if left[i] == picked:
            attr |= curses.A_REVERSE
        write(win, rows[i], 6, f"{name:>6} ######", attr)

        name, pair = WIRES[right[i]]
        write(win, rows[i], 60, f"###### {name}", color(pair) | curses.A_BOLD)

    # The cursor arrow.
    if side == "left":
        write(win, rows[cursor], 3, ">>", curses.A_BOLD)
    else:
        write(win, rows[cursor], 75, "<<", curses.A_BOLD)

    # Draw each fixed wire as an L-shaped line from its left end to its right end.
    for colour in fixed:
        attr = color(WIRES[colour][1]) | curses.A_BOLD
        y1 = rows[left.index(colour)]
        y2 = rows[right.index(colour)]
        middle = 26 + colour * 8  # each wire turns at a different column
        write(win, y1, 19, "-" * (middle - 19), attr)
        for y in range(min(y1, y2), max(y1, y2) + 1):
            write(win, y, middle, "|", attr)
        write(win, y2, middle, "-" * (60 - middle), attr)

    write_center(win, 19, message, curses.A_BOLD)


# ---- Swipe Card -------------------------------------------------------------

TRACK_X, TRACK_W = 20, 40
ZONE_START, ZONE_END = 29, 34   # green zone, as positions along the track
SWIPE_SPEED_MS = 40             # lower = faster card


def play_swipe(stdscr, task):
    position = 0
    message = "Press SPACE while the card is inside the green zone."
    stdscr.timeout(SWIPE_SPEED_MS)  # getch() returns -1 if no key, so the card keeps moving

    while True:
        win = task_window(stdscr, task, [("Space", "Swipe")])
        if win:
            draw_swipe(win, position, message)
            stdscr.refresh()

        key = stdscr.getch()
        if key in BACK_KEYS:
            return False
        if key in ENTER_KEYS:
            if ZONE_START <= position <= ZONE_END:
                return True
            message = "Too early. Try again." if position < ZONE_START else "Too late. Try again."

        position = (position + 1) % TRACK_W


def draw_swipe(win, position, message):
    write_center(win, 3, ".-------------------------.")
    write_center(win, 4, "|  STAFF KEYCARD   [====] |")
    write_center(win, 5, "'-------------------------'")
    write_center(win, 8, "CARD READER", curses.A_BOLD)

    write(win, 11, TRACK_X - 1, "|")
    write(win, 11, TRACK_X + TRACK_W, "|")
    for i in range(TRACK_W):
        in_zone = ZONE_START <= i <= ZONE_END
        write(win, 11, TRACK_X + i, "=" if in_zone else ".",
              color(GREEN) | curses.A_BOLD if in_zone else curses.A_DIM)
    write(win, 10, TRACK_X + position, "v", color(YELLOW) | curses.A_BOLD)
    write(win, 11, TRACK_X + position, "#", color(YELLOW) | curses.A_BOLD | curses.A_REVERSE)

    write_center(win, 15, message, curses.A_BOLD)


# ---- Download / Upload Data ---------------------------------------------------

def play_transfer(stdscr, task):
    seconds = task.get("seconds", 6)
    uploading = task["task"] == "upload"
    started = None  # time when the player pressed E
    stdscr.timeout(100)

    while True:
        progress = 0.0
        if started is not None:
            progress = min(1.0, (time.time() - started) / seconds)

        win = task_window(stdscr, task, [("E", "Start")])
        if win:
            draw_transfer(win, uploading, progress, started is not None, seconds)
            stdscr.refresh()
        if progress >= 1.0:
            curses.napms(500)
            return True

        key = stdscr.getch()
        if key in BACK_KEYS:
            return False  # leaving stops the transfer, like in Among Us
        if key in ENTER_KEYS and started is None:
            started = time.time()


def draw_transfer(win, uploading, progress, running, seconds):
    if uploading:
        source, target, verb = "GALLERY", "EXIT CONTROL", "Uploading"
    else:
        source, target, verb = "ARCHIVE", "THIS TERMINAL", "Downloading"

    write(win, 5, 10, ".-------.", color(YELLOW))
    write(win, 6, 10, "| [===] |", color(YELLOW))
    write(win, 7, 10, "'-------'", color(YELLOW))
    write(win, 8, 9, f"{source:^11}")
    write(win, 5, 58, ".-------.", color(CYAN))
    write(win, 6, 58, "| [===] |", color(CYAN))
    write(win, 7, 58, "'-------'", color(CYAN))
    write(win, 8, 56, f"{target:^14}")

    # A little file flies from one folder to the other.
    write(win, 6, 22, "- " * 17, curses.A_DIM)
    if running and progress < 1.0:
        write(win, 6, 22 + int(progress * 32), "[#]", color(GREEN) | curses.A_BOLD)

    write(win, 12, 18, progress_bar(progress, 1.0, 40), color(GREEN) | curses.A_BOLD)
    if not running:
        write_center(win, 14, "Press E to start.", curses.A_BOLD)
    elif progress < 1.0:
        left = max(0, round(seconds * (1 - progress)))
        write_center(win, 14, f"{verb}... {int(progress * 100)}%   Estimated time: {left}s")
        write_center(win, 16, "Don't leave until it finishes!", curses.A_DIM)
    else:
        write_center(win, 14, "Complete!", color(GREEN) | curses.A_BOLD)


# ---- Enter Code -------------------------------------------------------------

KEYPAD = [
    "[1]  [2]  [3]",
    "[4]  [5]  [6]",
    "[7]  [8]  [9]",
    "     [0]     ",
]


def play_code(stdscr, task):
    answer = task["answer"]
    typed = ""
    message = task.get("hint", "Enter the code.")

    while True:
        win = task_window(stdscr, task, [("0-9", "Type"), ("Backspace", "Delete"),
                                         ("Enter", "Submit")])
        if win:
            draw_code(win, answer, typed, message)
            stdscr.refresh()

        key = stdscr.getch()
        if key in BACK_KEYS:
            return False
        if ord("0") <= key <= ord("9") and len(typed) < len(answer):
            typed += chr(key)
        elif key in BACKSPACE_KEYS:
            typed = typed[:-1]
        elif key in (10, 13, curses.KEY_ENTER):
            if typed == answer:
                return True
            message = "ACCESS DENIED. Try again."
            typed = ""


def draw_code(win, answer, typed, message):
    shown = " ".join(typed.ljust(len(answer), "_"))
    draw_box(win, 3, 28, 3, 24, "", color(CYAN))
    write_center(win, 4, shown, color(GREEN) | curses.A_BOLD)
    for i, row in enumerate(KEYPAD):
        write_center(win, 8 + i * 2, row, color(YELLOW))
    attr = color(RED) | curses.A_BOLD if message.startswith("ACCESS") else curses.A_DIM
    write_center(win, 17, message, attr)


# ---- Reset Breakers ---------------------------------------------------------

def play_breaker(stdscr, task):
    numbers = list(range(1, 11))
    random.shuffle(numbers)  # 10 switches in a 2 x 5 grid
    next_number = 1
    cursor = 0
    message = "Press the switches in order: 1, 2, 3 ... 10."

    while next_number <= 10:
        win = task_window(stdscr, task, [("WASD", "Move"), ("Enter", "Press")])
        if win:
            draw_breaker(win, numbers, next_number, cursor, message)
            stdscr.refresh()

        key = stdscr.getch()
        if key in BACK_KEYS:
            return False
        if key in LEFT_KEYS and cursor % 5 > 0:
            cursor -= 1
        elif key in RIGHT_KEYS and cursor % 5 < 4:
            cursor += 1
        elif key in UP_KEYS and cursor >= 5:
            cursor -= 5
        elif key in DOWN_KEYS and cursor < 5:
            cursor += 5
        elif key in ENTER_KEYS:
            if numbers[cursor] == next_number:
                next_number += 1
                message = "Click!"
            elif numbers[cursor] < next_number:
                message = "That switch is already on."
            else:
                next_number = 1
                message = "Wrong order! All switches reset. Start again from 1."
    curses.napms(300)
    return True


def draw_breaker(win, numbers, next_number, cursor, message):
    write_center(win, 3, f"Next switch: {next_number}", curses.A_BOLD)
    for i, number in enumerate(numbers):
        x = 17 + (i % 5) * 10
        y = 7 + (i // 5) * 5
        is_on = number < next_number
        attr = color(GREEN) | curses.A_BOLD if is_on else color(YELLOW)
        if i == cursor:
            attr |= curses.A_REVERSE
        write(win, y, x, ".-----.", attr)
        write(win, y + 1, x, f"| {number:>2}  |", attr)
        write(win, y + 2, x, "'-----'", attr)
    write_center(win, 18, message, curses.A_BOLD)
