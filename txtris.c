/* Copyright (C) 2025-2026 RZ781
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

#include <errno.h>
#include <ncurses.h>
#include <poll.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include "citrus.h"

typedef struct {
	int x;
	int y;
	int width;
	int height;
} Rect;

const char* program_name;
const CitrusPiece** next_piece_queue;
CitrusGameConfig config;
CitrusCell* board;
CitrusGame game;
void* randomizer;
Rect board_rect, next_rect, hold_rect;
bool one_key_finesse = false;

// color, rgb, default ncurses color, 8-bit color code
const int colors[7][6] = {
	{CITRUS_COLOR_I, 89, 154, 209, COLOR_CYAN, 45},
	{CITRUS_COLOR_J, 55, 67, 190, COLOR_BLUE, 21},
	{CITRUS_COLOR_L, 202, 99, 41, COLOR_YELLOW, 166},
	{CITRUS_COLOR_O, 253, 255, 12, COLOR_YELLOW, 226},
	{CITRUS_COLOR_S, 117, 174, 54, COLOR_GREEN, 118},
	{CITRUS_COLOR_T, 155, 55, 134, COLOR_MAGENTA, 129},
	{CITRUS_COLOR_Z, 188, 46, 61, COLOR_RED, 160},
};

const char* clear_names[5] = {"", "Single", "Double", "Triple", "Quad"};

const char* rows[4] = {
	"asdfghjkl;",
	"zxcvbnm,./",
	"1234567890",
	"qwertyuiop",
};

void update_window(WINDOW* win, const CitrusCell* data, int height, int width, int y_offset, int x_offset) {
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
	wnoutrefresh(win);
}

void print_action_text(const char* fmt, ...) {
	char buffer[512];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, args);
	va_end(args);
	move(2, 0);
	clrtoeol();
	int x = board_rect.x + (board_rect.width - strlen(buffer)) / 2;
	mvaddstr(2, x, buffer);
}

void update(WINDOW* board_win, WINDOW* hold_win, WINDOW* next_piece_win) {
	werase(board_win);
	werase(hold_win);
	werase(next_piece_win);
	update_window(board_win, board, config.full_height, config.width, 0, 0);
	if (game.hold_piece == NULL) {
		update_window(hold_win, NULL, 0, 0, 0, 0);
	} else {
		const CitrusCell* data = game.hold_piece->piece_data;
		int height = game.hold_piece->height;
		int width = game.hold_piece->width;
		update_window(hold_win, data, height, width, height >= 4 ? 0 : 1, 4 - height);
	}
	for (int i = 0; i < config.next_piece_queue_size; i++) {
		const CitrusPiece* piece = CitrusGame_get_next_piece(&game, i);
		const CitrusCell* data = piece->piece_data;
		int height = piece->height;
		int width = piece->width;
		update_window(next_piece_win, data, height, width, (height >= 4 ? 0 : 1) + i * 4, 4 - height);
	}
	mvprintw(hold_rect.y + hold_rect.height + 1, hold_rect.x, "Score: %i", game.score);
	mvprintw(hold_rect.y + hold_rect.height + 2, hold_rect.x, "Level: %i", game.level);
	mvprintw(hold_rect.y + hold_rect.height + 3, hold_rect.x, "Lines: %i", game.lines);
	wnoutrefresh(stdscr);
	doupdate();
}

void action_text_callback(void* data, int n_lines_cleared, int combo, bool b2b, bool all_clear, bool spin, bool mini_spin) {
	(void) data;
	if (n_lines_cleared == 0 && !spin && !mini_spin) {
		return;
	}
	const char* name = clear_names[n_lines_cleared];
	char combo_text[512] = "";
	if (combo > 0) {
		snprintf(combo_text, sizeof(combo_text), " Combo %i", combo);
	}
	print_action_text("%s%s%s%s%s", all_clear ? "All Clear " : "", b2b ? "B2B " : "", spin ? "T Spin " : mini_spin ? "Mini T Spin " : "", name, combo_text);
}

void init_citrus() {
	srand(time(NULL));
	config.action_text = action_text_callback;
	if (config.randomizer == CitrusBagRandomizer_randomizer) {
		randomizer = malloc(sizeof(CitrusBagRandomizer));
		CitrusBagRandomizer_init(randomizer, rand());
	} else {
		randomizer = malloc(sizeof(CitrusClassicRandomizer));
		CitrusClassicRandomizer_init(randomizer, rand());
	}
	board = malloc(sizeof(CitrusCell) * config.full_height * config.width);
	next_piece_queue = malloc(sizeof(CitrusPiece*) * config.next_piece_queue_size);
	CitrusGame_init(&game, board, next_piece_queue, config, randomizer, NULL);
}

void init_ncurses(void) {
	initscr();
	cbreak();
	noecho();
	use_default_colors();
	keypad(stdscr, TRUE);
	start_color();
	init_pair(1, -1, COLOR_WHITE);
	for (int i=0; i<7; i++) {
		int color = colors[i][0];
		int r = colors[i][1] * 1000 / 256;
		int g = colors[i][2] * 1000 / 256;
		int b = colors[i][3] * 1000 / 256;
		int color_4bit = colors[i][4];
		int color_8bit = colors[i][5];
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

int string_to_int(const char* s, int minimum) {
	char* endptr;
	int i = strtol(optarg, &endptr, 10);
	if (endptr == optarg || *endptr != '\0') {
		fprintf(stderr, "%s: invalid number %s\n", program_name, s);
		exit(-1);
	}
	if (i < minimum) {
		fprintf(stderr, "%s: %i is too small\n", program_name, i);
		exit(-1);
	}
	return i;
}

int main(int argc, char** argv) {
	program_name = argv[0];
	config = citrus_preset_modern;
	int c;
	while ((c = getopt(argc, argv, "1cDd:f:g:h:l:m:q:s:w:")) != -1) {
		switch (c) {
			case 'w':
				config.width = string_to_int(optarg, 4);
				break;
			case 'h':
				config.height = string_to_int(optarg, -1);
				break;
			case 'f':
				config.full_height = string_to_int(optarg, 3);
				break;
			case 'g':
				config.gravity = 1.0 / 60.0 * string_to_int(optarg, 0);
				break;
			case 'l':
				config.lock_delay = string_to_int(optarg, 1);
				break;
			case 'm':
				config.max_move_reset = string_to_int(optarg, 0);
				break;
			case 'q':
				config.next_piece_queue_size = string_to_int(optarg, 0);
				break;
			case 'd':
				config.line_clear_delay = string_to_int(optarg, 0);
				break;
			case 's':
				config.shadow = string_to_int(optarg, 0);
				break;
			case 'c':
				config = citrus_preset_classic;
				break;
			case 'D':
				config = citrus_preset_delayless;
				break;
			case '1':
				one_key_finesse = true;
				break;
			case '?':
				exit(-1);
			default:
				break;
		}
	}
	if (config.full_height == 40) {
		int extra_height = config.height;
		if (extra_height < 4)
			extra_height = 4;
		if (extra_height > 20)
			extra_height = 20;
		config.full_height = config.height + extra_height;
	}
	init_citrus();
	init_ncurses();
	hold_rect.x = 2;
	hold_rect.y = 4;
	hold_rect.width = 10;
	hold_rect.height = 6;
	board_rect.x = hold_rect.x + hold_rect.width + 2;
	board_rect.y = hold_rect.y;
	board_rect.width = config.width * 2 + 2;
	board_rect.height = config.full_height + 2;
	next_rect.x = board_rect.x + board_rect.width;
	next_rect.y = board_rect.y;
	next_rect.width = 10;
	next_rect.height = config.next_piece_queue_size * 4 + 2;
	WINDOW* hold_win = newwin(hold_rect.height, hold_rect.width, hold_rect.y, hold_rect.x);
	WINDOW* board_win = newwin(board_rect.height, board_rect.width, board_rect.y, board_rect.x);
	WINDOW* next_piece_win = newwin(next_rect.height, next_rect.width, next_rect.y, next_rect.x);
	update(board_win, hold_win, next_piece_win);
	double time_since_tick = 0;
	struct timespec curr_time, prev_time;
	clock_gettime(CLOCK_MONOTONIC, &curr_time);
	int ticks = 0;
	while (CitrusGame_is_alive(&game)) {
		struct pollfd fd = {STDIN_FILENO, POLLIN, 0};
		int ms_timeout = (1.0 / 60.0 - time_since_tick) * 1e3;
		if (ms_timeout < 0)
			ms_timeout = 0;
		if (poll(&fd, 1, ms_timeout) == -1 && errno != EINTR) {
			perror("poll");
			exit(-1);
		}
		if (fd.revents & POLLIN) {
			int c = getch();
			if (one_key_finesse) {
				int rotation, column;
				for (rotation = 0; rotation < 4; rotation++) {
					for (column = 0; column < 10; column++) {
						if (rows[rotation][column] == c) {
							goto found;
						}
					}
				}
				found:
				if (rotation != 4) {
					for (int i = 0; i < rotation; i++) {
						CitrusGame_key_down(&game, CITRUS_KEY_CLOCKWISE);
					}
					if (column < 5) {
						for (int i = 0; i < 10; i++) {
							CitrusGame_key_down(&game, CITRUS_KEY_LEFT);
						}
						for (int i = 0; i < column; i++) {
							CitrusGame_key_down(&game, CITRUS_KEY_RIGHT);
						}
					} else {
						for (int i = 0; i < 10; i++) {
							CitrusGame_key_down(&game, CITRUS_KEY_RIGHT);
						}
						for (int i = 0; i < 9 - column; i++) {
							CitrusGame_key_down(&game, CITRUS_KEY_LEFT);
						}
					}
					CitrusGame_key_down(&game, CITRUS_KEY_HARD_DROP);
				} else if (c == ' ') {
					CitrusGame_key_down(&game, CITRUS_KEY_HOLD);
				}
			} else {
				int key = -1;
				switch (c) {
					case KEY_LEFT: key = CITRUS_KEY_LEFT; break;
					case KEY_RIGHT: key = CITRUS_KEY_RIGHT; break;
					case KEY_DOWN: key = CITRUS_KEY_SOFT_DROP; break;
					case ' ': key = CITRUS_KEY_HARD_DROP; break;
					case 'z': key = CITRUS_KEY_ANTICLOCKWISE; break;
					case 'x': case KEY_UP: key = CITRUS_KEY_CLOCKWISE; break;
					case 'c': key = CITRUS_KEY_HOLD; break;
					case 'a': key = CITRUS_KEY_180; break;
				}
				CitrusGame_key_down(&game, key);
			}
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
		update(board_win, hold_win, next_piece_win);
	}
	print_action_text("You died");
	refresh();
	sleep(5);
	endwin();
	free(board);
	free(next_piece_queue);
	free(randomizer);
	return 0;
}
