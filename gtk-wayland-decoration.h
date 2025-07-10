// Wayland audio plugin test
// Copyright (C) 2025 Filipe Coelho <falktx@falktx.com>
// SPDX-License-Identifier: ISC

#pragma once

#include <stdbool.h>
#include <stdint.h>

struct gtk_decoration {
    void* lib;
    // void* glib;
    struct wl_display* wl_display;
    struct wl_surface* wl_surface;
    struct {
        int x, y;
    } offset;
    bool closing;
};

struct gtk_decoration* gtk_decoration_init(uint32_t width, uint32_t height, bool resizable, const char* title);
void gtk_decoration_idle(struct gtk_decoration*);
void gtk_decoration_destroy(struct gtk_decoration*);
