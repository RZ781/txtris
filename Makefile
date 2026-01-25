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
USE_NCURSES ?= $(shell pkg-config ncursesw && echo 1 || echo 0)
USE_SDL3 ?= $(shell pkg-config sdl3 && echo 1 || echo 0)
CFLAGS := -Wall -Wextra -Wpedantic -Iinclude -I$(LIBCITRUS_PATH)/include
LDFLAGS := -L"$(LIBCITRUS_PATH)" -Wl,-Bstatic -lcitrus -Wl,-Bdynamic $(LDFLAGS)
SOURCE = src/txtris.c
OBJECT = $(SOURCE:.c=.o)
INCLUDE = $(wildcard include/*.h)

ifeq ($(USE_NCURSES), 1)
	CFLAGS += $(shell pkg-config --cflags ncursesw) -DNCURSES_BACKEND
	LDFLAGS += $(shell pkg-config --libs ncursesw)
	SOURCE += src/ncurses.c
endif

ifeq ($(USE_SDL3), 1)
	CFLAGS += $(shell pkg-config --cflags sdl3) -DSDL3_BACKEND
	LDFLAGS += $(shell pkg-config --libs sdl3)
	SOURCE += src/sdl3.c
endif

.PHONY: all clean distclean

all: txtris

clean:
	$(RM) $(OBJECT)

distclean: clean
	$(RM) txtris

txtris: $(OBJECT) $(LIBCITRUS_PATH)/libcitrus.a
	$(MAKE) -C libcitrus
	$(CC) -o $@ $(OBJECT) $(LDFLAGS)

src/%.o: src/%.c $(INCLUDE)
	$(CC) $(CFLAGS) -c $< -o $@
