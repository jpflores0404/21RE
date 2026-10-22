# UI Research — curses

**Task 1.5 · Owner: M4 · Due: Oct 26**

- `curses` ships with Python on macOS and Linux. On Windows it would need
  `windows-curses`, so we target macOS/Linux (the school lab machines).
- The default terminal is 80 x 24, so all screens are designed for exactly
  80 x 24 and centred in bigger windows. If the window is smaller, we show a
  "make the window bigger" message instead of crashing.
- Writing into the bottom-right cell raises an error, so every screen draws
  through one safe `write()` helper.
- Esc normally waits 1 second; setting `ESCDELAY=25` makes it instant.
- `curses.wrapper()` restores the terminal even if the game crashes.
- Colours: 7 colour pairs (white, yellow, red, green, magenta, cyan, bar).
  We keep the user's background colour so the game works on light and dark
  themes.
- Animation (swipe card, progress bars) uses `stdscr.timeout(ms)`, so
  `getch()` returns -1 when no key is pressed.
