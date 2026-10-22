"""Small curses drawing helpers used by every screen."""
import curses

# Colour pair numbers. Use them with color(), e.g. color(YELLOW).
WHITE, YELLOW, RED, GREEN, MAGENTA, CYAN, BAR = range(1, 8)

# Which colour each tile style is drawn in (see game/tiles.py).
STYLE_COLORS = {"wall": WHITE, "object": YELLOW, "door": RED, "decor": MAGENTA}

# Every in-game screen is drawn on a fixed 80x24 area (the default terminal
# size), centred if the terminal window is bigger.
SCREEN_W, SCREEN_H = 80, 24

# Keys used on many screens.
ENTER_KEYS = (10, 13, curses.KEY_ENTER, ord(" "), ord("e"), ord("E"))
BACK_KEYS = (27, ord("q"), ord("Q"))  # 27 is Esc
UP_KEYS = (curses.KEY_UP, ord("w"), ord("W"))
DOWN_KEYS = (curses.KEY_DOWN, ord("s"), ord("S"))
LEFT_KEYS = (curses.KEY_LEFT, ord("a"), ord("A"))
RIGHT_KEYS = (curses.KEY_RIGHT, ord("d"), ord("D"))
BACKSPACE_KEYS = (curses.KEY_BACKSPACE, 127, 8)


def init_colors():
    if not curses.has_colors():
        return
    curses.start_color()
    try:
        curses.use_default_colors()
        bg = -1  # keep the terminal's own background colour
    except curses.error:
        bg = curses.COLOR_BLACK
    curses.init_pair(WHITE, curses.COLOR_WHITE, bg)
    curses.init_pair(YELLOW, curses.COLOR_YELLOW, bg)
    curses.init_pair(RED, curses.COLOR_RED, bg)
    curses.init_pair(GREEN, curses.COLOR_GREEN, bg)
    curses.init_pair(MAGENTA, curses.COLOR_MAGENTA, bg)
    curses.init_pair(CYAN, curses.COLOR_CYAN, bg)
    curses.init_pair(BAR, curses.COLOR_BLACK, curses.COLOR_CYAN)


def color(pair):
    return curses.color_pair(pair) if curses.has_colors() else 0


def write(win, y, x, text, attr=0):
    """Draw text, cutting off anything that would go past the window edge.

    curses raises an error when you write outside the window (even in the
    bottom-right corner), so every screen draws through this function.
    """
    height, width = win.getmaxyx()
    if y < 0 or y >= height or x >= width:
        return
    if x < 0:
        text, x = text[-x:], 0
    text = text[:width - x]
    if not text:
        return
    try:
        win.addstr(y, x, text, attr)
    except curses.error:
        pass


def write_center(win, y, text, attr=0):
    _, width = win.getmaxyx()
    write(win, y, max(0, (width - len(text)) // 2), text, attr)


def draw_box(win, y, x, h, w, title="", attr=0):
    """Draw a rectangle border with an optional title on the top edge."""
    write(win, y, x, "+" + "-" * (w - 2) + "+", attr)
    for row in range(y + 1, y + h - 1):
        write(win, row, x, "|", attr)
        write(win, row, x + w - 1, "|", attr)
    write(win, y + h - 1, x, "+" + "-" * (w - 2) + "+", attr)
    if title:
        write(win, y, x + 2, f" {title} ", attr | curses.A_BOLD)


def draw_key_bar(win, y, keys):
    """Draw a bar of (key, meaning) pairs, e.g. [("E", "Use"), ("Q", "Menu")]."""
    _, width = win.getmaxyx()
    write(win, y, 0, " " * width, color(BAR))
    x = 1
    for key, label in keys:
        write(win, y, x, f" {key} ", color(BAR) | curses.A_REVERSE | curses.A_BOLD)
        x += len(key) + 2
        write(win, y, x, f" {label}", color(BAR))
        x += len(label) + 2


def progress_bar(done, total, width):
    """Text bar like [#####.....] for done out of total."""
    filled = 0 if total == 0 else round(width * done / total)
    return "[" + "#" * filled + "." * (width - filled) + "]"


def frame(stdscr):
    """Clear the screen and return a centred 80x24 area to draw in.

    Returns None (after showing a message) if the terminal is too small.
    """
    height, width = stdscr.getmaxyx()
    stdscr.erase()
    if width < SCREEN_W or height < SCREEN_H:
        write(stdscr, 0, 0, "Terminal too small.")
        write(stdscr, 1, 0, f"Need {SCREEN_W}x{SCREEN_H}, have {width}x{height}.")
        write(stdscr, 2, 0, "Make the window bigger, or press Q.")
        stdscr.refresh()
        return None
    return stdscr.derwin(SCREEN_H, SCREEN_W, (height - SCREEN_H) // 2, (width - SCREEN_W) // 2)
