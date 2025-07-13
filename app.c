// Wayland audio plugin test
// Copyright (C) 2025 Filipe Coelho <falktx@falktx.com>
// SPDX-License-Identifier: ISC

// ensure assert does its thing
#undef NDEBUG
#define DEBUG
#include <assert.h>

#include "app.h"
#include "gtk-wayland-decoration.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <wayland-client.h>
#include <GLES2/gl2.h>

#include "proto/xdg-decoration.h"
#include "proto/xdg-shell.h"

// #define TEST_CUSTOM_TITLE_BAR

#define LOG(...)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

// --------------------------------------------------------------------------------------------------------------------

static void wl_registry_global_announce(struct app* const app,
                                        struct wl_registry* const wl_registry,
                                        const uint32_t name,
                                        const char* const interface,
                                        const uint32_t version)
{
    LOG("[%s] %s(%p, %p, %u, \"%s\", %u)\n", app->name, __func__, app, wl_registry, name, interface, version);

    /**/ if (strcmp(interface, wl_compositor_interface.name) == 0)
    {
        assert(app->wl_compositor == NULL);
        app->wl_compositor = wl_registry_bind(app->wl_registry, name, &wl_compositor_interface, 1);
    }
    else if (strcmp(interface, wl_seat_interface.name) == 0)
    {
        assert(app->wl_seat == NULL);
        app->wl_seat = wl_registry_bind(app->wl_registry, name, &wl_seat_interface, 1);
    }
    else if (strcmp(interface, wl_shm_interface.name) == 0)
    {
        assert(app->wl_shm == NULL);
        app->wl_shm = wl_registry_bind(app->wl_registry, name, &wl_shm_interface, 1);
    }
    else if (app->embed)
    {
        /**/ if (strcmp(interface, wl_subcompositor_interface.name) == 0)
        {
            assert(app->wl_subcompositor == NULL);
            app->wl_subcompositor = wl_registry_bind(app->wl_registry, name, &wl_subcompositor_interface, 1);
        }
       #ifndef TEST_CUSTOM_TITLE_BAR
        else if (strcmp(interface, xdg_decoration_manager_interface.name) == 0)
        {
            assert(app->xdg_decoration_manager == NULL);
            // intentionally unused in embed views
            app->supports_decorations = true;
        }
       #endif
    }
    else
    {
       #ifndef TEST_CUSTOM_TITLE_BAR
        /**/ if (strcmp(interface, xdg_decoration_manager_interface.name) == 0)
        {
            assert(app->xdg_decoration_manager == NULL);
            app->xdg_decoration_manager = wl_registry_bind(app->wl_registry, name, &xdg_decoration_manager_interface, 1);
            app->supports_decorations = true;
        }
        else
       #endif
        if (strcmp(interface, xdg_wm_base_interface.name) == 0)
        {
            assert(app->xdg_wm_base == NULL);
            app->xdg_wm_base = wl_registry_bind(app->wl_registry, name, &xdg_wm_base_interface, 1);
        }
    }
}

static void wl_registry_global_remove(struct app* const app,
                                      struct wl_registry* const wl_registry,
                                      const uint32_t name)
{
    LOG("[%s] %s(%p, %p, %u)\n", app->name, __func__, app, wl_registry, name);
}

static const struct wl_registry_listener wl_registry_listener = {
    wl_registry_global_announce,
    wl_registry_global_remove,
};

// --------------------------------------------------------------------------------------------------------------------

static void wl_keyboard_keymap(struct app* const app,
                               struct wl_keyboard* const wl_keyboard,
                               const uint32_t format,
                               const int32_t fd,
                               const uint32_t size)
{
    LOG("[%s] %s(%p, %p, %u, %d, %u)\n", app->name, __func__, app, wl_keyboard, format, fd, size);
}

static void wl_keyboard_enter(struct app* const app,
                              struct wl_keyboard* const wl_keyboard,
                              const uint32_t serial,
                              struct wl_surface* const surface,
                              struct wl_array* const keys)
{
    LOG("[%s] %s(%p, %p, %u, %p, %p)\n", app->name, __func__, app, wl_keyboard, serial, surface, keys);
}

static void wl_keyboard_leave(struct app* const app,
                              struct wl_keyboard* const wl_keyboard,
                              const uint32_t serial,
                              struct wl_surface* const surface)
{
    LOG("[%s] %s(%p, %p, %u, %p)\n", app->name, __func__, app, wl_keyboard, serial, surface);
}

