// ensure assert does its thing
#undef NDEBUG
#define DEBUG
#include <assert.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <wayland-client.h>
#include <wayland-egl.h>
#include <xdg-shell.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>

// --------------------------------------------------------------------------------------------------------------------

struct egl {
    EGLDisplay display;
    EGLConfig config;
    EGLContext context;
    EGLSurface surface;
    struct wl_egl_window* window;
};

struct app {
    struct wl_registry* wl_registry;
    struct wl_compositor* wl_compositor;
    struct wl_seat* wl_seat;
#if 0
    struct wl_shell* wl_shell;
    struct wl_shell_surface* wl_shell_surface;
#endif
    struct wl_shm* wl_shm;
    struct wl_surface* wl_surface;
    struct xdg_wm_base* xdg_wm_base;
    struct xdg_surface* xdg_surface;
    struct xdg_toplevel* xdg_toplevel;
    struct egl egl;
    bool closing;
    int32_t width;
    int32_t height;
};

// --------------------------------------------------------------------------------------------------------------------

static void wl_registry_global_announce(void* const data,
                                        struct wl_registry* const wl_registry,
                                        const uint32_t name,
                                        const char* const interface,
                                        const uint32_t version)
{
    fprintf(stderr, "%s(%p, %p, %u, \"%s\", %u)\n", __func__, data, wl_registry, name, interface, version);

    struct app* const app = data;

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
#if 0
    else if (strcmp(interface, wl_shell_interface.name) == 0)
    {
        assert(app->wl_shell == NULL);
        app->wl_shell = wl_registry_bind(app->wl_registry, name, &wl_shell_interface, 1);
    }
    else if (strcmp(interface, wl_shell_surface_interface.name) == 0)
    {
        assert(app->wl_shell_surface == NULL);
        app->wl_shell_surface = wl_registry_bind(app->wl_registry, name, &wl_shell_surface_interface, 1);
    }
#endif
    else if (strcmp(interface, wl_shm_interface.name) == 0)
    {
        assert(app->wl_shm == NULL);
        app->wl_shm = wl_registry_bind(app->wl_registry, name, &wl_shm_interface, 1);
    }
    else if (strcmp(interface, xdg_wm_base_interface.name) == 0)
    {
        assert(app->xdg_wm_base == NULL);
        app->xdg_wm_base = wl_registry_bind(app->wl_registry, name, &xdg_wm_base_interface, 1);
    }
}

static void wl_registry_global_remove(void* const data,
                                      struct wl_registry* const wl_registry,
                                      const uint32_t name)
{
    fprintf(stderr, "%s(%p, %p, %u)\n", __func__, data, wl_registry, name);
}

static const struct wl_registry_listener wl_registry_listener = {
    wl_registry_global_announce,
    wl_registry_global_remove,
};

// --------------------------------------------------------------------------------------------------------------------

static void wl_keyboard_keymap(void* const data,
                               struct wl_keyboard* const wl_keyboard,
                               const uint32_t format,
                               const int32_t fd,
                               const uint32_t size)
{
    fprintf(stderr, "%s(%p, %p, %u, %d, %u)\n", __func__, data, wl_keyboard, format, fd, size);
}

static void wl_keyboard_enter(void* const data,
                              struct wl_keyboard* const wl_keyboard,
                              const uint32_t serial,
                              struct wl_surface* const surface,
                              struct wl_array* const keys)
{
    fprintf(stderr, "%s(%p, %p, %u, %p, %p)\n", __func__, data, wl_keyboard, serial, surface, keys);
}

static void wl_keyboard_leave(void* const data,
                              struct wl_keyboard* const wl_keyboard,
                              const uint32_t serial,
                              struct wl_surface* const surface)
{
    fprintf(stderr, "%s(%p, %p, %u, %p)\n", __func__, data, wl_keyboard, serial, surface);
}

