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
#include <string.h>

#define RTLD_FLAGS (RTLD_NOW|RTLD_NODELETE)

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"

static int gtk_idle_check_mainloop(struct gtk_decoration* const gtkdecor)
{
    fprintf(stderr, "[gtk-wayland-decoration] gtk_idle_check_mainloop reached\n");
    gtkdecor->idlerecv.glib = true;
    return 0;
}

static void gtk_ui_destroy(void* const handle, struct gtk_decoration* const gtkdecor)
{
    gtkdecor->closing = true;
    gtkdecor->gtkwindow = NULL;

    // unused
    (void)handle;
}

struct gtk_decoration* gtk3_decoration_init(void* const gobject,
                                            void* const glib,
                                            void* const gtklib,
                                            const uint32_t width,
                                            const uint32_t height,
                                            const bool resizable,
                                            const char* const title,
                                            const bool init)
{
    struct gtk_decoration* const gtkdecor = calloc(1, sizeof(struct gtk_decoration));
    assert(gtkdecor != NULL);

    gtkdecor->gobject = gobject;
    gtkdecor->glib = glib;
    gtkdecor->gtklib = gtklib;

    void* (*g_signal_connect_data)(void*, const char*, void*, void*, void*, int) = dlsym(gobject, "g_signal_connect_data");
    assert(g_signal_connect_data != NULL);

    unsigned (*g_idle_add_full)(int, void*, void*, void*) = dlsym(glib, "g_idle_add_full");
    assert(g_idle_add_full != NULL);

    void* (*gdk_wayland_display_get_wl_display)(void*) = dlsym(gtklib, "gdk_wayland_display_get_wl_display");
    assert(gdk_wayland_display_get_wl_display != NULL);

    void* (*gdk_wayland_window_get_wl_surface)(void*) = dlsym(gtklib, "gdk_wayland_window_get_wl_surface");
    assert(gdk_wayland_window_get_wl_surface != NULL);

    void* (*gdk_window_get_display)(void*) = dlsym(gtklib, "gdk_window_get_display");
    assert(gdk_window_get_display != NULL);

    void* (*gtk_header_bar_new)(void) = dlsym(gtklib, "gtk_header_bar_new");
    assert(gtk_header_bar_new != NULL);

    void (*gtk_header_bar_set_show_close_button)(void*, int) = dlsym(gtklib, "gtk_header_bar_set_show_close_button");
    assert(gtk_header_bar_set_show_close_button != NULL);

    void (*gtk_header_bar_set_title)(void*, const char*) = dlsym(gtklib, "gtk_header_bar_set_title");
    assert(gtk_header_bar_set_title != NULL);

    void (*gtk_init)(int*, char***) = dlsym(gtklib, "gtk_init");
    assert(gtk_init != NULL);

    void (*gtk_widget_destroy)(void*) = dlsym(gtklib, "gtk_widget_destroy");
    assert(gtk_widget_destroy != NULL);

    void (*gtk_widget_get_preferred_size)(void*, int*, int*) = dlsym(gtklib, "gtk_widget_get_preferred_size");
    assert(gtk_widget_get_preferred_size != NULL);

    void* (*gtk_widget_get_window)(void*) = dlsym(gtklib, "gtk_widget_get_window");
    assert(gtk_widget_get_window != NULL);

    void (*gtk_widget_hide)(void*) = dlsym(gtklib, "gtk_widget_hide");
    assert(gtk_widget_hide != NULL);

    void (*gtk_widget_realize)(void*) = dlsym(gtklib, "gtk_widget_realize");
    assert(gtk_widget_realize != NULL);

    void (*gtk_widget_show)(void*) = dlsym(gtklib, "gtk_widget_show");
    assert(gtk_widget_show != NULL);

    void (*gtk_widget_show_all)(void*) = dlsym(gtklib, "gtk_widget_show_all");
    assert(gtk_widget_show_all != NULL);

    struct GtkWindow* (*gtk_window_new)(int) = dlsym(gtklib, "gtk_window_new");
    assert(gtk_window_new != NULL);

    void (*gtk_window_set_decorated)(void*, int) = dlsym(gtklib, "gtk_window_set_decorated");
    assert(gtk_window_set_decorated != NULL);

    void (*gtk_window_set_default_size)(void*, int, int) = dlsym(gtklib, "gtk_window_set_default_size");
    assert(gtk_window_set_default_size != NULL);

    void (*gtk_window_set_resizable)(void*, int) = dlsym(gtklib, "gtk_window_set_resizable");
    assert(gtk_window_set_resizable != NULL);

    void (*gtk_window_set_title)(void*, const char*) = dlsym(gtklib, "gtk_window_set_title");
    assert(gtk_window_set_title != NULL);

    void (*gtk_window_set_titlebar)(void*, void*) = dlsym(gtklib, "gtk_window_set_titlebar");
    assert(gtk_window_set_titlebar != NULL);

    if (init)
    {
        static int argc = 0;
        static char* argv[] = { NULL };
        gtk_init(&argc, &argv);
    }

    // this will check if the host is running the mainloop
    const unsigned idle = g_idle_add_full(300, gtk_idle_check_mainloop, gtkdecor, NULL);
    fprintf(stderr, "[gtk-wayland-decoration] idle %u\n", idle);

    // create a dummy gtk3 window so we can find the offset and header height
    {
        struct GtkWindow* const window = gtk_window_new(0);
        assert(window != NULL);

        struct GtkWidget* const header = gtk_header_bar_new();
        assert(header != NULL);

        // get initial window size
        int size[4];
        gtk_widget_get_preferred_size(window, size, size + 2);

        // add title bar
        gtk_window_set_decorated(window, true);
        gtk_window_set_default_size(window, size[2], size[3]);
        gtk_window_set_title(window, "test");
        gtk_window_set_titlebar(window, header);

        // position offset is (new size - old size) / 2
        // NOTE need to show widget or else we get invalid size
        gtk_widget_show(window);
        gtk_widget_get_preferred_size(window, size, size + 2);
        gtk_widget_hide(window);
        gtkdecor->offset.x = gtkdecor->offset.y = (size[3] - size[2]) / 2;

        // also get header bar height
        gtk_widget_show(header);
        gtk_widget_get_preferred_size(header, size, size + 2);
        gtk_widget_hide(header);
        gtkdecor->offset.y += size[3];

        gtk_widget_destroy(window);
    }

    struct GtkWindow* const window = gtk_window_new(0);
    assert(window != NULL);

    gtk_window_set_decorated(window, true);
    gtk_window_set_default_size(window, width, height);
    gtk_window_set_resizable(window, resizable);

    struct GtkWidget* const header = gtk_header_bar_new();
    assert(header != NULL);

    gtk_header_bar_set_show_close_button(header, true);
    gtk_header_bar_set_title(header, title);
    gtk_window_set_titlebar(window, header);

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

    gtkdecor->gtkver = 3;
    gtkdecor->gtkwindow = window;

    fprintf(stderr, "[gtk-wayland-decoration] DONE %u\n", idle);

    return gtkdecor;
}

