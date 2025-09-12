// Wayland audio plugin test
// Copyright (C) 2025 Filipe Coelho <falktx@falktx.com>
// SPDX-License-Identifier: ISC

// ensure assert does its thing
#undef NDEBUG
#define DEBUG
#include <assert.h>

#include <unistd.h>

#include <gtk/gtk.h>
#include <gdk/gdkwayland.h>

#include "proto/xdg-decoration.h"
#include "proto/xdg-foreign.h"
#include "proto/xdg-shell.h"

#include "app.h"

#define PADDING 20

// --------------------------------------------------------------------------------------------------------------------

static char* exported_handle = NULL;

static void app_exported(GdkWindow* const window, struct xdg_exported* const xdg_exported, const char* const handle)
{
    printf("app_exported %p\n", window);
    gdk_wayland_window_set_transient_for_exported(window, (char*)handle);

    (void)window;
    (void)handle;
    (void)xdg_exported;
}

static const struct zxdg_exported_v2_listener xdg_exported_listener = {
    app_exported,
};

static void window_exported_cb(GdkWindow* const window, const char* const handle, struct app* const app)
{
    exported_handle = strdup(handle);
    assert(exported_handle);

    // struct xdg_imported* const imported = xdg_importer_import_toplevel(app->xdg_importer, handle);
    // assert(imported);
    // zxdg_imported_v2_set_parent_of(imported, app->wl_surface);

    struct xdg_exported* exported = xdg_exporter_export_toplevel(app->xdg_exporter, app->wl_surface);
    assert(exported);

    printf("export %p\n", window);
    int err = zxdg_exported_v2_add_listener(exported, &xdg_exported_listener, window);
    assert(err == 0);

    // struct zxdg_exporter_v2 *zxdg_exporter_v2, struct wl_surface *surface
    // zxdg_exporter_v2_export_toplevel();
    (void)app;
    (void)app_exported;
    (void)window;
}

static void widget_realize_cb(GtkWidget* const widget, struct app* const app)
{
    GdkWindow* const gwin = gtk_widget_get_window(widget);
    assert(gwin != NULL);

    gdk_wayland_window_set_use_custom_surface(gwin);

    struct wl_surface* const wl_surface = gdk_wayland_window_get_wl_surface(gwin);
    assert(wl_surface != NULL);

    gdk_wayland_window_export_handle(gwin, G_CALLBACK(window_exported_cb), app, NULL);
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
    bool ok = gtk_init_check(NULL, NULL);
    assert(ok);

    GtkWindow* const window = gtk_window_new(GTK_WINDOW_POPUP);
    assert(window != NULL);

    GtkWidget* const header = gtk_header_bar_new();
    assert(header != NULL);

    gtk_window_set_decorated(window, false);
    gtk_window_set_titlebar(window, header);

    GtkWidget* button = gtk_button_new_with_label("Red");
    gtk_widget_set_size_request(button, 160, 130);
    // gtk_widget_set_parent(button, window);
    g_object_set(button, "margin", 5, NULL);

    // p_main_box = gtk_vbox_new(TRUE, 0);
    gtk_container_add(GTK_CONTAINER(window), button);

    gtk_window_set_default_size(window, INITIAL_WIDTH, INITIAL_HEIGHT);
    // gtk_window_move(window, PADDING, PADDING);

    // g_signal_connect(window, "realize", G_CALLBACK(widget_realize_cb), app);
    (void)widget_realize_cb;

    gtk_widget_realize(window);

    GdkWindow* const gwin = gtk_widget_get_window(window);
    assert(gwin != NULL);

    GdkDisplay* const gdisplay = gdk_window_get_display(gwin);
    assert(gdisplay != NULL);

    struct wl_display* const wl_display = gdk_wayland_display_get_wl_display(gdisplay);
    assert(wl_display != NULL);

    struct wl_surface* const wl_surface = gdk_wayland_window_get_wl_surface(gwin);
    assert(wl_surface != NULL);

    gdk_wayland_window_export_handle(gwin, G_CALLBACK(window_exported_cb), app, NULL);

    gtk_widget_show_all(window);

    // non-block idle testing
    while (!app->closing)
    {
        // idle app
        app_idle(app);

        // idle glib
        g_main_context_iteration(NULL, false);

        // wl_display_dispatch_pending(wl_display);
        // wl_display_roundtrip(wl_display);

        usleep(16666);
    }

    gtk_widget_destroy(window);
    app_destroy(app);
    return 0;
}

// --------------------------------------------------------------------------------------------------------------------
