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

// TODO find these values dynamically
#define GTK4_SHADOW_SIZE 23
#define GTK4_TITLEBAR_HEIGHT 50

static void gtk_ui_destroy(void* const handle, struct gtk_decoration* const gtkdecor)
{
    gtkdecor->closing = true;

    // unused
    (void)handle;
}

struct gtk_decoration* gtk_decoration_init(const uint32_t width,
                                           const uint32_t height,
                                           const bool resizable,
                                           const char* const title)
{
    struct gtk_decoration* const gtkdecor = calloc(1, sizeof(struct gtk_decoration));
    assert(gtkdecor != NULL);

    // TODO check if already running under gtk or Qt before anything else

    // gtkdecor->glib = dlopen("libglib-2.0.so.0", RTLD_NOW|RTLD_GLOBAL);
    // assert(gtkdecor->glib != NULL);

    gtkdecor->lib = dlopen("libgtk-4.so.1", RTLD_NOW|RTLD_GLOBAL);
    assert(gtkdecor->lib != NULL);

    void* (*g_signal_connect_data)(void*, const char*, void*, void*, void*, int) = dlsym(NULL, "g_signal_connect_data");
    assert(g_signal_connect_data != NULL);

    void (*gtk_init)(void) = dlsym(NULL, "gtk_init");
    assert(gtk_init != NULL);

    struct GtkWindow* (*gtk_window_new)(void) = dlsym(NULL, "gtk_window_new");
    assert(gtk_window_new != NULL);

    void (*gtk_window_set_decorated)(void*, int) = dlsym(NULL, "gtk_window_set_decorated");
    assert(gtk_window_set_decorated != NULL);

    void (*gtk_window_set_default_size)(void*, uint32_t, uint32_t) = dlsym(NULL, "gtk_window_set_default_size");
    assert(gtk_window_set_default_size != NULL);

    void (*gtk_window_set_resizable)(void*, int) = dlsym(NULL, "gtk_window_set_resizable");
    assert(gtk_window_set_resizable != NULL);

    void (*gtk_window_set_title)(void*, const char*) = dlsym(NULL, "gtk_window_set_title");
    assert(gtk_window_set_title != NULL);

    void (*gtk_widget_realize)(void*) = dlsym(NULL, "gtk_widget_realize");
    assert(gtk_widget_realize != NULL);

    void* (*gtk_window_get_titlebar)(void*) = dlsym(NULL, "gtk_window_get_titlebar");
    assert(gtk_window_get_titlebar != NULL);

    void (*gtk_window_present)(void*) = dlsym(NULL, "gtk_window_present");
    assert(gtk_window_present != NULL);

    void* (*gtk_widget_get_display)(void*) = dlsym(NULL, "gtk_widget_get_display");
    assert(gtk_widget_get_display != NULL);

    void* (*gtk_widget_get_native)(void*) = dlsym(NULL, "gtk_widget_get_native");
    assert(gtk_widget_get_native != NULL);

    void* (*gtk_native_get_surface)(void*) = dlsym(NULL, "gtk_native_get_surface");
    assert(gtk_native_get_surface != NULL);

    void* (*gdk_wayland_display_get_wl_display)(void*) = dlsym(NULL, "gdk_wayland_display_get_wl_display");
    assert(gdk_wayland_display_get_wl_display != NULL);

    void* (*gdk_wayland_surface_get_wl_surface)(void*) = dlsym(NULL, "gdk_wayland_surface_get_wl_surface");
    assert(gdk_wayland_surface_get_wl_surface != NULL);

    gtk_init();

    struct GtkWindow* const window = gtk_window_new();
    assert(window != NULL);

    gtk_window_set_decorated(window, true);
    gtk_window_set_default_size(window, width, height + GTK4_TITLEBAR_HEIGHT);
    gtk_window_set_resizable(window, resizable);
    gtk_window_set_title(window, title);

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

    // TODO find these values dynamically
    gtkdecor->offset.x = GTK4_SHADOW_SIZE;
    gtkdecor->offset.y = GTK4_TITLEBAR_HEIGHT + GTK4_SHADOW_SIZE;

    // void* titlebar = gtk_window_get_titlebar(window);
    // assert(titlebar != NULL);

    return gtkdecor;
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