static void wl_keyboard_key(struct app* const app,
                            struct wl_keyboard* const wl_keyboard,
                            const uint32_t serial,
                            const uint32_t time,
                            const uint32_t key,
                            const uint32_t state)
{
    LOG("[%s] %s(%p, %p, %u, %u, %u, %u)\n", app->name, __func__, app, wl_keyboard, serial, time, key, state);
}

static void wl_keyboard_modifiers(struct app* const app,
                                  struct wl_keyboard* const wl_keyboard,
                                  const uint32_t serial,
                                  const uint32_t mods_depressed,
                                  const uint32_t mods_latched,
                                  const uint32_t mods_locked,
                                  const uint32_t group)
{
    LOG("[%s] %s(%p, %p, %u, %u, %u, %u, %u)\n", app->name, __func__, app, wl_keyboard, serial, mods_depressed, mods_latched, mods_locked, group);
}

static void wl_keyboard_repeat_info(struct app* const app,
                                    struct wl_keyboard* const wl_keyboard,
                                    const int32_t rate,
                                    const int32_t delay)
{
    LOG("[%s] %s(%p, %p, %d, %d)\n", app->name, __func__, app, wl_keyboard, rate, delay);
}

static const struct wl_keyboard_listener wl_keyboard_listener = {
    wl_keyboard_keymap,
    wl_keyboard_enter,
    wl_keyboard_leave,
    wl_keyboard_key,
    wl_keyboard_modifiers,
    wl_keyboard_repeat_info,
};

// --------------------------------------------------------------------------------------------------------------------

static void wl_pointer_enter(struct app* const app,
                             struct wl_pointer* const wl_pointer,
                             const uint32_t serial,
                             struct wl_surface* const surface,
                             const wl_fixed_t surface_x,
                             const wl_fixed_t surface_y)
{
    LOG("[%s] %s(%p, %p, %u, %p, %f, %f)\n", app->name, __func__, app, wl_pointer, serial, surface, wl_fixed_to_double(surface_x), wl_fixed_to_double(surface_y));
}

static void wl_pointer_leave(struct app* const app,
                             struct wl_pointer* const wl_pointer,
                             const uint32_t serial,
                             struct wl_surface* const surface)
{
    LOG("[%s] %s(%p, %p, %u, %p)\n", app->name, __func__, app, wl_pointer, serial, surface);
}

static void wl_pointer_motion(struct app* const app,
                              struct wl_pointer* const wl_pointer,
                              const uint32_t time,
                              const wl_fixed_t surface_x,
                              const wl_fixed_t surface_y)
{
    LOG("[%s] %s(%p, %p, %u, %f, %f)\n", app->name, __func__, app, wl_pointer, time, wl_fixed_to_double(surface_x), wl_fixed_to_double(surface_y));
}

static void wl_pointer_button(struct app* const app,
                              struct wl_pointer* const wl_pointer,
                              const uint32_t serial,
                              const uint32_t time,
                              const uint32_t button,
                              const uint32_t state)
{
    LOG("[%s] %s(%p, %p, %u, %u, %u, %u)\n", app->name, __func__, app, wl_pointer, serial, time, button, state);
}

static void wl_pointer_axis(struct app* const app,
                            struct wl_pointer* const wl_pointer,
                            const uint32_t time,
                            const uint32_t axis,
                            const wl_fixed_t value)
{
    LOG("[%s] %s(%p, %p, %u, %u, %f)\n", app->name, __func__, app, wl_pointer, time, axis, wl_fixed_to_double(value));
}

static void wl_pointer_frame(struct app* const app, struct wl_pointer* const wl_pointer)
{
    LOG("[%s] %s(%p, %p)\n", app->name, __func__, app, wl_pointer);
}

static void wl_pointer_axis_source(struct app* const app,
                                   struct wl_pointer* const wl_pointer,
                                   const uint32_t axis_source)
{
    LOG("[%s] %s(%p, %p, %u)\n", app->name, __func__, app, wl_pointer, axis_source);
}

static void wl_pointer_axis_stop(struct app* const app,
                                 struct wl_pointer* const wl_pointer,
                                 const uint32_t time,
                                 const uint32_t axis)
{
    LOG("[%s] %s(%p, %p, %u, %u)\n", app->name, __func__, app, wl_pointer, time, axis);
}