struct gtk_decoration* gtk4_decoration_init(void* const gobject,
                                            void* const glib,
                                            void* const gtklib,
                                            const uint32_t width,
                                            const uint32_t height,
                                            const bool resizable,
                                            const char* const title,
                                            const bool init)
{
    struct gtk_decoration* const gtkdecor = calloc(1, sizeof(struct gtk_decoration));
    assert(gtkdecor != NULL);

    gtkdecor->gobject = gobject;
    gtkdecor->glib = glib;
    gtkdecor->gtklib = gtklib;

    void* (*g_signal_connect_data)(void*, const char*, void*, void*, void*, int) = dlsym(gobject, "g_signal_connect_data");
    assert(g_signal_connect_data != NULL);

    unsigned (*g_idle_add_full)(int, void*, void*, void*) = dlsym(glib, "g_idle_add_full");
    assert(g_idle_add_full != NULL);

    void* (*gdk_wayland_display_get_wl_display)(void*) = dlsym(gtklib, "gdk_wayland_display_get_wl_display");
    assert(gdk_wayland_display_get_wl_display != NULL);

    void* (*gdk_wayland_surface_get_wl_surface)(void*) = dlsym(gtklib, "gdk_wayland_surface_get_wl_surface");
    assert(gdk_wayland_surface_get_wl_surface != NULL);

    void* (*gtk_header_bar_new)(void) = dlsym(gtklib, "gtk_header_bar_new");
    assert(gtk_header_bar_new != NULL);

    void (*gtk_init)(void) = dlsym(gtklib, "gtk_init");
    assert(gtk_init != NULL);

    void* (*gtk_native_get_surface)(void*) = dlsym(gtklib, "gtk_native_get_surface");
    assert(gtk_native_get_surface != NULL);

    void (*gtk_style_context_get_border)(void*, void*) = dlsym(gtklib, "gtk_style_context_get_border");
    assert(gtk_style_context_get_border != NULL);

    void* (*gtk_widget_get_display)(void*) = dlsym(gtklib, "gtk_widget_get_display");
    assert(gtk_widget_get_display != NULL);

    void* (*gtk_widget_get_native)(void*) = dlsym(gtklib, "gtk_widget_get_native");
    assert(gtk_widget_get_native != NULL);

    void (*gtk_widget_get_preferred_size)(void*, int*, int*) = dlsym(gtklib, "gtk_widget_get_preferred_size");
    assert(gtk_widget_get_preferred_size != NULL);

    void* (*gtk_widget_get_style_context)(void*) = dlsym(gtklib, "gtk_widget_get_style_context");
    assert(gtk_widget_get_style_context != NULL);

    void (*gtk_widget_realize)(void*) = dlsym(gtklib, "gtk_widget_realize");
    assert(gtk_widget_realize != NULL);

    void (*gtk_window_destroy)(void*) = dlsym(gtklib, "gtk_window_destroy");
    assert(gtk_window_destroy != NULL);

    void* (*gtk_window_new)(void) = dlsym(gtklib, "gtk_window_new");
    assert(gtk_window_new != NULL);

    void (*gtk_window_present)(void*) = dlsym(gtklib, "gtk_window_present");
    assert(gtk_window_present != NULL);

    void (*gtk_window_set_decorated)(void*, int) = dlsym(gtklib, "gtk_window_set_decorated");
    assert(gtk_window_set_decorated != NULL);

    void (*gtk_window_set_default_size)(void*, int, int) = dlsym(gtklib, "gtk_window_set_default_size");
    assert(gtk_window_set_default_size != NULL);

    void (*gtk_window_set_resizable)(void*, int) = dlsym(gtklib, "gtk_window_set_resizable");
    assert(gtk_window_set_resizable != NULL);

    void (*gtk_window_set_title)(void*, const char*) = dlsym(gtklib, "gtk_window_set_title");
    assert(gtk_window_set_title != NULL);

    void (*gtk_window_set_titlebar)(void*, void*) = dlsym(gtklib, "gtk_window_set_titlebar");
    assert(gtk_window_set_titlebar != NULL);

    if (init)
        gtk_init();

    // this will check if the host is running the mainloop
    const unsigned idle = g_idle_add_full(300, gtk_idle_check_mainloop, gtkdecor, NULL);
    fprintf(stderr, "[gtk-wayland-decoration] idle %u\n", idle);

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

    gtkdecor->gtkver = 4;
    gtkdecor->gtkwindow = window;
    return gtkdecor;
}

