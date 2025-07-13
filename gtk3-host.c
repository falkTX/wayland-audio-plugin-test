// Wayland audio plugin test
// Copyright (C) 2025 Filipe Coelho <falktx@falktx.com>
// SPDX-License-Identifier: ISC

// ensure assert does its thing
#undef NDEBUG
#define DEBUG
#include <assert.h>

#include <gtk/gtk.h>
#include <gdk/gdkwayland.h>

#include "app.h"

// #define TEST_CUSTOM_TITLE_BAR

#define PADDING 20

// --------------------------------------------------------------------------------------------------------------------

static void get_gtk_offsets(int* x, int* y, double *scale_factor)
{
    GtkWindow* const window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    assert(window != NULL);

    GtkWidget* const header = gtk_header_bar_new();
    assert(header != NULL);

    // *scale_factor = gtk_widget_get_scale_factor(window);
    // NOTE no direct fractional scaling API available
    *scale_factor = 1;

    // get initial window size
    GtkRequisition _, req;
    gtk_widget_get_preferred_size(window, &_, &req);

    // add title bar
    gtk_window_set_decorated(window, true);
    gtk_window_set_default_size(window, req.width, req.height);
    gtk_window_set_title(window, "test");
    gtk_window_set_titlebar(window, header);

    // position offset is (new size - old size) / 2
    // NOTE need to show widget or else we get invalid size
    gtk_widget_show(window);
    gtk_widget_get_preferred_size(window, &_, &req);
    gtk_widget_hide(window);
    *x = *y = (req.height - req.width) / 2;

    // also get header bar height, need to account for it during window creation
    gtk_widget_show(header);
    gtk_widget_get_preferred_size(header, &_, &req);
    gtk_widget_hide(header);
    *y += req.height;

    gtk_widget_destroy(window);
}

// --------------------------------------------------------------------------------------------------------------------

static void gtk_ui_destroy(void* const handle, struct app* const plugin)
{
    plugin->closing = true;

    if (gtk_main_level() != 0)
        gtk_main_quit();

    // unused
    (void)handle;
}

static int gtk_ui_timeout(struct app* const plugin)
{
    app_idle(plugin);
    return 1;
}

// --------------------------------------------------------------------------------------------------------------------

int main(int argc, char* argv[])
{
    gtk_init(&argc, &argv);

    struct {
        int x, y;
    } offsets = { 0, 0 };
    double scale_factor = 1.0;

#ifndef TEST_CUSTOM_TITLE_BAR
    if (!wayland_compositor_supports_decorations())
#endif
        get_gtk_offsets(&offsets.x, &offsets.y, &scale_factor);

    GtkWindow* const window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    assert(window != NULL);

    gtk_window_set_decorated(window, true);
    gtk_window_set_default_size(window,
                                (INITIAL_WIDTH + PADDING * 2) * scale_factor,
                                (INITIAL_HEIGHT + PADDING * 2) * scale_factor);
    gtk_window_set_resizable(window, true);

#ifdef TEST_CUSTOM_TITLE_BAR
    GtkWidget* const header = gtk_header_bar_new();
    assert(header != NULL);

    gtk_header_bar_set_show_close_button(header, true);
    gtk_header_bar_set_title(header, "gtk3-host");
    gtk_window_set_titlebar(window, header);
#else
    gtk_window_set_title(window, "gtk3-host");
#endif

    gtk_widget_realize(window);

    GdkWindow* const gwin = gtk_widget_get_window(window);
    assert(gwin != NULL);

    GdkDisplay* const gdisplay = gdk_window_get_display(gwin);
    assert(gdisplay != NULL);

    struct wl_display* const wl_display = gdk_wayland_display_get_wl_display(gdisplay);
    assert(wl_display != NULL);

    struct wl_surface* const wl_surface = gdk_wayland_window_get_wl_surface(gwin);
    assert(wl_surface != NULL);

    struct app* const plugin = app_init(wl_display, wl_surface, NULL, "plugin", scale_factor);
    assert(plugin != NULL);
    plugin->name = "plugin";

    // move plugin surface to center
    if (plugin->wl_subsurface != NULL)
    {
        int x = PADDING * scale_factor;
        int y = PADDING * scale_factor;
        if (!plugin->supports_decorations)
        {
            x += offsets.x;
            y += offsets.y;
        }
        wl_subsurface_set_position(plugin->wl_subsurface, x, y);
    }

    g_signal_connect_data(window, "destroy", gtk_ui_destroy, plugin, NULL, 0);
    g_timeout_add(30, gtk_ui_timeout, plugin);

    gtk_widget_show_all(window);
    gtk_main();

    app_destroy(plugin);
    return 0;
}

// --------------------------------------------------------------------------------------------------------------------
