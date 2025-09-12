#!/usr/bin/make -f

CFLAGS += -Wall -Wextra -Werror -Wno-incompatible-pointer-types
CFLAGS += -std=gnu11
CFLAGS += -fPIC
CFLAGS += -O0 -g
CFLAGS += -Iproto
CFLAGS += $(shell pkg-config --cflags egl glesv2 wayland-client wayland-egl wayland-protocols)

CXXFLAGS += -Wall -Wextra
CXXFLAGS += -std=gnu++17
CXXFLAGS += -fPIC
CXXFLAGS += -O0 -g
CXXFLAGS += $(shell pkg-config --cflags egl glesv2 wayland-client wayland-egl wayland-protocols)

LDFLAGS += -Wl,-no-undefined
LDFLAGS += $(shell pkg-config --libs egl glesv2 wayland-client wayland-egl wayland-protocols)

WAYLAND_PROTOCOLS_DIR = $(shell pkg-config --variable=pkgdatadir wayland-protocols)
WAYLAND_PROTOCOL_FILE_XDG_DECORATION = $(WAYLAND_PROTOCOLS_DIR)/unstable/xdg-decoration/xdg-decoration-unstable-v1.xml
WAYLAND_PROTOCOL_FILE_XDG_FOREIGN = $(WAYLAND_PROTOCOLS_DIR)/unstable/xdg-foreign/xdg-foreign-unstable-v2.xml
WAYLAND_PROTOCOL_FILE_XDG_SHELL = $(WAYLAND_PROTOCOLS_DIR)/stable/xdg-shell/xdg-shell.xml

TARGETS = gtk3-embed gtk4-embed gtk3-host gtk4-host qt5-host qt6-host test-crash wayland-audio-plugin-test wayland-audio-plugin-test.lv2/plugin.so wl-host

all: build

build: $(TARGETS)

clean:
	rm -f $(TARGETS) *.o proto/*.c  proto/*.h proto/*.o

run: wayland-audio-plugin-test
	valgrind --leak-check=full ./wayland-audio-plugin-test

gtk3-embed: gtk3-embed.c app.o gtk-wayland-decoration.o proto/xdg-decoration.o proto/xdg-foreign.o proto/xdg-shell.o
	$(CC) $^ $(CFLAGS) $(LDFLAGS) $(shell pkg-config --cflags --libs gtk+-wayland-3.0) -o $@

gtk4-embed: gtk4-embed.c app.o gtk-wayland-decoration.o proto/xdg-decoration.o proto/xdg-foreign.o proto/xdg-shell.o
	$(CC) $^ $(CFLAGS) $(LDFLAGS) $(shell pkg-config --cflags --libs gtk4-wayland) -o $@

gtk3-host: gtk3-host.c app.o gtk-wayland-decoration.o proto/xdg-decoration.o proto/xdg-foreign.o proto/xdg-shell.o
	$(CC) $^ $(CFLAGS) $(LDFLAGS) $(shell pkg-config --cflags --libs gtk+-wayland-3.0) -o $@

gtk4-host: gtk4-host.c app.o gtk-wayland-decoration.o proto/xdg-decoration.o proto/xdg-foreign.o proto/xdg-shell.o
	$(CC) $^ $(CFLAGS) $(LDFLAGS) $(shell pkg-config --cflags --libs gtk4-wayland) -o $@

qt5-host: qt-host.cpp app.o gtk-wayland-decoration.o proto/xdg-decoration.o proto/xdg-foreign.o proto/xdg-shell.o
	$(CXX) $^ $(CXXFLAGS) $(LDFLAGS) $(shell pkg-config --cflags --libs Qt5WaylandClient Qt5Widgets) -o $@

test-crash: test-crash.o wayland-audio-plugin-test.lv2/plugin.so
	$(CC) $^ $(CFLAGS) $(LDFLAGS) -o $@

qt6-host: qt-host.cpp app.o gtk-wayland-decoration.o proto/xdg-decoration.o proto/xdg-foreign.o proto/xdg-shell.o
	$(CXX) $^ $(CXXFLAGS) $(LDFLAGS) $(shell pkg-config --cflags --libs Qt6WaylandClient Qt6Widgets) -o $@

wayland-audio-plugin-test: app.o gtk-wayland-decoration.o main.o proto/xdg-decoration.o proto/xdg-foreign.o proto/xdg-shell.o
	$(CC) $^ $(LDFLAGS) -o $@

wayland-audio-plugin-test.lv2/plugin.so: app.o gtk-wayland-decoration.o plugin.o proto/xdg-decoration.o proto/xdg-foreign.o proto/xdg-shell.o
	$(CC) $^ $(LDFLAGS) -Wl,-z,nodelete -shared -o $@

wl-host: app.o gtk-wayland-decoration.o wl-host.o proto/xdg-decoration.o proto/xdg-foreign.o proto/xdg-shell.o
	$(CC) $^ $(LDFLAGS) -o $@

%.o: %.c
	$(CC) $< $(CFLAGS) -c -o $@

# extra protocol deps
app.c: proto/xdg-decoration.h proto/xdg-foreign.h proto/xdg-shell.h

proto/xdg-decoration.h:
	wayland-scanner client-header "$(WAYLAND_PROTOCOL_FILE_XDG_DECORATION)" $@

proto/xdg-decoration.c:
	wayland-scanner private-code "$(WAYLAND_PROTOCOL_FILE_XDG_DECORATION)" $@

proto/xdg-foreign.h:
	wayland-scanner client-header "$(WAYLAND_PROTOCOL_FILE_XDG_FOREIGN)" $@

proto/xdg-foreign.c:
	wayland-scanner private-code "$(WAYLAND_PROTOCOL_FILE_XDG_FOREIGN)" $@

proto/xdg-shell.h:
	wayland-scanner client-header "$(WAYLAND_PROTOCOL_FILE_XDG_SHELL)" $@

proto/xdg-shell.c:
	wayland-scanner private-code "$(WAYLAND_PROTOCOL_FILE_XDG_SHELL)" $@
