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

// TODO find these values dynamically
#define GTK3_SHADOW_SIZE 45
#define GTK3_TITLEBAR_HEIGHT 50

// --------------------------------------------------------------------------------------------------------------------

static void gtk_ui_destroy(void* const handle, struct app* const plugin)
{
    app_destroy(plugin);

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

int main(int argc, char* argv[])
{
    gtk_init(&argc, &argv);
    GtkWindow* const window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    assert(window != NULL);

    gtk_window_resize(window, INITIAL_WIDTH + 40, INITIAL_HEIGHT + 40);
    gtk_window_set_resizable(window, true);
    gtk_window_set_title(window, "gtk3-host");

    gtk_widget_realize(window);

    GdkWindow* const gwin = gtk_widget_get_window(window);
    assert(gwin != NULL);

    GdkDisplay* const gdisplay = gdk_window_get_display(gwin);
    assert(gdisplay != NULL);

    struct wl_display* const wl_display = gdk_wayland_display_get_wl_display(gdisplay);
    assert(wl_display != NULL);

    struct wl_surface* const wl_surface = gdk_wayland_window_get_wl_surface(gwin);
    assert(wl_surface != NULL);

    struct app* const plugin = app_init(wl_display, wl_surface, "plugin", 1.0f);
    assert(plugin != NULL);
    plugin->name = "plugin";

    // move plugin surface to center
    if (plugin->wl_subsurface != NULL)
    {
        int x = 20;
        int y = 20;
        if (!plugin->supports_decorations)
        {
            x += GTK3_SHADOW_SIZE;
            y += GTK3_SHADOW_SIZE + GTK3_TITLEBAR_HEIGHT;
        }
        wl_subsurface_set_position(plugin->wl_subsurface, x, y);
    }

    g_signal_connect_data(window, "destroy", gtk_ui_destroy, plugin, NULL, 0);
    g_timeout_add(30, gtk_ui_timeout, plugin);

    gtk_widget_show_all(window);
    gtk_main();
    return 0;
}

// --------------------------------------------------------------------------------------------------------------------