static void wl_pointer_axis_discrete(struct app* const app,
                                     struct wl_pointer* const wl_pointer,
                                     const uint32_t axis,
                                     const int32_t discrete)
{
    LOG("[%s] %s(%p, %p, %u, %d)\n", app->name, __func__, app, wl_pointer, axis, discrete);
}

static void wl_pointer_axis_value120(struct app* const app,
                                     struct wl_pointer* const wl_pointer,
                                     const uint32_t axis,
                                     const int32_t value120)
{
    LOG("[%s] %s(%p, %p, %u, %d)\n", app->name, __func__, app, wl_pointer, axis, value120);
}

#ifdef WL_POINTER_AXIS_RELATIVE_DIRECTION_SINCE_VERSION
static void wl_pointer_axis_relative_direction(struct app* const app,
                                               struct wl_pointer* const wl_pointer,
                                               const uint32_t axis,
                                               const uint32_t direction)
{
    LOG("[%s] %s(%p, %p, %u, %u)\n", app->name, __func__, app, wl_pointer, axis, direction);
}
#endif

struct wl_pointer_listener wl_pointer_listener = {
    wl_pointer_enter,
    wl_pointer_leave,
    wl_pointer_motion,
    wl_pointer_button,
    wl_pointer_axis,
    wl_pointer_frame,
    wl_pointer_axis_source,
    wl_pointer_axis_stop,
    wl_pointer_axis_discrete,
    wl_pointer_axis_value120,
#ifdef WL_POINTER_AXIS_RELATIVE_DIRECTION_SINCE_VERSION
    wl_pointer_axis_relative_direction,
#endif
};

// --------------------------------------------------------------------------------------------------------------------

static void wl_seat_capabilities(struct app* const app, struct wl_seat* const wl_seat, const uint32_t capabilities)
{
    LOG("[%s] %s(%p, %p, %u)\n", app->name, __func__, app, wl_seat, capabilities);

    if (capabilities & WL_SEAT_CAPABILITY_KEYBOARD)
    {
        struct wl_keyboard* const wl_keyboard = wl_seat_get_keyboard(wl_seat);
        wl_keyboard_add_listener(wl_keyboard, &wl_keyboard_listener, app);
    }
    if (capabilities & WL_SEAT_CAPABILITY_POINTER)
    {
        struct wl_pointer* const wl_pointer = wl_seat_get_pointer(wl_seat);
        wl_pointer_add_listener(wl_pointer, &wl_pointer_listener, app);
    }
}

static void wl_seat_name(struct app* const app,
                         struct wl_seat* const wl_seat,
                         const char* const name)
{
    LOG("[%s] %s(%p, %p, \"%s\")\n", app->name, __func__, app, wl_seat, name);
}

static const struct wl_seat_listener wl_seat_listener = {
    wl_seat_capabilities,
    wl_seat_name,
};

// --------------------------------------------------------------------------------------------------------------------

static void wl_surface_enter(struct app* const app,
                             struct wl_surface* const wl_surface,
                             struct wl_output* const wl_output)
{
    LOG("[%s] %s(%p, %p, %p)\n", app->name, __func__, app, wl_surface, wl_output);
}

static void wl_surface_leave(struct app* const app,
                             struct wl_surface* const wl_surface,
                             struct wl_output* const wl_output)
{
    LOG("[%s] %s(%p, %p, %p)\n", app->name, __func__, app, wl_surface, wl_output);
}

#ifdef WL_SURFACE_PREFERRED_BUFFER_SCALE_SINCE_VERSION
static void wl_surface_preferred_buffer_scale(struct app* const app, struct wl_surface* const wl_surface, const int32_t factor)
{
    LOG("[%s] %s(%p, %p, %d)\n", app->name, __func__, app, wl_surface, factor);
}
#endif

#ifdef WL_SURFACE_PREFERRED_BUFFER_TRANSFORM_SINCE_VERSION
static void wl_surface_preferred_buffer_transform(struct app* const app, struct wl_surface* const wl_surface, const uint32_t transform)
{
    LOG("[%s] %s(%p, %p, %u)\n", app->name, __func__, app, wl_surface, transform);
}
#endif

