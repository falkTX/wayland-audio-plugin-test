// Wayland audio plugin test
// Copyright (C) 2025 Filipe Coelho <falktx@falktx.com>
// SPDX-License-Identifier: ISC

// ensure assert does its thing
#undef NDEBUG
#define DEBUG
#include <assert.h>

#include "gtk-wayland-decoration.h"

#include <dlfcn.h>
#include <stdlib.h>
#include <stdio.h>

// TODO find these values dynamically
#define GTK4_SHADOW_SIZE 23
#define GTK4_TITLEBAR_HEIGHT 50

static void gtk_ui_destroy(void* const handle, struct gtk_decoration* const gtkdecor)
{
    gtkdecor->closing = true;

    // unused
    (void)handle;
}

struct gtk_decoration* gtk3_decoration_init(const uint32_t width,
                                            const uint32_t height,
                                            const bool resizable,
                                            const char* const title)
{
    struct gtk_decoration* const gtkdecor = calloc(1, sizeof(struct gtk_decoration));
    assert(gtkdecor != NULL);

    gtkdecor->lib = dlopen("libgtk-3.so.0", RTLD_NOW|RTLD_GLOBAL);
    assert(gtkdecor->lib != NULL);

    void* (*g_signal_connect_data)(void*, const char*, void*, void*, void*, int) = dlsym(NULL, "g_signal_connect_data");
    assert(g_signal_connect_data != NULL);

    struct GtkWindow* (*gtk_window_new)(int) = dlsym(NULL, "gtk_window_new");
    assert(gtk_window_new != NULL);

    void (*gtk_window_resize)(void*, int, int) = dlsym(NULL, "gtk_window_resize");
    assert(gtk_window_resize != NULL);

    void (*gtk_window_set_decorated)(void*, int) = dlsym(NULL, "gtk_window_set_decorated");
    assert(gtk_window_set_decorated != NULL);

    void (*gtk_window_set_resizable)(void*, int) = dlsym(NULL, "gtk_window_set_resizable");
    assert(gtk_window_set_resizable != NULL);

    void (*gtk_window_set_title)(void*, const char*) = dlsym(NULL, "gtk_window_set_title");
    assert(gtk_window_set_title != NULL);

    void (*gtk_widget_realize)(void*) = dlsym(NULL, "gtk_widget_realize");
    assert(gtk_widget_realize != NULL);

    void (*gtk_widget_show_all)(void*) = dlsym(NULL, "gtk_widget_show_all");
    assert(gtk_widget_show_all != NULL);

    void* (*gtk_widget_get_window)(void*) = dlsym(NULL, "gtk_widget_get_window");
    assert(gtk_widget_get_window != NULL);

    void* (*gdk_window_get_display)(void*) = dlsym(NULL, "gdk_window_get_display");
    assert(gdk_window_get_display != NULL);

    void* (*gdk_wayland_display_get_wl_display)(void*) = dlsym(NULL, "gdk_wayland_display_get_wl_display");
    assert(gdk_wayland_display_get_wl_display != NULL);

    void* (*gdk_wayland_window_get_wl_surface)(void*) = dlsym(NULL, "gdk_wayland_window_get_wl_surface");
    assert(gdk_wayland_window_get_wl_surface != NULL);

    struct GtkWindow* const window = gtk_window_new(0);
    assert(window != NULL);

    gtk_window_resize(window, width, height);
    gtk_window_set_decorated(window, true);
    gtk_window_set_resizable(window, resizable);
    gtk_window_set_title(window, title);

    gtk_widget_realize(window);

    struct GdkWindow* const gwin = gtk_widget_get_window(window);
    assert(gwin != NULL);

    struct GdkDisplay* const gdisplay = gdk_window_get_display(gwin);
    assert(gdisplay != NULL);

    gtkdecor->wl_display = gdk_wayland_display_get_wl_display(gdisplay);
    assert(gtkdecor->wl_display != NULL);

    gtkdecor->wl_surface = gdk_wayland_window_get_wl_surface(gwin);
    assert(gtkdecor->wl_surface != NULL);

    g_signal_connect_data(window, "destroy", gtk_ui_destroy, gtkdecor, NULL, 0);

    gtk_widget_show_all(window);

    // TODO find these values dynamically
    gtkdecor->offset.x = GTK4_SHADOW_SIZE;
    gtkdecor->offset.y = GTK4_TITLEBAR_HEIGHT + GTK4_SHADOW_SIZE;

    return gtkdecor;
}

