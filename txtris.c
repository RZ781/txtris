/* Copyright (C) 2025 RZ781
 *
 * This file is part of txtris.
 *
 * txtris is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at
 * your option) any later version.
 *
 * txtris is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see
 * <https://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <ncurses.h>
#include "citrus.h"

CitrusCell board[10 * 40];
CitrusGame game;
CitrusBagRandomizer bag;

void update(WINDOW* win, CitrusCell* board) {
	for (int y=0; y<40; y++) {
		for (int x=0; x<10; x++) {
			CitrusCell cell = board[y * 10 + x];
			int ch;
			if (cell.type == CITRUS_CELL_FULL)
				ch = ' ' | COLOR_PAIR(cell.colour + 2);
			else if (cell.type == CITRUS_CELL_SHADOW)
				ch = ' ' | COLOR_PAIR(1);
			else
				ch = ' ' | COLOR_PAIR(0);
			mvwaddch(win, 40 - y, x * 2 + 1, ch);
			mvwaddch(win, 40 - y, x * 2 + 2, ch);
		}
	}
	box(win, 0, 0);
	refresh();
	wrefresh(win);
}

void init_citrus() {
	srand(time(NULL));
	CitrusBagRandomizer_init(&bag, rand());
	CitrusGameConfig config;
	CitrusGameConfig_init(&config, CitrusBagRandomizer_randomizer);
	CitrusGame_init(&game, board, config, &bag);
}

void init_ncurses(void) {
	initscr();
	cbreak();
	noecho();
	use_default_colors();
	keypad(stdscr, TRUE);
	start_color();
	init_pair(1, -1, COLOR_WHITE);
	init_pair(CITRUS_COLOUR_O + 2, -1, COLOR_YELLOW);
	init_pair(CITRUS_COLOUR_T + 2, -1, COLOR_MAGENTA);
	init_pair(CITRUS_COLOUR_S + 2, -1, COLOR_GREEN);
	init_pair(CITRUS_COLOUR_Z + 2, -1, COLOR_RED);
	init_pair(CITRUS_COLOUR_L + 2, -1, COLOR_BLUE);
	init_pair(CITRUS_COLOUR_J + 2, -1, COLOR_YELLOW);
	init_pair(CITRUS_COLOUR_I + 2, -1, COLOR_CYAN);
	timeout(0);
}

int main() {
	init_citrus();
	init_ncurses();
	WINDOW* win = newwin(42, 22, 4, 2);
	update(win, board);
	while (CitrusGame_is_alive(&game)) {
		int c = getch();
		int key = -1;
		if (c == KEY_LEFT) {
			key = CITRUS_KEY_LEFT;
		} else if (c == KEY_RIGHT) {
			key = CITRUS_KEY_RIGHT;
		} else if (c == KEY_DOWN) {
			key = CITRUS_KEY_SOFT_DROP;
		} else if (c == ' ') {
			key = CITRUS_KEY_HARD_DROP;
		} else if (c == 'z') {
			key = CITRUS_KEY_ANTICLOCKWISE;
		} else if (c == 'x') {
			key = CITRUS_KEY_CLOCKWISE;
		}
		CitrusGame_key_down(&game, key);
		update(win, board);
	}
	mvprintw(2, 2, "You died");
	refresh();
	sleep(5);
	endwin();
	return 0;
}