struct gtk_decoration* gtk_decoration_init(const uint32_t width,
                                           const uint32_t height,
                                           const bool resizable,
                                           const char* const title)
{
    void* const gobject = dlopen("libgobject-2.0.so.0", RTLD_FLAGS);
    assert(gobject != NULL);

    void* const glib = dlopen("libglib-2.0.so.0", RTLD_FLAGS);
    assert(glib != NULL);

    const char* (*g_type_name)(unsigned long) = dlsym(gobject, "g_type_name");
    assert(g_type_name != NULL);

    unsigned long (*g_type_from_name)(const char*) = dlsym(gobject, "g_type_from_name");
    assert(g_type_from_name != NULL);

    int gtkver = 0;
    void* gtklib = NULL;

    // this type is only available for Gtk4
    const unsigned long GdkWaylandToplevel_type = g_type_from_name("GdkWaylandToplevel");
    if (GdkWaylandToplevel_type != 0)
    {
        // make sure everything is ok first!
        const char* const GdkWaylandToplevel_name = g_type_name(GdkWaylandToplevel_type);
        if (GdkWaylandToplevel_name != NULL && strncmp(GdkWaylandToplevel_name, "GdkWaylandToplevel", 19) == 0)
        {
            gtklib = dlopen("libgtk-4.so.1", RTLD_FLAGS) ?:
                     dlopen("libgtk-4.so.0", RTLD_FLAGS) ?:
                     dlopen("libgtk-4.so", RTLD_FLAGS);

            if (gtklib != NULL)
            {
                fprintf(stderr, "[gtk-wayland-decoration] auto-detected gtk4!\n");
                gtkver = 4;
            }
        }
    }

    // check if another kind of Gtk has been loaded, it's likely Gtk3
    const unsigned long GdkEvent_type = g_type_from_name("GdkEvent");
    if (GdkEvent_type != 0)
    {
        // make sure everything is ok first!
        const char* const GdkEvent_name = g_type_name(GdkEvent_type);
        if (GdkEvent_name != NULL && strncmp(GdkEvent_name, "GdkEvent", 9) == 0)
        {
            gtklib = dlopen("libgtk-3.so.0", RTLD_FLAGS) ?:
                     dlopen("libgtk-3.so", RTLD_FLAGS);

            if (gtklib != NULL)
            {
                fprintf(stderr, "[gtk-wayland-decoration] auto-detected gtk3!\n");
                gtkver = 3;
            }
        }
    }

    if (gtkver != 0)
    {
        // auto-detection is a success, so gtk has been initialized before
        switch (gtkver)
        {
        case 3:
            return gtk3_decoration_init(gobject, glib, gtklib, width, height, resizable, title, false);
        case 4:
            return gtk4_decoration_init(gobject, glib, gtklib, width, height, resizable, title, false);
        }
    }

    // auto-detection failed, try gtk4 first
    gtklib = dlopen("libgtk-4.so.1", RTLD_FLAGS) ?:
             dlopen("libgtk-4.so.0", RTLD_FLAGS) ?:
             dlopen("libgtk-4.so", RTLD_FLAGS);

    if (gtklib != NULL)
    {
        fprintf(stderr, "[gtk-wayland-decoration] using gtk4 based decoration as fallback\n");
        return gtk4_decoration_init(gobject, glib, gtklib, width, height, resizable, title, true);
    }

    // try gtk3 last
    gtklib = dlopen("libgtk-3.so.0", RTLD_FLAGS) ?:
             dlopen("libgtk-3.so", RTLD_FLAGS);

    if (gtklib != NULL)
    {
        fprintf(stderr, "[gtk-wayland-decoration] using gtk3 based decoration as fallback\n");
        return gtk3_decoration_init(gobject, glib, gtklib, width, height, resizable, title, true);
    }

    fprintf(stderr, "[gtk-wayland-decoration] failed to load any gtk for fallback window decorations\n");
    return NULL;
}

void gtk_decoration_idle(struct gtk_decoration* const gtkdecor)
{
    static int (*g_main_context_iteration)(void*, int);

    if (g_main_context_iteration == NULL)
    {
        g_main_context_iteration = dlsym(gtkdecor->glib, "g_main_context_iteration");
        assert(g_main_context_iteration != NULL);
    }

    if (gtkdecor->idlerecv.plugin < 2)
    {
        fprintf(stderr, "[gtk-wayland-decoration] gtk_decoration_idle gtkdecor->idlerecv.glib:%d\n", (gtkdecor->idlerecv.glib));

        if (++gtkdecor->idlerecv.plugin == 2)
            gtkdecor->mainloop = !gtkdecor->idlerecv.glib;

        return;
    }

    if (gtkdecor->mainloop)
        g_main_context_iteration(NULL, true);
}

void gtk_decoration_destroy(struct gtk_decoration* const gtkdecor)
{
    if (gtkdecor->gtkwindow != NULL)
    {
        /**/ if (gtkdecor->gtkver == 3)
        {
            void (*gtk_widget_destroy)(void*) = dlsym(gtkdecor->gtklib, "gtk_widget_destroy");
            assert(gtk_widget_destroy != NULL);

            gtk_widget_destroy(gtkdecor->gtkwindow);
        }
        else if (gtkdecor->gtkver == 4)
        {
            void (*gtk_window_destroy)(void*) = dlsym(gtkdecor->gtklib, "gtk_window_destroy");
            assert(gtk_window_destroy != NULL);

            gtk_window_destroy(gtkdecor->gtkwindow);
        }
    }

    // glib and gtk stuff is not meant to be unloaded, this is just cleanup
    // we always use RTLD_NODELETE
    dlclose(gtkdecor->gtklib);
    dlclose(gtkdecor->glib);
    dlclose(gtkdecor->gobject);

    free(gtkdecor);
}
