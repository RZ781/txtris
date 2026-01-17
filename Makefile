# Copyright (C) 2026 RZ781
#
# This file is part of txtris.
#
# txtris is free software: you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation, either version 3 of the License, or (at
# your option) any later version.
#
# txtris is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see
# <https://www.gnu.org/licenses/>.

LIBCITRUS_PATH ?= libcitrus
CFLAGS = -Wall -Wextra -Wpedantic -I$(LIBCITRUS_PATH)/include $(shell pkg-config --cflags ncursesw) $(shell pkg-config --cflags raylib) -DNCURSES_BACKEND -DRAYLIB_BACKEND

.PHONY: libcitrus

txtris: txtris.o raylib.o libcitrus $(LIBCITRUS_PATH)/libcitrus.a
	gcc -o txtris txtris.o raylib.o -L"$(LIBCITRUS_PATH)" -Wl,-Bstatic -lcitrus -Wl,-Bdynamic $$(pkg-config --libs ncursesw) $$(pkg-config --libs raylib)

libcitrus:
	make -C libcitrus

%.o: %.c
	gcc $(CFLAGS) -c $< -o $@
