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
    struct app* const app = app_init(NULL, NULL, EGL_NO_DISPLAY, "testing", 1.0f);
    assert(app != NULL);

#if 0
    // block-wait
    app_run(app);
#else
    // non-block idle testing
    while (!app->closing)
    {
        app_idle(app);
        usleep(16666);

        if ((app->r += 0.01f) > 1.f)
            app->r = 0.f;

        app_update(app);
    }
#endif

    app_destroy(app);
    return 0;
}

// --------------------------------------------------------------------------------------------------------------------
