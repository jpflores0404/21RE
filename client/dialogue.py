"""RPG-style dialogue box text, split into pages of a few lines each."""
import textwrap

LINES_PER_PAGE = 3
TEXT_WIDTH = 74


class Dialogue:
    def __init__(self, title, lines):
        self.title = title
        wrapped = []
        for line in lines:
            wrapped += textwrap.wrap(line, TEXT_WIDTH) or [""]
        self.pages = [wrapped[i:i + LINES_PER_PAGE] for i in range(0, len(wrapped), LINES_PER_PAGE)]
        if not self.pages:
            self.pages = [[""]]
        self.page = 0

    def lines(self):
        return self.pages[self.page]

    def is_last_page(self):
        return self.page == len(self.pages) - 1

    def next_page(self):
        self.page += 1
