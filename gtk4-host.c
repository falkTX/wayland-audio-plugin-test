// Wayland audio plugin test
// Copyright (C) 2025 Filipe Coelho <falktx@falktx.com>
// SPDX-License-Identifier: ISC

// ensure assert does its thing
#undef NDEBUG
#define DEBUG
#include <assert.h>

#include <gtk/gtk.h>
#include <gdk/wayland/gdkwayland.h>

#include "app.h"

// TODO find these values dynamically
#define GTK4_SHADOW_SIZE 23
#define GTK4_TITLEBAR_HEIGHT 50

// --------------------------------------------------------------------------------------------------------------------

static void gtk_ui_destroy(void* const handle, struct app* const plugin)
{
    app_destroy(plugin);

    // unused
    (void)handle;
}

static int gtk_ui_timeout(struct app* const plugin)
{
    app_idle(plugin);
    return 1;
}

int main()
{
    gtk_init();

    GtkWindow* const window = gtk_window_new();
    assert(window != NULL);

    // TODO find if compositor supports decorations
    int extra_height = 0;
    if (getenv("GNOME_SETUP_DISPLAY") != NULL)
        extra_height += GTK4_TITLEBAR_HEIGHT;

    gtk_window_set_decorated(window, true);
    gtk_window_set_default_size(window, INITIAL_WIDTH + 40, INITIAL_HEIGHT + 40 + extra_height);
    gtk_window_set_resizable(window, true);
    gtk_window_set_title(window, "gtk4-host");

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
            x += GTK4_SHADOW_SIZE;
            y += GTK4_SHADOW_SIZE + GTK4_TITLEBAR_HEIGHT;
        }
        wl_subsurface_set_position(plugin->wl_subsurface, x, y);
    }

    g_signal_connect_data(window, "destroy", gtk_ui_destroy, plugin, NULL, 0);
    g_timeout_add(30, gtk_ui_timeout, plugin);

    gtk_window_present(window);

    while (g_list_model_get_n_items(gtk_window_get_toplevels()) > 0)
        g_main_context_iteration(NULL, true);

    return 0;
}

// --------------------------------------------------------------------------------------------------------------------