static const struct wl_surface_listener wl_surface_listener = {
    wl_surface_enter,
    wl_surface_leave,
#ifdef WL_SURFACE_PREFERRED_BUFFER_SCALE_SINCE_VERSION
    wl_surface_preferred_buffer_scale,
#endif
#ifdef WL_SURFACE_PREFERRED_BUFFER_TRANSFORM_SINCE_VERSION
    wl_surface_preferred_buffer_transform,
#endif
};

// --------------------------------------------------------------------------------------------------------------------

static void xdg_wm_base_ping(struct app* const app,
                             struct xdg_wm_base* const xdg_wm_base,
                             const uint32_t serial)
{
    LOG("[%s] %s(%p, %p, %u)\n", app->name, __func__, app, xdg_wm_base, serial);
    xdg_wm_base_pong(xdg_wm_base, serial);
}

static const struct xdg_wm_base_listener xdg_wm_base_listener = {
    xdg_wm_base_ping,
};

// --------------------------------------------------------------------------------------------------------------------

static void xdg_surface_configure(struct app* const app,
                                  struct xdg_surface* const xdg_surface,
                                  const uint32_t serial)
{
    LOG("[%s] %s(%p, %p, %u)\n", app->name, __func__, app, xdg_surface, serial);

    if (app->width == 0 || app->height == 0)
        return;

    int err;
    EGLDisplay old_display;
    EGLContext old_context;
    EGLSurface old_surface;
    if (app->reuse_egl_display)
    {
        old_display = eglGetCurrentDisplay();
        old_context = eglGetCurrentContext();
        old_surface = eglGetCurrentSurface(EGL_DRAW);
    }
    else
    {
        old_display = app->egl.display;
        old_context = NULL;
        old_surface = NULL;
    }

    err = eglMakeCurrent(app->egl.display, app->egl.surface, app->egl.surface, app->egl.context);
    assert(err == EGL_TRUE);

    glViewport(0, 0, app->width, app->height);

    glClearColor(app->r, app->g, app->b, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    // glFlush();
    err = eglSwapBuffers(app->egl.display, app->egl.surface);
    assert(err == EGL_TRUE);

    err = eglMakeCurrent(old_display, old_surface, old_surface, old_context);
    assert(err == EGL_TRUE);

    xdg_surface_ack_configure(xdg_surface, serial);
    wl_surface_commit(app->wl_surface);
}

static const struct xdg_surface_listener xdg_surface_listener = {
    xdg_surface_configure,
};

// --------------------------------------------------------------------------------------------------------------------

static void xdg_toplevel_configure(struct app* const app,
                                   struct xdg_toplevel* const xdg_toplevel,
                                   int32_t width,
                                   int32_t height,
                                   struct wl_array* const states)
{
    LOG("[%s] %s(%p, %p, %d, %d, %p)\n", app->name, __func__, app, xdg_toplevel, width, height, states);

    if (width == 0 || height == 0)
        return;
    if (app->width == width && app->height == height)
        return;

    app->width = width;
    app->height = height;
    wl_egl_window_resize(app->egl.window, width, height, 0, 0);
}

static void xdg_toplevel_close(struct app* const app, struct xdg_toplevel* const xdg_toplevel)
{
    LOG("[%s] %s(%p, %p)\n", app->name, __func__, app, xdg_toplevel);

    app->closing = true;
}

static void xdg_toplevel_configure_bounds(struct app* const app,
                                          struct xdg_toplevel* const xdg_toplevel,
                                          const int32_t width,
                                          const int32_t height)
{
    LOG("[%s] %s(%p, %p, %d, %d)\n", app->name, __func__, app, xdg_toplevel, width, height);
}

static void xdg_toplevel_wm_capabilities(struct app* const app,
                                         struct xdg_toplevel* const xdg_toplevel,
                                         struct wl_array* const capabilities)
{
    LOG("[%s] %s(%p, %p, %p)\n", app->name, __func__, app, xdg_toplevel, capabilities);
}

static const struct xdg_toplevel_listener xdg_toplevel_listener = {
    xdg_toplevel_configure,
    xdg_toplevel_close,
    xdg_toplevel_configure_bounds,
    xdg_toplevel_wm_capabilities,
};

// --------------------------------------------------------------------------------------------------------------------

#pragma GCC diagnostic pop

// --------------------------------------------------------------------------------------------------------------------

struct app* app_init(struct wl_display* const wl_display,
                     struct wl_surface* parent_wl_surface,
                     const EGLDisplay egl_display,
                     const char* const title,
                     const float scaleFactor)
{
    int err;
    struct app* const app = calloc(1, sizeof(struct app));
    assert(app != NULL);

    app->r = app->g = 1.f;
    app->name = "testing";

    if (wl_display == NULL || parent_wl_surface == NULL)
    {
        app->wl_display = wl_display_connect(NULL);
        assert(app->wl_display != NULL);
    }
    else
    {
        app->embed = true;
        app->wl_display = wl_display;
    }

    app->wl_registry = wl_display_get_registry(app->wl_display);
    assert(app->wl_registry != NULL);

    err = wl_registry_add_listener(app->wl_registry, &wl_registry_listener, app);
    assert(err == 0);

    // first block-wait to receive global announce events
    err = wl_display_roundtrip(app->wl_display);
    assert(err > 0);

    // these must all be valid now
    assert(app->wl_compositor != NULL);
    assert(app->wl_seat != NULL);
    assert(app->wl_shm != NULL);

    if (app->embed)
    {
        assert(app->wl_subcompositor != NULL);
        assert(app->xdg_decoration_manager == NULL);
        assert(app->xdg_wm_base == NULL);
    }
    else
    {
        assert(app->wl_subcompositor == NULL);
        assert(app->xdg_wm_base != NULL);

        // NOTE mutter and weston do not implement this one
        if (app->xdg_decoration_manager == NULL)
        {
            // we really, really do not want to do our custom decorations that will look out of place everywhere..
            // so here we restart the whole thing by and embed on a gtk app that does it for us.

            wl_compositor_destroy(app->wl_compositor);
            app->wl_compositor = NULL;
            wl_seat_destroy(app->wl_seat);
            app->wl_seat = NULL;
            wl_shm_destroy(app->wl_shm);
            app->wl_shm = NULL;
            xdg_wm_base_destroy(app->xdg_wm_base);
            app->xdg_wm_base = NULL;
            wl_registry_destroy(app->wl_registry);
            app->wl_registry = NULL;
            wl_display_disconnect(app->wl_display);
            app->wl_display = NULL;

            app->gtkdecor = gtk_decoration_init(INITIAL_WIDTH * scaleFactor,
                                                INITIAL_HEIGHT * scaleFactor,
                                                false,
                                                title);
            assert(app->gtkdecor != NULL);

            app->embed = true;
            app->wl_display = app->gtkdecor->wl_display;

            // repeat the same steps as above, but now embed
            app->wl_registry = wl_display_get_registry(app->wl_display);
            assert(app->wl_registry != NULL);

            err = wl_registry_add_listener(app->wl_registry, &wl_registry_listener, app);
            assert(err == 0);

            // first block-wait to receive global announce events
            err = wl_display_roundtrip(app->wl_display);
            assert(err > 0);

            // these must all be valid now
            assert(app->wl_compositor != NULL);
            assert(app->wl_seat != NULL);
            assert(app->wl_shm != NULL);
            assert(app->wl_subcompositor != NULL);
            assert(app->xdg_decoration_manager == NULL);
            assert(app->xdg_wm_base == NULL);

            // parent surface comes from gtk decoration
            parent_wl_surface = app->gtkdecor->wl_surface;
        }
    }

    err = wl_seat_add_listener(app->wl_seat, &wl_seat_listener, app);
    assert(err == 0);

    app->wl_surface = wl_compositor_create_surface(app->wl_compositor);
    assert(app->wl_surface != NULL);

    err = wl_surface_add_listener(app->wl_surface, &wl_surface_listener, app);
    assert(err == 0);

    if (app->embed)
    {
        app->wl_subsurface = wl_subcompositor_get_subsurface(app->wl_subcompositor,
                                                             app->wl_surface, parent_wl_surface);
        assert(app->wl_subsurface != NULL);

        if (app->gtkdecor != NULL)
            wl_subsurface_set_position(app->wl_subsurface, app->gtkdecor->offset.x, app->gtkdecor->offset.y);
    }
    else
    {
        err = xdg_wm_base_add_listener(app->xdg_wm_base, &xdg_wm_base_listener, app);
        assert(err == 0);

        app->xdg_surface = xdg_wm_base_get_xdg_surface(app->xdg_wm_base, app->wl_surface);
        assert(app->xdg_surface != NULL);

        err = xdg_surface_add_listener(app->xdg_surface, &xdg_surface_listener, app);
        assert(err == 0);

        app->xdg_toplevel = xdg_surface_get_toplevel(app->xdg_surface);
        assert(app->xdg_toplevel != NULL);

        err = xdg_toplevel_add_listener(app->xdg_toplevel, &xdg_toplevel_listener, app);
        assert(err == 0);

        if (app->xdg_decoration_manager != NULL)
        {
            app->xdg_toplevel_decoration = xdg_decoration_manager_get_toplevel_decoration(app->xdg_decoration_manager, app->xdg_toplevel);
            assert(app->xdg_toplevel_decoration != NULL);

            xdg_toplevel_decoration_set_mode(app->xdg_toplevel_decoration, XDG_TOPLEVEL_DECORATION_MODE_SERVER_SIDE);
            xdg_toplevel_set_title(app->xdg_toplevel, title);
        }
    }

    // commit so we can receive configure event
    wl_surface_commit(app->wl_surface);
    wl_display_roundtrip(app->wl_display);

    EGLDisplay old_display;
    EGLContext old_context;
    EGLSurface old_surface;
    if (egl_display != EGL_NO_DISPLAY)
    {
        app->egl.display = egl_display;
        app->reuse_egl_display = true;
        old_display = eglGetCurrentDisplay();
        old_context = eglGetCurrentContext();
        old_surface = eglGetCurrentSurface(EGL_DRAW);
    }
    else
    {
        app->egl.display = eglGetDisplay(app->wl_display);
        assert(app->egl.display != EGL_NO_DISPLAY);

        err = eglInitialize(app->egl.display, NULL, NULL);
        assert(err == EGL_TRUE);

        old_display = app->egl.display;
        old_context = NULL;
        old_surface = NULL;
    }

    app->egl.window = wl_egl_window_create(app->wl_surface,
                                           INITIAL_WIDTH * scaleFactor,
                                           INITIAL_HEIGHT * scaleFactor);
    assert(app->egl.window != EGL_NO_SURFACE);

    EGLint num_configs;
    err = eglGetConfigs(app->egl.display, NULL, 0, &num_configs);
    assert(err == EGL_TRUE);

    const EGLint config_attrs[] = {
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_NONE
    };
    err = eglChooseConfig(app->egl.display, config_attrs, &app->egl.config, 1, &num_configs);
    assert(err == EGL_TRUE);
    // assert(app->egl.config != EGL_NO_CONFIG);

    app->egl.surface = eglCreateWindowSurface(app->egl.display, app->egl.config, app->egl.window, NULL);
    assert(app->egl.surface != EGL_NO_SURFACE);

    const EGLint context_attrs[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };
    app->egl.context = eglCreateContext(app->egl.display, app->egl.config, EGL_NO_CONTEXT, context_attrs);
    assert(app->egl.context != EGL_NO_CONTEXT);

    err = eglMakeCurrent(app->egl.display, app->egl.surface, app->egl.surface, app->egl.context);
    assert(err == EGL_TRUE);

    // initial drawing
    glClearColor(1.0, 1.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    // glFlush();
    if (!app->reuse_egl_display)
    {
        err = eglSwapInterval(app->egl.display, 0);
        assert(err == EGL_TRUE);
    }
    err = eglSwapBuffers(app->egl.display, app->egl.surface);
    assert(err == EGL_TRUE);

    err = eglMakeCurrent(old_display, old_surface, old_surface, old_context);
    assert(err == EGL_TRUE);

    return app;
}

void app_run(struct app* const app)
{
    assert(!app->embed);

    if (app->gtkdecor != NULL)
    {
        while (!app->gtkdecor->closing && !app->closing)
            gtk_decoration_idle(app->gtkdecor);
    }
    else
    {
        while (wl_display_dispatch(app->wl_display) != -1 && !app->closing);
    }
}

void app_idle(struct app* const app)
{
    if (app->gtkdecor != NULL)
    {
        if (app->gtkdecor->closing)
            app->closing = true;
        else
            gtk_decoration_idle(app->gtkdecor);
    }
    // TESTING do we need this?
    else if (!app->embed)
    {
        wl_display_dispatch_pending(app->wl_display);
        wl_display_roundtrip(app->wl_display);
    }
}

void app_resize(struct app* app, int width, int height)
{
    app->width = width;
    app->height = height;
    wl_egl_window_resize(app->egl.window, width, height, 0, 0);

    if (app->xdg_toplevel != NULL)
        xdg_toplevel_resize(app->xdg_toplevel, app->wl_seat, 0, 0);
}

void app_update(struct app* app)
{
    int err;
    EGLDisplay old_display;
    EGLContext old_context;
    EGLSurface old_surface;
    if (app->reuse_egl_display)
    {
        old_display = eglGetCurrentDisplay();
        old_context = eglGetCurrentContext();
        old_surface = eglGetCurrentSurface(EGL_DRAW);
    }
    else
    {
        old_display = app->egl.display;
        old_context = NULL;
        old_surface = NULL;
    }

    err = eglMakeCurrent(app->egl.display, app->egl.surface, app->egl.surface, app->egl.context);
    assert(err == EGL_TRUE);

    glClearColor(app->r, app->g, app->b, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    // if (!app->reuse_egl_display)
    {
        // FIXME
        err = eglSwapBuffers(app->egl.display, app->egl.surface);
        assert(err == EGL_TRUE);
    }

    wl_surface_commit(app->wl_surface);

    err = eglMakeCurrent(old_display, old_surface, old_surface, old_context);
    assert(err == EGL_TRUE);
}

void app_destroy(struct app* const app)
{
    eglDestroyContext(app->egl.display, app->egl.context);
    eglDestroySurface(app->egl.display, app->egl.surface);
    wl_egl_window_destroy(app->egl.window);

    if (!app->reuse_egl_display)
        eglTerminate(app->egl.display);

    if (app->embed)
    {
        assert(app->xdg_toplevel_decoration == NULL);
        assert(app->xdg_toplevel == NULL);
        assert(app->xdg_surface == NULL);
        wl_subsurface_destroy(app->wl_subsurface);
    }
    else
    {
        assert(app->wl_subsurface == NULL);
        if (app->xdg_toplevel_decoration != NULL)
            xdg_toplevel_decoration_destroy(app->xdg_toplevel_decoration);
        xdg_toplevel_destroy(app->xdg_toplevel);
        xdg_surface_destroy(app->xdg_surface);
    }
    wl_surface_destroy(app->wl_surface);

    wl_compositor_destroy(app->wl_compositor);
    wl_seat_destroy(app->wl_seat);
    wl_shm_destroy(app->wl_shm);

    if (app->embed)
    {
        wl_subcompositor_destroy(app->wl_subcompositor);
    }
    else
    {
        if (app->xdg_decoration_manager != NULL)
            xdg_decoration_manager_destroy(app->xdg_decoration_manager);
        xdg_wm_base_destroy(app->xdg_wm_base);
    }

    wl_registry_destroy(app->wl_registry);

    if (app->gtkdecor != NULL)
        gtk_decoration_destroy(app->gtkdecor);
    else if (!app->embed)
        wl_display_disconnect(app->wl_display);

    free(app);
}

// --------------------------------------------------------------------------------------------------------------------

static void wayland_compositor_test(bool* const supports_decorations,
                                    struct wl_registry* const wl_registry,
                                    const uint32_t name,
                                    const char* const interface,
                                    const uint32_t version)
{
    if (strcmp(interface, xdg_decoration_manager_interface.name) == 0)
        *supports_decorations = wl_registry != NULL && name != 0 && version != 0;
}

bool wayland_compositor_supports_decorations()
{
    static bool first_run = true;
    static bool supports_decorations = false;

    if (first_run)
    {
        first_run = false;
        int err;

        const struct wl_registry_listener wl_registry_listener = {
            wayland_compositor_test,
            NULL,
        };

        struct wl_display* const wl_display = wl_display_connect(NULL);
        assert(wl_display != NULL);

        struct wl_registry* const wl_registry = wl_display_get_registry(wl_display);
        assert(wl_registry != NULL);

        err = wl_registry_add_listener(wl_registry, &wl_registry_listener, &supports_decorations);
        assert(err == 0);

        // first block-wait to receive global announce events
        err = wl_display_roundtrip(wl_display);
        assert(err > 0);

        wl_registry_destroy(wl_registry);
        wl_display_disconnect(wl_display);
    }

    return supports_decorations;
}

// --------------------------------------------------------------------------------------------------------------------
