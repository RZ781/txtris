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
#include <poll.h>
#include <sys/types.h>
#include "citrus.h"

const char* program_name;
int width = 10;
int height = 20;
int full_height = 40;
CitrusCell* board;
CitrusGame game;
CitrusBagRandomizer bag;

void update_window(WINDOW* win, const CitrusCell* data, int height, int width, int y_offset, int x_offset) {
	werase(win);
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			CitrusCell cell = data[y * width + x];
			int ch;
			if (cell.type == CITRUS_CELL_FULL)
				ch = ' ' | COLOR_PAIR(cell.color + 2);
			else if (cell.type == CITRUS_CELL_SHADOW)
				ch = ' ' | COLOR_PAIR(1);
			else
				ch = ' ' | COLOR_PAIR(0);
			mvwaddch(win, height - y + y_offset, x * 2 + 1 + x_offset, ch);
			mvwaddch(win, height - y + y_offset, x * 2 + 2 + x_offset, ch);
		}
	}
	box(win, 0, 0);
	refresh();
	wrefresh(win);
}

void update(WINDOW* board_win, WINDOW* hold_win) {
	update_window(board_win, board, full_height, width, 0, 0);
	if (game.hold_piece == NULL) {
		update_window(hold_win, NULL, 0, 0, 0, 0);
	} else {
		const CitrusCell* data = game.hold_piece->piece_data;
		int height = game.hold_piece->height;
		int width = game.hold_piece->width;
		update_window(hold_win, data, height, width, height >= 4 ? 0 : 1, 4 - height);
	}
}

void init_citrus(int width, int height, int full_height) {
	srand(time(NULL));
	CitrusBagRandomizer_init(&bag, rand());
	CitrusGameConfig config;
	CitrusGameConfig_init(&config, CitrusBagRandomizer_randomizer);
	config.width = width;
	config.height = height;
	config.full_height = full_height;
	board = malloc(sizeof(CitrusCell) * full_height * width);
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
	init_pair(CITRUS_COLOR_O + 2, -1, COLOR_YELLOW);
	init_pair(CITRUS_COLOR_T + 2, -1, COLOR_MAGENTA);
	init_pair(CITRUS_COLOR_S + 2, -1, COLOR_GREEN);
	init_pair(CITRUS_COLOR_Z + 2, -1, COLOR_RED);
	init_pair(CITRUS_COLOR_L + 2, -1, COLOR_YELLOW);
	init_pair(CITRUS_COLOR_J + 2, -1, COLOR_BLUE);
	init_pair(CITRUS_COLOR_I + 2, -1, COLOR_CYAN);
	curs_set(0);
}

int string_to_int(const char* s) {
	char* endptr;
	int i = strtol(optarg, &endptr, 10);
	if (endptr == optarg || *endptr != '\0') {
		fprintf(stderr, "%s: invalid integer %s", program_name, s);
		exit(-1);
	}
	return i;
}

int main(int argc, char** argv) {
	program_name = argv[0];
	int c;
	while ((c = getopt(argc, argv, "f:h:w:")) != -1) {
		switch (c) {
			case 'w':
				width = string_to_int(optarg);
				break;
			case 'h':
				height = string_to_int(optarg);
				break;
			case 'f':
				full_height = string_to_int(optarg);
				break;
			case '?':
				exit(-1);
			default:
				break;
		}
	}
	init_citrus(width, height, full_height);
	init_ncurses();
	WINDOW* hold_win = newwin(6, 10, 4, 2);
	WINDOW* board_win = newwin(full_height + 2, width*2 + 2, 4, 12);
	update(board_win, hold_win);
	double time_since_tick = 0;
	struct timespec curr_time, prev_time;
	clock_gettime(CLOCK_MONOTONIC, &curr_time);
	int ticks = 0;
	while (CitrusGame_is_alive(&game)) {
		struct pollfd fd = {STDIN_FILENO, POLLIN, 0};
		int ms_timeout = (1.0 / 60.0 - time_since_tick) * 1e3;
		if (ms_timeout < 0)
			ms_timeout = 0;
		if (poll(&fd, 1, ms_timeout) == -1) {
			perror("poll");
			exit(-1);
		}
		if (fd.revents & POLLIN) {
			int c = getch();
			int key = -1;
			switch (c) {
				case KEY_LEFT: key = CITRUS_KEY_LEFT; break;
				case KEY_RIGHT: key = CITRUS_KEY_RIGHT; break;
				case KEY_DOWN: key = CITRUS_KEY_SOFT_DROP; break;
				case ' ': key = CITRUS_KEY_HARD_DROP; break;
				case 'z': key = CITRUS_KEY_ANTICLOCKWISE; break;
				case 'x': case KEY_UP: key = CITRUS_KEY_CLOCKWISE; break;
				case 'c': key = CITRUS_KEY_HOLD; break;
			}
			CitrusGame_key_down(&game, key);
		}
		prev_time = curr_time;
		clock_gettime(CLOCK_MONOTONIC, &curr_time);
		time_since_tick += curr_time.tv_sec - prev_time.tv_sec;
		time_since_tick += (curr_time.tv_nsec - prev_time.tv_nsec) / 1e9;
		while (time_since_tick >= 1.0 / 60.0) {
			time_since_tick -= 1.0 / 60.0;
			CitrusGame_tick(&game);
			ticks++;
		}
		update(board_win, hold_win);
	}
	mvprintw(2, 2, "You died");
	refresh();
	sleep(5);
	endwin();
	free(board);
	return 0;
}
