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

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "backend.h"
#include "citrus.h"

const SDL_Color colors[7] = {
	[CITRUS_COLOR_I] = {89, 154, 209},
	[CITRUS_COLOR_J] = {55, 67, 190},
	[CITRUS_COLOR_L] = {202, 99, 41},
	[CITRUS_COLOR_O] = {253, 255, 12},
	[CITRUS_COLOR_S] = {117, 174, 54},
	[CITRUS_COLOR_T] = {155, 55, 134},
	[CITRUS_COLOR_Z] = {188, 46, 61}
};

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
TTF_TextEngine* text_engine = NULL;
TTF_Font* font = NULL;
int cell_width = 5;
int cell_height = 10;
const char* font_path = "";

void sdl3_init(void) {
	SDL_Init(SDL_INIT_VIDEO);
	TTF_Init();
	window = SDL_CreateWindow("txtris", cell_width * 50, cell_height * 50, SDL_WINDOW_RESIZABLE);
	renderer = SDL_CreateRenderer(window, NULL);
	text_engine = TTF_CreateRendererTextEngine(renderer);
	font = TTF_OpenFont(font_path, cell_height);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);
}

void sdl3_exit(void) {
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

KeyType sdl3_get_key(int timeout, int* key) {
	SDL_Event event;
	if (SDL_WaitEventTimeout(&event, timeout) == 0) {
		return KEYTYPE_NONE;
	}
	if (event.type == SDL_EVENT_QUIT)
		exit(0);
	if (event.type == SDL_EVENT_WINDOW_RESIZED) {
		cell_width = event.window.data1 / 50;
		cell_height = event.window.data2 / 50;
		if (cell_width * 2 < cell_height) {
			cell_height = cell_width * 2;
		}
		else {
			cell_width = cell_height / 2;
		}
		TTF_CloseFont(font);
		font = TTF_OpenFont(font_path, cell_height);
	}
	if (event.type == SDL_EVENT_KEY_DOWN || event.type == SDL_EVENT_KEY_UP) {
		if (event.key.repeat) {
			return KEYTYPE_NONE;
		}
		switch (event.key.key) {
			case SDLK_LEFT: *key = K_LEFT; break;
			case SDLK_RIGHT: *key = K_RIGHT; break;
			case SDLK_UP: *key = K_UP; break;
			case SDLK_DOWN: *key = K_DOWN; break;
			default: *key = event.key.key;
		}
		return event.key.down ? KEYTYPE_DOWN : KEYTYPE_UP;
	}
	return KEYTYPE_NONE;
}

void sdl3_init_window(Window* window) {
	(void) window;
}

void sdl3_full_update(void) {
	SDL_RenderPresent(renderer);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);
}

void sdl3_update(Window window) {
	(void) window;
}

void sdl3_print(int y, int x, const char* fmt, ...) {
	char buffer[512];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, args);
	va_end(args);
	TTF_Text* text = TTF_CreateText(text_engine, font, buffer, 0);
	TTF_DrawRendererText(text, cell_width * x, cell_height * y);
}

void sdl3_erase_window(Window window) {
	SDL_FRect r = {
		window.x * cell_width,
		window.y * cell_height,
		window.width * cell_width,
		window.height * cell_height
	};
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderFillRect(renderer, &r);
}

void sdl3_erase_line(int x, int y) {
	// TODO
}

void sdl3_draw_cell(Window window, int x, int y, int color) {
	SDL_Color c;
	if (color == 0) {
		c = (SDL_Color) {0, 0, 0, 255};
	} else if (color == 1) {
		c = (SDL_Color) {255, 255, 255, 255};
	} else {
		c = colors[color - 2];
	}
	SDL_FRect r = {
		(window.x + x) * cell_width,
		(window.y + y) * cell_height,
		cell_width * 2,
		cell_height
	};
	SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
	SDL_RenderFillRect(renderer, &r);
}

void sdl3_draw_box(Window window) {
	SDL_FRect r = {
		window.x * cell_width,
		window.y * cell_height,
		window.width * cell_width,
		window.height * cell_height
	};
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	SDL_RenderRect(renderer, &r);
}

void sdl3_get_size(int* width, int* height) {
	int pixel_width, pixel_height;
	SDL_GetWindowSize(window, &pixel_width, &pixel_height);
	*width = pixel_width / cell_width;
	*height = pixel_height / cell_height;
}

Backend sdl3_backend = {
	.init = sdl3_init,
	.exit = sdl3_exit,
	.get_key = sdl3_get_key,
	.init_window = sdl3_init_window,
	.full_update = sdl3_full_update,
	.update = sdl3_update,
	.print = sdl3_print,
	.erase_window = sdl3_erase_window,
	.erase_line = sdl3_erase_line,
	.draw_cell = sdl3_draw_cell,
	.draw_box = sdl3_draw_box,
	.get_size = sdl3_get_size
};