struct gtk_decoration* gtk4_decoration_init(const uint32_t width,
                                            const uint32_t height,
                                            const bool resizable,
                                            const char* const title,
                                            const bool init)
{
    struct gtk_decoration* const gtkdecor = calloc(1, sizeof(struct gtk_decoration));
    assert(gtkdecor != NULL);

    // gtkdecor->glib = dlopen("libglib-2.0.so.0", RTLD_NOW|RTLD_GLOBAL);
    // assert(gtkdecor->glib != NULL);

    gtkdecor->lib = dlopen("libgtk-4.so.1", RTLD_NOW|RTLD_GLOBAL) ?: dlopen("libgtk-4.so.0", RTLD_NOW|RTLD_GLOBAL);
    assert(gtkdecor->lib != NULL);

    void* (*g_signal_connect_data)(void*, const char*, void*, void*, void*, int) = dlsym(NULL, "g_signal_connect_data");
    assert(g_signal_connect_data != NULL);

    void* (*gdk_wayland_display_get_wl_display)(void*) = dlsym(NULL, "gdk_wayland_display_get_wl_display");
    assert(gdk_wayland_display_get_wl_display != NULL);

    void* (*gdk_wayland_surface_get_wl_surface)(void*) = dlsym(NULL, "gdk_wayland_surface_get_wl_surface");
    assert(gdk_wayland_surface_get_wl_surface != NULL);

    void* (*gtk_header_bar_new)(void) = dlsym(NULL, "gtk_header_bar_new");
    assert(gtk_header_bar_new != NULL);

    void* (*gtk_native_get_surface)(void*) = dlsym(NULL, "gtk_native_get_surface");
    assert(gtk_native_get_surface != NULL);

    void (*gtk_style_context_get_border)(void*, void*) = dlsym(NULL, "gtk_style_context_get_border");
    assert(gtk_style_context_get_border != NULL);

    void* (*gtk_widget_get_display)(void*) = dlsym(NULL, "gtk_widget_get_display");
    assert(gtk_widget_get_display != NULL);

    void* (*gtk_widget_get_native)(void*) = dlsym(NULL, "gtk_widget_get_native");
    assert(gtk_widget_get_native != NULL);

    void* (*gtk_widget_get_style_context)(void*) = dlsym(NULL, "gtk_widget_get_style_context");
    assert(gtk_widget_get_style_context != NULL);

    void (*gtk_widget_get_preferred_size)(void*, int*, int*) = dlsym(NULL, "gtk_widget_get_preferred_size");
    assert(gtk_widget_get_preferred_size != NULL);

    void (*gtk_widget_realize)(void*) = dlsym(NULL, "gtk_widget_realize");
    assert(gtk_widget_realize != NULL);

    void (*gtk_window_destroy)(void*) = dlsym(NULL, "gtk_window_destroy");
    assert(gtk_window_destroy != NULL);

    void* (*gtk_window_new)(void) = dlsym(NULL, "gtk_window_new");
    assert(gtk_window_new != NULL);

    void (*gtk_window_set_decorated)(void*, int) = dlsym(NULL, "gtk_window_set_decorated");
    assert(gtk_window_set_decorated != NULL);

    void (*gtk_window_set_default_size)(void*, uint32_t, uint32_t) = dlsym(NULL, "gtk_window_set_default_size");
    assert(gtk_window_set_default_size != NULL);

    void (*gtk_window_set_resizable)(void*, int) = dlsym(NULL, "gtk_window_set_resizable");
    assert(gtk_window_set_resizable != NULL);

    void (*gtk_window_set_title)(void*, const char*) = dlsym(NULL, "gtk_window_set_title");
    assert(gtk_window_set_title != NULL);

    void (*gtk_window_set_titlebar)(void*, void*) = dlsym(NULL, "gtk_window_set_titlebar");
    assert(gtk_window_set_titlebar != NULL);

    void (*gtk_window_present)(void*) = dlsym(NULL, "gtk_window_present");
    assert(gtk_window_present != NULL);

    if (init)
    {
        void (*gtk_init)(void) = dlsym(NULL, "gtk_init");
        assert(gtk_init != NULL);

        gtk_init();
    }

    // create a dummy gtk4 window so we can find the offset and header height
    int extrawidth, extraheight;
    {
        struct GtkWindow* const window = gtk_window_new();
        assert(window != NULL);

        struct GtkWidget* const header = gtk_header_bar_new();
        assert(header != NULL);

        struct GtkStyleContext* const style = gtk_widget_get_style_context(header);
        assert(style != NULL);

        // get initial window size
        int size[4];
        gtk_widget_get_preferred_size(window, size, size + 2);

        const int initial_width = size[2];
        const int initial_height = size[3];

        // add title bar
        gtk_window_set_decorated(window, true);
        gtk_window_set_default_size(window, initial_width, initial_height);
        gtk_window_set_title(window, "test");
        gtk_window_set_titlebar(window, header);

        // position offset is (new size - old size) / 2 - borders
        int16_t border[4];
        gtk_style_context_get_border(style, border);
        gtk_widget_get_preferred_size(window, size, size + 2);
        gtkdecor->offset.x = (size[2] - initial_width) / 2 - border[2];
        gtkdecor->offset.y = (size[3] - initial_height) / 2 - border[2] - border[3];

        // also get header bar height, need to account for it during window creation
        gtk_widget_get_preferred_size(header, size, size + 2);
        gtkdecor->offset.y += size[3];
        extrawidth = 0;
        extraheight = size[3];

        gtk_window_destroy(window);
    }

    struct GtkWindow* const window = gtk_window_new();
    assert(window != NULL);

    struct GtkWidget* const header = gtk_header_bar_new();
    assert(header != NULL);

    gtk_window_set_decorated(window, true);
    gtk_window_set_default_size(window, width + extrawidth, height + extraheight);
    gtk_window_set_resizable(window, resizable);
    gtk_window_set_title(window, title);
    gtk_window_set_titlebar(window, header);

    gtk_widget_realize(window);

    void* const gdisplay = gtk_widget_get_display(window);
    assert(gdisplay != NULL);

    void* const gnative = gtk_widget_get_native(window);
    assert(gnative != NULL);

    void* const gsurface = gtk_native_get_surface(gnative);
    assert(gsurface != NULL);

    gtkdecor->wl_display = gdk_wayland_display_get_wl_display(gdisplay);
    assert(gtkdecor->wl_display != NULL);

    gtkdecor->wl_surface = gdk_wayland_surface_get_wl_surface(gsurface);
    assert(gtkdecor->wl_surface != NULL);

    g_signal_connect_data(window, "destroy", gtk_ui_destroy, gtkdecor, NULL, 0);

    gtk_window_present(window);

    return gtkdecor;
}

