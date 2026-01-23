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

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "backend.h"
#include "citrus.h"

Backend backend;

#ifdef NCURSES_BACKEND
extern Backend ncurses_backend;
#endif

#ifdef SDL3_BACKEND
extern Backend sdl3_backend;
#endif

const char* program_name;
const CitrusPiece** next_piece_queue;
CitrusGameConfig config;
CitrusCell* board;
CitrusGame game;
void* randomizer;
Window board_win, next_piece_win, hold_win;
bool one_key_finesse = false;
int pieces = 0;
int ticks = 0;

const char* clear_names[5] = {"", "Single", "Double", "Triple", "Quad"};

const char* rows[4] = {
	"asdfghjkl;",
	"zxcvbnm,./",
	"1234567890",
	"qwertyuiop",
};

void update_window(Window win, const CitrusCell* data, int height, int width, int y_offset, int x_offset) {
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			CitrusCell cell = data[y * width + x];
			int color;
			if (cell.type == CITRUS_CELL_FULL)
				color = cell.color + 2;
			else if (cell.type == CITRUS_CELL_SHADOW)
				color = 1;
			else
				color = 0;
			backend.draw_cell(win, x * 2 + x_offset + 1, height - y + y_offset, color);
		}
	}
	backend.draw_box(win);
	backend.update(win);
}

void print_action_text(const char* fmt, ...) {
	char buffer[512];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, args);
	va_end(args);
	backend.erase_line(0, 2);
	int x = board_win.x + (board_win.width - strlen(buffer)) / 4;
	backend.print(2, x, buffer);
	backend.full_update();
}

void update(void) {
	backend.erase_window(board_win);
	backend.erase_window(hold_win);
	backend.erase_window(next_piece_win);
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
	backend.print(hold_win.y + hold_win.height + 1, hold_win.x, "Score: %i", game.score);
	backend.print(hold_win.y + hold_win.height + 2, hold_win.x, "Level: %i", game.level);
	backend.print(hold_win.y + hold_win.height + 3, hold_win.x, "Lines: %i", game.lines);
	backend.print(hold_win.y + hold_win.height + 4, hold_win.x, "  PPS: %.2f", ((float)pieces)/((float)ticks/60.0));
	backend.full_update();
}

void action_text_callback(void* data, int n_lines_cleared, int combo, bool b2b, bool all_clear, bool spin, bool mini_spin) {
	pieces++;
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
#ifdef NCURSES_BACKEND
	backend = ncurses_backend;
#elif defined SDL3_BACKEND
	backend = sdl3_backend;
#else
#error "no backend selected"
#endif
	while ((c = getopt(argc, argv, "1cDSd:f:g:h:l:m:q:s:w:")) != -1) {
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
			case 'S':
#ifdef SDL3_BACKEND
				backend = sdl3_backend;
				break;
#else
				fprintf(stderr, "%s: sdl3 not included\n", program_name);
				exit(-1);
#endif
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
	backend.init();
	hold_win.x = 2;
	hold_win.y = 4;
	hold_win.width = 10;
	hold_win.height = 6;
	board_win.x = hold_win.x + hold_win.width + 2;
	board_win.y = hold_win.y;
	board_win.width = config.width * 2 + 2;
	board_win.height = config.full_height + 2;
	next_piece_win.x = board_win.x + board_win.width;
	next_piece_win.y = board_win.y;
	next_piece_win.width = 10;
	next_piece_win.height = config.next_piece_queue_size * 4 + 2;
	backend.init_window(&hold_win);
	backend.init_window(&board_win);
	backend.init_window(&next_piece_win);
	update();
	double time_since_tick = 0;
	struct timespec curr_time, prev_time;
	clock_gettime(CLOCK_MONOTONIC, &curr_time);
	while (CitrusGame_is_alive(&game)) {
		int ms_timeout = (1.0 / 60.0 - time_since_tick) * 1e3;
		if (ms_timeout < 0)
			ms_timeout = 0;
		int c = backend.get_key(ms_timeout);
		if (c != 0) {
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
					case K_LEFT: key = CITRUS_KEY_LEFT; break;
					case K_RIGHT: key = CITRUS_KEY_RIGHT; break;
					case K_DOWN: key = CITRUS_KEY_SOFT_DROP; break;
					case ' ': key = CITRUS_KEY_HARD_DROP; break;
					case 'z': key = CITRUS_KEY_ANTICLOCKWISE; break;
					case 'x': case K_UP: key = CITRUS_KEY_CLOCKWISE; break;
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
		update();
	}
	print_action_text("You died");
	sleep(5);
	backend.exit();
	free(board);
	free(next_piece_queue);
	free(randomizer);
	return 0;
}
