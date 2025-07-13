# Wayland audio plugin test

This repository contains code used for testing audio plugin UIs under wayland.

It is not meant end-users, only for developers.

Rough description of each source code file:

 - app.c: implementation of the plugin UI (a boring fully-filled yellow rectangle, using OpenGL)
 - app.h: definitions for "plugin" UI functions, so we can call them in different host environments for testing
 - gtk-wayland-decoration.c: create window decorations by dynamically loading gtk3 or gtk4 (auto-detection if already loaded by host)
 - {gtk3,gtk3,qt}-host: a gtk3, gtk4, qt5 or qt6 application that embeds the plugin UI as subsurface
 - main.c: run the plugin UI as "standalone", without a host
 - plugin.c: LV2 plugin format implementation
 - test-crash.c: load, run and unload the LV2 UI a few times for testing
 - wl-host.c: pure wayland based host code that embeds the plugin UI (basically embedding a 2nd instance into itself)
