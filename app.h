// Wayland audio plugin test
// Copyright (C) 2025 Filipe Coelho <falktx@falktx.com>
// SPDX-License-Identifier: ISC

#pragma once

#include <stdbool.h>

#include <wayland-egl.h>
#include <EGL/egl.h>

#define INITIAL_WIDTH 300
#define INITIAL_HEIGHT 300

#ifdef __cplusplus
extern "C" {
#endif

// unstable protocols
#define XDG_TOPLEVEL_DECORATION_MODE_SERVER_SIDE ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE
#define xdg_decoration_manager_destroy zxdg_decoration_manager_v1_destroy
#define xdg_decoration_manager_interface zxdg_decoration_manager_v1_interface
#define xdg_decoration_manager_get_toplevel_decoration zxdg_decoration_manager_v1_get_toplevel_decoration
#define xdg_toplevel_decoration zxdg_toplevel_decoration_v1
#define xdg_toplevel_decoration_destroy zxdg_toplevel_decoration_v1_destroy
#define xdg_toplevel_decoration_interface zxdg_toplevel_decoration_v1_interface
#define xdg_toplevel_decoration_set_mode zxdg_toplevel_decoration_v1_set_mode

// --------------------------------------------------------------------------------------------------------------------

struct egl {
    EGLDisplay display;
    EGLConfig config;
    EGLContext context;
    EGLSurface surface;
    struct wl_egl_window* window;
};

struct app {
    struct wl_compositor* wl_compositor;
    struct wl_display* wl_display;
    struct wl_registry* wl_registry;
    struct wl_seat* wl_seat;
    struct wl_shm* wl_shm;
    struct wl_subcompositor* wl_subcompositor;
    struct wl_subsurface* wl_subsurface;
    struct wl_surface* wl_surface;
    struct xdg_decoration_manager* xdg_decoration_manager;
    struct xdg_surface* xdg_surface;
    struct xdg_toplevel* xdg_toplevel;
    struct xdg_toplevel_decoration* xdg_toplevel_decoration;
    struct xdg_wm_base* xdg_wm_base;
    struct egl egl;
    struct gtk_decoration* gtkdecor;
    const char* name;
    bool closing, embed, reuse_egl_display, supports_decorations;
    int32_t width;
    int32_t height;
    float r, g, b;
};

// init everything, allow passing custom initial values
struct app* app_init(struct wl_display* wl_display,
                     struct wl_surface* wl_surface,
                     EGLDisplay egl_display,
                     const char* title,
                     float scaleFactor);

// run loop (blocking)
void app_run(struct app* app);

// run loop once (non-blocking)
void app_idle(struct app* app);

// resize
void app_resize(struct app* app, int width, int height);

// update window color
void app_update(struct app* app);

// destroy app
void app_destroy(struct app* app);

// --------------------------------------------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif
