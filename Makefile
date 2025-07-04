#!/usr/bin/env

CFLAGS += -Wall -Wextra -Werror
CFLAGS += -std=gnu11
CFLAGS += -Iproto
CFLAGS += -fPIC
CFLAGS += $(shell pkg-config --cflags egl glesv2 wayland-client wayland-egl wayland-protocols)

CXXFLAGS += -Wall -Wextra
CXXFLAGS += -std=gnu++17
CXXFLAGS += $(shell pkg-config --cflags egl glesv2 wayland-client wayland-egl wayland-protocols)

LDFLAGS += -Wl,-no-undefined
LDFLAGS += $(shell pkg-config --libs egl glesv2 wayland-client wayland-egl wayland-protocols)

WAYLAND_PROTOCOLS_DIR = $(shell pkg-config --variable=pkgdatadir wayland-protocols)
WAYLAND_PROTOCOL_FILE_XDG_SHELL = $(WAYLAND_PROTOCOLS_DIR)/stable/xdg-shell/xdg-shell.xml

TARGETS = qt-host wayland-audio-plugin-test wayland-audio-plugin-test.lv2/plugin.so

all: build

build: $(TARGETS)

clean:
	rm -f $(TARGETS) *.o proto/*.*

run: wayland-audio-plugin-test
	valgrind --leak-check=full ./wayland-audio-plugin-test

qt-host: qt-host.cpp app.o proto/xdg-shell.o
	$(CXX) $^ $(CXXFLAGS) $(LDFLAGS) $(shell pkg-config --cflags --libs Qt6Widgets) -o $@

wayland-audio-plugin-test: app.o main.o proto/xdg-shell.o
	$(CC) $^ $(LDFLAGS) -o $@

wayland-audio-plugin-test.lv2/plugin.so: app.o plugin.o proto/xdg-shell.o
	$(CC) $^ $(LDFLAGS) -shared -o $@

%.o: %.c glview.h
	$(CC) $< $(CFLAGS) -c -o $@

# extra protocol deps
app.c: proto/xdg-shell.h

proto/xdg-shell.h:
	wayland-scanner client-header "$(WAYLAND_PROTOCOL_FILE_XDG_SHELL)" $@

proto/xdg-shell.c:
	wayland-scanner private-code "$(WAYLAND_PROTOCOL_FILE_XDG_SHELL)" $@
