#!/usr/bin/env

CFLAGS += -Wall -Wextra -Werror
CFLAGS += -std=gnu11
CFLAGS += -Iproto
CFLAGS += $(shell pkg-config --cflags egl glesv2 wayland-client wayland-egl wayland-protocols)
LDFLAGS += $(shell pkg-config --libs egl glesv2 wayland-client wayland-egl wayland-protocols)

WAYLAND_PROTOCOLS_DIR = $(shell pkg-config --variable=pkgdatadir wayland-protocols)
WAYLAND_PROTOCOL_FILE_XDG_SHELL = $(WAYLAND_PROTOCOLS_DIR)/stable/xdg-shell/xdg-shell.xml

all: build

build: wayland-audio-plugin-test

clean:
	rm -f wayland-audio-plugin-test *.o proto/*.*

run: wayland-audio-plugin-test
	valgrind --leak-check=full ./wayland-audio-plugin-test

wayland-audio-plugin-test: main.o proto/xdg-shell.o
	$(CC) $^ $(LDFLAGS) -o $@

%.o: %.c glview.h
	$(CC) $< $(CFLAGS) -c -o $@

# extra protocol deps
main.c: proto/xdg-shell.h

proto/xdg-shell.h:
	wayland-scanner client-header "$(WAYLAND_PROTOCOL_FILE_XDG_SHELL)" $@

proto/xdg-shell.c:
	wayland-scanner private-code "$(WAYLAND_PROTOCOL_FILE_XDG_SHELL)" $@