struct gtk_decoration* gtk_decoration_init(const uint32_t width,
                                           const uint32_t height,
                                           const bool resizable,
                                           const char* const title)
{
    const char* (*gtk_version_check)(int, int, int) = dlsym(NULL, "gtk_version_check");

    if (gtk_version_check != NULL)
    {
        const char* check;

        check = gtk_version_check(3, 0, 0);
        if (check != NULL)
        {
            fprintf(stderr, "detected gtk3 under current process: %s\n", check);
            return gtk3_decoration_init(width, height, resizable, title);
        }

        check = gtk_version_check(4, 0, 0);
        if (check != NULL)
        {
            fprintf(stderr, "detected gtk4 under current process: %s\n", check);
            return gtk4_decoration_init(width, height, resizable, title, false);
        }
    }

    fprintf(stderr, "using gtk4 based decoration as fallback\n");
    return gtk4_decoration_init(width, height, resizable, title, true);
}

void gtk_decoration_idle(struct gtk_decoration* const gtkdecor)
{
    int (*g_main_context_iteration)(void*, int) = dlsym(NULL, "g_main_context_iteration");
    assert(g_main_context_iteration != NULL);

    g_main_context_iteration(NULL, true);

    // unused
    (void)gtkdecor;
}

void gtk_decoration_destroy(struct gtk_decoration* const gtkdecor)
{
    dlclose(gtkdecor->lib);
    // dlclose(gtkdecor->glib);
    free(gtkdecor);
}