static void wl_keyboard_key(void* const data,
                            struct wl_keyboard* const wl_keyboard,
                            const uint32_t serial,
                            const uint32_t time,
                            const uint32_t key,
                            const uint32_t state)
{
    fprintf(stderr, "%s(%p, %p, %u, %u, %u, %u)\n", __func__, data, wl_keyboard, serial, time, key, state);
}

static void wl_keyboard_modifiers(void* const data,
                                  struct wl_keyboard* const wl_keyboard,
                                  const uint32_t serial,
                                  const uint32_t mods_depressed,
                                  const uint32_t mods_latched,
                                  const uint32_t mods_locked,
                                  const uint32_t group)
{
    fprintf(stderr, "%s(%p, %p, %u, %u, %u, %u, %u)\n", __func__, data, wl_keyboard, serial, mods_depressed, mods_latched, mods_locked, group);
}

static void wl_keyboard_repeat_info(void* const data,
                                    struct wl_keyboard* const wl_keyboard,
                                    const int32_t rate,
                                    const int32_t delay)
{
    fprintf(stderr, "%s(%p, %p, %d, %d)\n", __func__, data, wl_keyboard, rate, delay);
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

static void wl_pointer_enter(void* const data,
                             struct wl_pointer* const wl_pointer,
                             const uint32_t serial,
                             struct wl_surface* const surface,
                             const wl_fixed_t surface_x,
                             const wl_fixed_t surface_y)
{
    fprintf(stderr, "%s(%p, %p, %u, %p, %f, %f)\n", __func__, data, wl_pointer, serial, surface, wl_fixed_to_double(surface_x), wl_fixed_to_double(surface_y));
}

static void wl_pointer_leave(void* const data,
                             struct wl_pointer* const wl_pointer,
                             const uint32_t serial,
                             struct wl_surface* const surface)
{
    fprintf(stderr, "%s(%p, %p, %u, %p)\n", __func__, data, wl_pointer, serial, surface);
}

static void wl_pointer_motion(void* const data,
                              struct wl_pointer* const wl_pointer,
                              const uint32_t time,
                              const wl_fixed_t surface_x,
                              const wl_fixed_t surface_y)
{
    fprintf(stderr, "%s(%p, %p, %u, %f, %f)\n", __func__, data, wl_pointer, time, wl_fixed_to_double(surface_x), wl_fixed_to_double(surface_y));
}

static void wl_pointer_button(void* const data,
                              struct wl_pointer* const wl_pointer,
                              const uint32_t serial,
                              const uint32_t time,
                              const uint32_t button,
                              const uint32_t state)
{
    fprintf(stderr, "%s(%p, %p, %u, %u, %u, %u)\n", __func__, data, wl_pointer, serial, time, button, state);
}

static void wl_pointer_axis(void* const data,
                            struct wl_pointer* const wl_pointer,
                            const uint32_t time,
                            const uint32_t axis,
                            const wl_fixed_t value)
{
    fprintf(stderr, "%s(%p, %p, %u, %u, %f)\n", __func__, data, wl_pointer, time, axis, wl_fixed_to_double(value));
}

static void wl_pointer_frame(void* const data, struct wl_pointer* const wl_pointer)
{
    fprintf(stderr, "%s(%p, %p)\n", __func__, data, wl_pointer);
}

static void wl_pointer_axis_source(void* const data,
                                   struct wl_pointer* const wl_pointer,
                                   const uint32_t axis_source)
{
    fprintf(stderr, "%s(%p, %p, %u)\n", __func__, data, wl_pointer, axis_source);
}

static void wl_pointer_axis_stop(void* const data,
                                 struct wl_pointer* const wl_pointer,
                                 const uint32_t time,
                                 const uint32_t axis)
{
    fprintf(stderr, "%s(%p, %p, %u, %u)\n", __func__, data, wl_pointer, time, axis);
}

static void wl_pointer_axis_discrete(void* const data,
                                     struct wl_pointer* const wl_pointer,
                                     const uint32_t axis,
                                     const int32_t discrete)
{
    fprintf(stderr, "%s(%p, %p, %u, %d)\n", __func__, data, wl_pointer, axis, discrete);
}

static void wl_pointer_axis_value120(void* const data,
                                     struct wl_pointer* const wl_pointer,
                                     const uint32_t axis,
                                     const int32_t value120)
{
    fprintf(stderr, "%s(%p, %p, %u, %d)\n", __func__, data, wl_pointer, axis, value120);
}

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
};

