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

// #define TEST_CUSTOM_TITLE_BAR

#define PADDING 20

// --------------------------------------------------------------------------------------------------------------------

static void get_gtk_offsets(int* x, int* y, int* width, int* height, double *scale_factor)
{
#if 1 // def TEST_CUSTOM_TITLE_BAR
    GtkWindow* const window = gtk_window_new();
    assert(window != NULL);

    GtkWidget* const header = gtk_header_bar_new();
    assert(header != NULL);

    GtkStyleContext* const style = gtk_widget_get_style_context(header);
    assert(style != NULL);

    // *scale_factor = gtk_widget_get_scale_factor(window);
    // NOTE no direct fractional scaling API available
    *scale_factor = 1;

    // get initial window size
    GtkRequisition _, req;
    gtk_widget_get_preferred_size(window, &_, &req);

    const int initial_width = req.width;
    const int initial_height = req.height;

    // add title bar
    gtk_window_set_decorated(window, true);
    gtk_window_set_default_size(window, initial_width, initial_height);
    gtk_window_set_title(window, "test");
    gtk_window_set_titlebar(window, header);

    // position offset is (new size - old size) / 2 - borders
    GtkBorder border;
    gtk_style_context_get_border(style, &border);
    gtk_widget_get_preferred_size(window, &_, &req);
    *x = (req.width - initial_width) / 2 - border.top;
    *y = (req.height - initial_height) / 2 - border.top - border.bottom;

    // also get header bar height, need to account for it during window creation
    gtk_widget_get_preferred_size(header, &_, &req);
    *y += req.height;
    *width = 0;
    *height = req.height;

    gtk_window_destroy(window);
#else
    *x = *y = *width = *height = 0;
    *scale_factor = 1.0;
#endif
}

// --------------------------------------------------------------------------------------------------------------------

static void gtk_ui_destroy(void* const handle, struct app* const plugin)
{
    plugin->closing = true;

    // unused
    (void)handle;
}

static int gtk_ui_timeout(struct app* const plugin)
{
    // app_idle(plugin);

    if ((plugin->r += 0.01f) > 1.f)
        plugin->r = 0.f;

    app_update(plugin);
    return 1;
}

// --------------------------------------------------------------------------------------------------------------------

int main()
{
    gtk_init();

    struct {
        int x, y, width, height;
    } offsets = { 0 };
    double scale_factor = 1;

    GtkWindow* const window = gtk_window_new();
    assert(window != NULL);

    GtkWindowControls* const wincontrols = gtk_window_controls_new(GTK_PACK_START);
    assert(wincontrols != NULL);

    // bool native = gtk_window_controls_get_use_native_controls(wincontrols);
    // assert(native);

    bool check = gtk_window_get_decorated(window);
    assert(check);

    if (check)
        get_gtk_offsets(&offsets.x, &offsets.y, &offsets.width, &offsets.height, &scale_factor);

    gtk_window_set_decorated(window, true);
    gtk_window_set_default_size(window,
                                (INITIAL_WIDTH + PADDING * 2) * scale_factor + offsets.width,
                                (INITIAL_HEIGHT + PADDING * 2) * scale_factor + offsets.height);
    gtk_window_set_resizable(window, true);
    gtk_window_set_title(window, "gtk4-host");

#ifdef TEST_CUSTOM_TITLE_BAR
    GtkWidget* const header = gtk_header_bar_new();
    assert(header != NULL);

    gtk_window_set_titlebar(window, header);
#endif

    gtk_widget_realize(window);

    GdkDisplay* const gdisplay = gtk_widget_get_display(window);
    assert(gdisplay != NULL);

    GdkDisplay* const gnative = gtk_widget_get_native(window);
    assert(gnative != NULL);

    GdkSurface* const gsurface = gtk_native_get_surface(gnative);
    assert(gsurface != NULL);

    struct wl_display* const wl_display = gdk_wayland_display_get_wl_display(gdisplay);
    assert(wl_display != NULL);

    const EGLDisplay egl_display = gdk_wayland_display_get_egl_display(gdisplay);
    assert(egl_display != EGL_NO_DISPLAY);

    struct wl_surface* const wl_surface = gdk_wayland_surface_get_wl_surface(gsurface);
    assert(wl_surface != NULL);

    struct app* const plugin = app_init(wl_display, wl_surface, egl_display, "plugin", scale_factor);
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

    gtk_window_present(window);

    while (g_list_model_get_n_items(gtk_window_get_toplevels()) > 0 && !plugin->closing)
        g_main_context_iteration(NULL, true);

    app_destroy(plugin);
    return 0;
}

// --------------------------------------------------------------------------------------------------------------------
