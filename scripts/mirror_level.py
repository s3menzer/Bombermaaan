#!/usr/bin/env python3
"""Mirror a 19x13 Bombermaaan level from the bottom-left quadrant.

Usage:
    scripts/mirror <LevelFile>

The bottom-left quadrant (rows 7..11, columns 1..8) is the source. Its
non-player cells are reflected to the upper-left, bottom-right, and upper-
right quadrants. Player markers (1..8) are never overwritten.
"""

from __future__ import annotations

import sys
from pathlib import Path

WIDTH = 19
HEIGHT = 13
PLAYER_MARKERS = set("12345678")


def usage() -> None:
    print(f"Usage: {Path(sys.argv[0]).as_posix()} <LevelFile>", file=sys.stderr)


def main() -> int:
    if len(sys.argv) != 2:
        usage()
        return 2

    path = Path(sys.argv[1])
    if not path.is_file():
        print(f"error: level file not found: {path}", file=sys.stderr)
        return 1

    text = path.read_text()
    trailing_newline = text.endswith("\n")
    rows = text.splitlines()

    if len(rows) != HEIGHT or any(len(row) != WIDTH for row in rows):
        print(
            f"error: expected a {WIDTH}x{HEIGHT} level: {path}",
            file=sys.stderr,
        )
        return 1

    grid = [list(row) for row in rows]

    # Use the bottom-left quadrant as the canonical source. The center row
    # and center column are intentionally excluded.
    for y in range(7, 12):
        for x in range(1, 9):
            value = grid[y][x]
            if value in PLAYER_MARKERS:
                continue

            targets = (
                (y, x),          # bottom-left
                (y, WIDTH - 1 - x),       # bottom-right
                (HEIGHT - 1 - y, x),     # upper-left
                (HEIGHT - 1 - y, WIDTH - 1 - x),  # upper-right
            )
            for target_y, target_x in targets:
                # Never overwrite a player marker in any quadrant.
                if grid[target_y][target_x] not in PLAYER_MARKERS:
                    grid[target_y][target_x] = value

    result = "\n".join("".join(row) for row in grid)
    if trailing_newline:
        result += "\n"
    path.write_text(result)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
