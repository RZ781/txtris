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

#include <stdio.h>
#include <stdarg.h>
#include <raylib.h>
#include "backend.h"
#include "citrus.h"

#define CELL_WIDTH 5
#define CELL_HEIGHT 10

void raylib_init(void) {
	InitWindow(CELL_WIDTH * 60, CELL_HEIGHT * 46, "txtris");
	SetTargetFPS(60);
	BeginDrawing();
}

void raylib_exit(void) {
	EndDrawing();
	CloseWindow();
}

int raylib_get_key(int timeout) {
	// TODO
	return 0;
}

void raylib_init_window(Window* window) {
	(void) window;
}

void raylib_full_update(void) {
	EndDrawing();
	BeginDrawing();
}

void raylib_update(Window window) {
//	raylib_full_update();
}

void raylib_print(int y, int x, const char* fmt, ...) {
	char buffer[512];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, args);
	va_end(args);
	DrawText(buffer, x * CELL_WIDTH, y * CELL_HEIGHT, CELL_HEIGHT, WHITE);
}

void raylib_erase_window(Window window) {
	DrawRectangle(window.x * CELL_WIDTH, window.y * CELL_HEIGHT, window.width * CELL_WIDTH, window.height * CELL_HEIGHT, BLACK);
}

void raylib_erase_line(int x, int y) {
	// TODO
}

void raylib_draw_cell(Window window, int x, int y, int color) {
	Color raylib_color;
	switch (color) {
		case 0: raylib_color = BLACK; break;
		case 1: raylib_color = WHITE; break;
		case 2 + CITRUS_COLOR_I: raylib_color = SKYBLUE; break;
		case 2 + CITRUS_COLOR_J: raylib_color = BLUE; break;
		case 2 + CITRUS_COLOR_L: raylib_color = ORANGE; break;
		case 2 + CITRUS_COLOR_O: raylib_color = YELLOW; break;
		case 2 + CITRUS_COLOR_S: raylib_color = LIME; break;
		case 2 + CITRUS_COLOR_T: raylib_color = MAGENTA; break;
		case 2 + CITRUS_COLOR_Z: raylib_color = RED; break;
	}
	DrawRectangle((window.x + x) * CELL_WIDTH, (window.y + y) * CELL_HEIGHT, CELL_WIDTH * 2, CELL_HEIGHT, raylib_color);
}

void raylib_draw_box(Window window) {
	DrawRectangleLines(window.x * CELL_WIDTH, window.y * CELL_HEIGHT, window.width * CELL_WIDTH, window.height * CELL_HEIGHT, WHITE);
}

Backend raylib_backend = {
	.init = raylib_init,
	.exit = raylib_exit,
	.get_key = raylib_get_key,
	.init_window = raylib_init_window,
	.full_update = raylib_full_update,
	.update = raylib_update,
	.print = raylib_print,
	.erase_window = raylib_erase_window,
	.erase_line = raylib_erase_line,
	.draw_cell = raylib_draw_cell,
	.draw_box = raylib_draw_box
};
