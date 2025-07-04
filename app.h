#pragma once

#include <stdbool.h>

#include <wayland-egl.h>
#include <EGL/egl.h>

#define INITIAL_WIDTH 300
#define INITIAL_HEIGHT 300

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
#if 0
    struct wl_shell* wl_shell;
    struct wl_shell_surface* wl_shell_surface;
#endif
    struct wl_shm* wl_shm;
    struct wl_subcompositor* wl_subcompositor;
    struct wl_surface* wl_surface;
    struct xdg_wm_base* xdg_wm_base;
    struct xdg_surface* xdg_surface;
    struct xdg_toplevel* xdg_toplevel;
    struct egl egl;
    bool closing;
    int32_t width;
    int32_t height;
    float r, g, b;
};

// init everything, allow passing custom initial values
struct app* app_init(struct wl_surface* wl_surface, const char* title, float scaleFactor);

// create/get subsurface
struct wl_subsurface* app_get_subsurface(struct app* app);

// run loop (blocking)
void app_run(struct app* app);

// run loop once (non-blocking)
void app_idle(struct app* app);

// update window color
void app_update(struct app* app);

// destroy app
void app_destroy(struct app* app);

// --------------------------------------------------------------------------------------------------------------------
