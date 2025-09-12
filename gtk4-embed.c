// Wayland audio plugin test
// Copyright (C) 2025 Filipe Coelho <falktx@falktx.com>
// SPDX-License-Identifier: ISC

// ensure assert does its thing
#undef NDEBUG
#define DEBUG
#include <assert.h>

#include <unistd.h>

#include <gtk/gtk.h>
#include <gdk/wayland/gdkwayland.h>

#include "proto/xdg-decoration.h"
#include "proto/xdg-foreign.h"
#include "proto/xdg-shell.h"

#include "app.h"

#define PADDING 20

// --------------------------------------------------------------------------------------------------------------------

static void app_exported(struct app* const app, struct xdg_exported* const xdg_exported, const char* const handle)
{
    gdk_wayland_toplevel_set_transient_for_exported(app->extra, handle);

    (void)xdg_exported;
}

static const struct zxdg_exported_v2_listener xdg_exported_listener = {
    app_exported,
};

static void window_exported_cb(GdkToplevel* const toplevel, const char* const handle, struct app* const app)
{
    struct xdg_exported* exported = xdg_exporter_export_toplevel(app->xdg_exporter, app->wl_surface);
    assert(exported);

    int err = zxdg_exported_v2_add_listener(exported, &xdg_exported_listener, app);
    assert(err == 0);

    (void)app;
    (void)handle;
    (void)toplevel;
}

// --------------------------------------------------------------------------------------------------------------------

int main()
{
    // create host
    struct app* const app = app_init(NULL, NULL, EGL_NO_DISPLAY, "host", 2.0f);
    assert(app != NULL);

    // host app is 2x size and grey color
    app->r = app->g = app->b = 0.5f;
    app->name = "host";
    app_update(app);

    // create gtk plugin
    gdk_set_allowed_backends("wayland");
    bool ok = gtk_init_check();
    assert(ok);

    GtkWindow* const window = gtk_window_new();
    assert(window != NULL);

    gtk_window_set_decorated(window, false);
    gtk_window_set_default_size(window, INITIAL_WIDTH, INITIAL_HEIGHT);

    gtk_widget_realize(window);

    GdkDisplay* const gdisplay = gtk_widget_get_display(window);
    assert(gdisplay != NULL);

    GdkDisplay* const gnative = gtk_widget_get_native(window);
    assert(gnative != NULL);

    GdkSurface* const gsurface = gtk_native_get_surface(gnative);
    assert(gsurface != NULL);

    struct wl_display* const wl_display = gdk_wayland_display_get_wl_display(gdisplay);
    assert(wl_display != NULL);

    struct wl_surface* const wl_surface = gdk_wayland_surface_get_wl_surface(gsurface);
    assert(wl_surface != NULL);

    app->extra = gsurface;
    gdk_wayland_toplevel_export_handle(gsurface, G_CALLBACK(window_exported_cb), app, NULL);

    gtk_window_present(window);

    // non-block idle testing
    while (!app->closing)
    {
        // idle app
        app_idle(app);

        // idle glib
        g_main_context_iteration(NULL, false);

        usleep(16666);
    }

    gtk_window_destroy(window);
    app_destroy(app);
    return 0;
}

// --------------------------------------------------------------------------------------------------------------------
