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

#ifndef BACKEND_H
#define BACKEND_H

#define K_LEFT  (-1)
#define K_RIGHT (-2)
#define K_UP    (-3)
#define K_DOWN  (-4)

typedef struct {
       int x;
       int y;
       int width;
       int height;
       void* backend_data;
} Window;

typedef struct Backend {
       void (*init)(void);
       void (*exit)(void);
       int (*get_key)(int);
       void (*init_window)(Window*);
       void (*full_update)(void);
       void (*update)(Window);
       void (*print)(int, int, const char*, ...);
       void (*erase_window)(Window);
       void (*erase_line)(int, int);
       void (*draw_cell)(Window, int, int, int);
       void (*draw_box)(Window);
} Backend;

#endif
