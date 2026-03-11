/* Copyright (C) 2026 RZ781
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

#include <ncurses.h>
#include "backend.h"
#include "citrus.h"

// color, rgb, default ncurses color, 8-bit color code
const int ncurses_colors[7][6] = {
	{CITRUS_COLOR_I, 89, 154, 209, COLOR_CYAN, 45},
	{CITRUS_COLOR_J, 55, 67, 190, COLOR_BLUE, 21},
	{CITRUS_COLOR_L, 202, 99, 41, COLOR_YELLOW, 166},
	{CITRUS_COLOR_O, 253, 255, 12, COLOR_YELLOW, 226},
	{CITRUS_COLOR_S, 117, 174, 54, COLOR_GREEN, 118},
	{CITRUS_COLOR_T, 155, 55, 134, COLOR_MAGENTA, 129},
	{CITRUS_COLOR_Z, 188, 46, 61, COLOR_RED, 160},
};

void ncurses_init(void) {
	initscr();
	cbreak();
	noecho();
	use_default_colors();
	keypad(stdscr, TRUE);
	start_color();
	init_pair(1, -1, COLOR_WHITE);
	for (int i=0; i < 7; i++) {
		int color = ncurses_colors[i][0];
		int r = ncurses_colors[i][1] * 1000 / 256;
		int g = ncurses_colors[i][2] * 1000 / 256;
		int b = ncurses_colors[i][3] * 1000 / 256;
		int color_4bit = ncurses_colors[i][4];
		int color_8bit = ncurses_colors[i][5];
		if (can_change_color() && COLORS >= 16+7) {
			init_color(i+16, r, g, b);
			init_pair(color + 2, -1, i+16);
		} else if (COLORS >= 256) {
			init_pair(color + 2, -1, color_8bit);
		} else {
			init_pair(color + 2, -1, color_4bit);
		}
	}
	curs_set(0);
}

void ncurses_exit(void) {
	endwin();
}

KeyType ncurses_get_key(int ms_timeout, int* key) {
	timeout(ms_timeout);
	int ch = getch();
	switch (ch) {
		case KEY_LEFT: *key = K_LEFT; break;
		case KEY_RIGHT: *key = K_RIGHT; break;
		case KEY_UP: *key = K_UP; break;
		case KEY_DOWN: *key = K_DOWN; break;
		case KEY_RESIZE: *key = K_RESIZE; break;
		case ERR: return KEYTYPE_NONE;
		default: *key = ch;
	}
	return KEYTYPE_PRESS;
}

void ncurses_init_window(Window* window) {
	 window->backend_data = newwin(window->height, window->width, window->y, window->x);
}

void ncurses_full_update(void) {
	wnoutrefresh(stdscr);
	doupdate();
}

void ncurses_print(int y, int x, const char* format, ...) {
	va_list args;
	move(y, x);
	va_start(args, format);
	vw_printw(stdscr, format, args);
	va_end(args);
}

void ncurses_update(Window window) {
	wnoutrefresh(window.backend_data);
}

void ncurses_erase_window(Window window) {
	werase(window.backend_data);
}

void ncurses_erase_line(int x, int y) {
	move(y, x);
 	clrtoeol();
}

void ncurses_clear_screen(void) {
	erase();
}

void ncurses_draw_cell(Window window, int x, int y, int color) {
	int ch = ' ' | COLOR_PAIR(color);
	mvwaddch(window.backend_data, y, x, ch);
	mvwaddch(window.backend_data, y, x + 1, ch);
}

void ncurses_draw_box(Window window) {
	box(window.backend_data, 0, 0);
}

void ncurses_get_size(int* width, int* height) {
	getmaxyx(stdscr, *height, *width);
}

void ncurses_set_target_size(int width, int height) {
	(void) width; (void) height;
}

void ncurses_resize_window(Window* window) {
	mvwin(window->backend_data, window->y, window->x);
	wresize(window->backend_data, window->height, window->width);
}

Backend ncurses_backend = {
	.init = ncurses_init,
	.exit = ncurses_exit,
	.get_key = ncurses_get_key,
	.init_window = ncurses_init_window,
	.resize_window = ncurses_resize_window,
	.full_update = ncurses_full_update,
	.update = ncurses_update,
	.print = ncurses_print,
	.erase_window = ncurses_erase_window,
	.erase_line = ncurses_erase_line,
	.clear_screen = ncurses_clear_screen,
	.draw_cell = ncurses_draw_cell,
	.draw_box = ncurses_draw_box,
	.get_size = ncurses_get_size,
	.set_target_size = ncurses_set_target_size,
};