// --------------------------------------------------------------------------------------------------------------------

static void wl_seat_capabilities(void* const data, struct wl_seat* const wl_seat, const uint32_t capabilities)
{
    fprintf(stderr, "%s(%p, %p, %u)\n", __func__, data, wl_seat, capabilities);

    if (capabilities & WL_SEAT_CAPABILITY_KEYBOARD)
    {
        struct wl_keyboard* const wl_keyboard = wl_seat_get_keyboard(wl_seat);
        wl_keyboard_add_listener(wl_keyboard, &wl_keyboard_listener, data);
    }
    if (capabilities & WL_SEAT_CAPABILITY_POINTER)
    {
        struct wl_pointer* const wl_pointer = wl_seat_get_pointer(wl_seat);
        wl_pointer_add_listener(wl_pointer, &wl_pointer_listener, data);
    }
}

static void wl_seat_name(void* const data,
                         struct wl_seat* const wl_seat,
                         const char* const name)
{
    fprintf(stderr, "%s(%p, %p, \"%s\")\n", __func__, data, wl_seat, name);
}

static const struct wl_seat_listener wl_seat_listener = {
    wl_seat_capabilities,
    wl_seat_name,
};

// --------------------------------------------------------------------------------------------------------------------

#if 0
static void wl_shell_surface_ping(void* const data,
                                  struct wl_shell_surface* const wl_shell_surface,
                                  const uint32_t serial)
{
    fprintf(stderr, "%s(%p, %p, %u)\n", __func__, data, wl_shell_surface, serial);
}

static void wl_shell_surface_configure(void* const data,
                                       struct wl_shell_surface* const wl_shell_surface,
                                       const uint32_t edges,
                                       const int32_t width,
                                       const int32_t height)
{
    fprintf(stderr, "%s(%p, %p, %u, %d, %d)\n", __func__, data, wl_shell_surface, edges, width, height);
}

static void wl_shell_surface_popup_done(void* const data,
                                        struct wl_shell_surface* const wl_shell_surface)
{
    fprintf(stderr, "%s(%p, %p)\n", __func__, data, wl_shell_surface);
}

static const struct wl_shell_surface_listener wl_shell_surface_listener = {
    wl_shell_surface_ping,
    wl_shell_surface_configure,
    wl_shell_surface_popup_done,
};
#endif

// --------------------------------------------------------------------------------------------------------------------

static void wl_surface_enter(void* const data,
                             struct wl_surface* const wl_surface,
                             struct wl_output* const wl_output)
{
    fprintf(stderr, "%s(%p, %p, %p)\n", __func__, data, wl_surface, wl_output);
}

static void wl_surface_leave(void* const data,
                             struct wl_surface* const wl_surface,
                             struct wl_output* const wl_output)
{
    fprintf(stderr, "%s(%p, %p, %p)\n", __func__, data, wl_surface, wl_output);
}

static const struct wl_surface_listener wl_surface_listener = {
    wl_surface_enter,
    wl_surface_leave,
};

// --------------------------------------------------------------------------------------------------------------------

static void xdg_wm_base_ping(void* const data,
                             struct xdg_wm_base* const xdg_wm_base,
                             const uint32_t serial)
{
    fprintf(stderr, "%s(%p, %p, %u)\n", __func__, data, xdg_wm_base, serial);
    xdg_wm_base_pong(xdg_wm_base, serial);
}

