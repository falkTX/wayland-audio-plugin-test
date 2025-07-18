// Wayland audio plugin test
// Copyright (C) 2025 Filipe Coelho <falktx@falktx.com>
// SPDX-License-Identifier: ISC

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include <wayland-egl.h>
#include <EGL/egl.h>

struct gtk_decoration {
    void* gobject;
    void* glib;
    void* gtklib;
    EGLDisplay egl_display;
    struct wl_display* wl_display;
    struct wl_surface* wl_surface;
    struct {
        int x, y;
    } offset;

    /* check if host is running glib mainloop
     * we do this by adding a high priority timeout when creating the gtk window.
     * if our timeout is called before the first gtk_decoration_idle, host has mainloop.
     */
    struct {
        // gtk_decoration_idle count
        int plugin;
        // glib timeout received
        bool glib;
    } idlerecv;

    bool closing;
    bool mainloop;
    int gtkver;
    void* gtkwindow;
};

struct gtk_decoration* gtk_decoration_init(uint32_t width, uint32_t height, bool resizable, const char* title);
void gtk_decoration_idle(struct gtk_decoration*);
void gtk_decoration_destroy(struct gtk_decoration*);
