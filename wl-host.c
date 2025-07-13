// Wayland audio plugin test
// Copyright (C) 2025 Filipe Coelho <falktx@falktx.com>
// SPDX-License-Identifier: ISC

// ensure assert does its thing
#undef NDEBUG
#define DEBUG
#include <assert.h>

#include <unistd.h>

#include "app.h"

// --------------------------------------------------------------------------------------------------------------------

int main()
{
    struct app* const app = app_init(NULL, NULL, NULL, "host", 2.0f);
    assert(app != NULL);

    // bigger app is 2x size and grey color
    app->r = app->g = app->b = 0.5f;
    app->name = "host";
    app_update(app);

    struct app* const plugin = app_init(app->wl_display, app->wl_surface, app->egl.display, "plugin", 1.0f);
    assert(plugin != NULL);
    plugin->name = "plugin";

    // move plugin surface to center
    wl_subsurface_set_position(plugin->wl_subsurface, INITIAL_WIDTH / 2, INITIAL_HEIGHT / 2);

    // these do nothing
    // wl_subsurface_place_below(plugin->wl_subsurface, app->wl_surface);
    // app_update(plugin);

    // non-block idle testing
    while (!app->closing)
    {
        app_idle(app);
        app_idle(plugin);
        usleep(16666);
    }

    app_destroy(plugin);
    app_destroy(app);
    return 0;
}