static const struct xdg_wm_base_listener xdg_wm_base_listener = {
    xdg_wm_base_ping,
};

// --------------------------------------------------------------------------------------------------------------------

static void xdg_surface_configure(void* const data,
                                  struct xdg_surface* const xdg_surface,
                                  const uint32_t serial)
{
    fprintf(stderr, "%s(%p, %p, %u)\n", __func__, data, xdg_surface, serial);

    int err;
    struct app* const app = data;

    if (app->width == 0 || app->height == 0)
        return;

    err = eglMakeCurrent(app->egl.display, app->egl.surface, app->egl.surface, app->egl.context);
    assert(err == EGL_TRUE);

    glViewport(0, 0, app->width, app->height);

    glClearColor(1.0, 1.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    // glFlush();
    err = eglSwapBuffers(app->egl.display, app->egl.surface);
    assert(err == EGL_TRUE);

    err = eglMakeCurrent(app->egl.display, NULL, NULL, NULL);
    assert(err == EGL_TRUE);

    xdg_surface_ack_configure(xdg_surface, serial);
    wl_surface_commit(app->wl_surface);
}

static const struct xdg_surface_listener xdg_surface_listener = {
    xdg_surface_configure,
};

// --------------------------------------------------------------------------------------------------------------------

static void xdg_toplevel_configure(void* const data,
                                   struct xdg_toplevel* const xdg_toplevel,
                                   int32_t width,
                                   int32_t height,
                                   struct wl_array* const states)
{
    fprintf(stderr, "%s(%p, %p, %d, %d, %p)\n", __func__, data, xdg_toplevel, width, height, states);

    struct app* const app = data;

    if (width == 0 || height == 0)
        return;
    if (app->width == width && app->height == height)
        return;

    app->width = width;
    app->height = height;
    wl_egl_window_resize(app->egl.window, width, height, 0, 0);
}

static void xdg_toplevel_close(void* const data, struct xdg_toplevel* const xdg_toplevel)
{
    fprintf(stderr, "%s(%p, %p)\n", __func__, data, xdg_toplevel);

    struct app* const app = data;
    app->closing = true;
}

static void xdg_toplevel_configure_bounds(void* const data,
                                          struct xdg_toplevel* const xdg_toplevel,
                                          const int32_t width,
                                          const int32_t height)
{
    fprintf(stderr, "%s(%p, %p, %d, %d)\n", __func__, data, xdg_toplevel, width, height);
}

static void xdg_toplevel_wm_capabilities(void* const data,
                                         struct xdg_toplevel* const xdg_toplevel,
                                         struct wl_array* const capabilities)
{
    fprintf(stderr, "%s(%p, %p, %p)\n", __func__, data, xdg_toplevel, capabilities);
}

static const struct xdg_toplevel_listener xdg_toplevel_listener = {
    xdg_toplevel_configure,
    xdg_toplevel_close,
    xdg_toplevel_configure_bounds,
    xdg_toplevel_wm_capabilities,
};

// --------------------------------------------------------------------------------------------------------------------

int main()
{
    int err;
    struct app* const app = calloc(1, sizeof(struct app));

    struct wl_display* const wl_display = wl_display_connect(NULL);
    assert(wl_display != NULL);

    app->wl_registry = wl_display_get_registry(wl_display);
    assert(app->wl_registry != NULL);

    err = wl_registry_add_listener(app->wl_registry, &wl_registry_listener, app);
    assert(err == 0);

    // first block-wait to receive global announce events
    err = wl_display_roundtrip(wl_display);
    assert(err > 0);

    // these must all be valid now
    assert(app->wl_compositor != NULL);
    assert(app->wl_seat != NULL);
#if 0
    assert(app->wl_shell != NULL);
#endif
    assert(app->wl_shm != NULL);
    assert(app->xdg_wm_base != NULL);

    err = wl_seat_add_listener(app->wl_seat, &wl_seat_listener, app);
    assert(err == 0);

    err = xdg_wm_base_add_listener(app->xdg_wm_base, &xdg_wm_base_listener, app);
    assert(err == 0);

    app->wl_surface = wl_compositor_create_surface(app->wl_compositor);
    assert(app->wl_surface != NULL);

    err = wl_surface_add_listener(app->wl_surface, &wl_surface_listener, app);
    assert(err == 0);

#if 0
    app->wl_shell_surface = wl_shell_get_shell_surface(app->wl_shell, app->wl_surface);
    assert(app->wl_shell_surface != NULL);

    err = wl_shell_surface_add_listener(app->wl_shell_surface, &wl_shell_surface_listener, app);
    assert(err == 0);
#endif

    app->xdg_surface = xdg_wm_base_get_xdg_surface(app->xdg_wm_base, app->wl_surface);
    assert(app->xdg_surface != NULL);

    err = xdg_surface_add_listener(app->xdg_surface, &xdg_surface_listener, app);
    assert(err == 0);

    app->xdg_toplevel = xdg_surface_get_toplevel(app->xdg_surface);
    assert(app->xdg_toplevel != NULL);

    err = xdg_toplevel_add_listener(app->xdg_toplevel, &xdg_toplevel_listener, app);
    assert(err == 0);

    xdg_toplevel_set_title(app->xdg_toplevel, "testing");

    // commit so we can receive configure event
    wl_surface_commit(app->wl_surface);
    wl_display_roundtrip(wl_display);

    // (EGLNativeDisplayType)
    app->egl.display = eglGetDisplay(wl_display);
    assert(app->egl.display != EGL_NO_DISPLAY);

    err = eglInitialize(app->egl.display, NULL, NULL);
    assert(err == EGL_TRUE);

    app->egl.window = wl_egl_window_create(app->wl_surface, app->width ?: 300, app->height ?: 300);
    assert(app->egl.window != EGL_NO_SURFACE);

    EGLint num_configs;
    err = eglGetConfigs(app->egl.display, NULL, 0, &num_configs);
    assert(err == EGL_TRUE);

    const EGLint config_attrs[] = {
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

    app->egl.context = eglCreateContext(app->egl.display, app->egl.config, EGL_NO_CONTEXT, NULL);
    assert(app->egl.context != EGL_NO_CONTEXT);

    err = eglMakeCurrent(app->egl.display, app->egl.surface, app->egl.surface, app->egl.context);
    assert(err == EGL_TRUE);

    // initial drawing
    glClearColor(1.0, 1.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    // glFlush();
    err = eglSwapInterval(app->egl.display, 0);
    assert(err == EGL_TRUE);
    err = eglSwapBuffers(app->egl.display, app->egl.surface);
    assert(err == EGL_TRUE);

    // needed?
    wl_surface_commit(app->wl_surface);

    err = eglMakeCurrent(app->egl.display, NULL, NULL, NULL);
    assert(err == EGL_TRUE);

    while (wl_display_dispatch(wl_display) != -1 && !app->closing)
    {
        wl_display_roundtrip(wl_display);
    }

    eglDestroyContext(app->egl.display, app->egl.context);
    eglDestroySurface(app->egl.display, app->egl.surface);
    wl_egl_window_destroy(app->egl.window);
    eglTerminate(app->egl.display);

    xdg_toplevel_destroy(app->xdg_toplevel);
    xdg_surface_destroy(app->xdg_surface);
#if 0
    wl_shell_surface_destroy(app->wl_shell_surface);
#endif
    wl_surface_destroy(app->wl_surface);

    wl_compositor_destroy(app->wl_compositor);
    wl_seat_destroy(app->wl_seat);
#if 0
    wl_shell_destroy(app->wl_shell);
#endif
    wl_shm_destroy(app->wl_shm);
    xdg_wm_base_destroy(app->xdg_wm_base);

    wl_registry_destroy(app->wl_registry);
    wl_display_disconnect(wl_display);
    free(app);
    return 0;
}
