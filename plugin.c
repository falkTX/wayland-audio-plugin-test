// Wayland audio plugin test
// Copyright (C) 2025 Filipe Coelho <falktx@falktx.com>
// SPDX-License-Identifier: ISC

// ensure assert does its thing
#undef NDEBUG
#define DEBUG
#include <assert.h>

#include <lv2/core/lv2.h>
#include <lv2/atom/atom.h>
#include <lv2/options/options.h>
#include <lv2/ui/ui.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <wayland-egl.h>
#include <EGL/egl.h>

#include "app.h"

// --------------------------------------------------------------------------------------------------------------------

struct plugin {
    union {
        struct {
            const float* ins[2];
            float* outs[2];
            float* ctrl[3];
        };
        void* ports[7];
    };
};

static LV2_Handle lv2_instantiate(const LV2_Descriptor* const descriptor,
                                  const double sampleRate,
                                  const char* const bundlePath,
                                  const LV2_Feature* const* const features)
{
    return calloc(1, sizeof(struct plugin));

    // unused
    (void)descriptor;
    (void)sampleRate;
    (void)bundlePath;
    (void)features;
}

static void lv2_connect_port(const LV2_Handle handle, const uint32_t port, void* const dataLocation)
{
    struct plugin* const plugin = handle;

    plugin->ports[port] = dataLocation;
}

static void lv2_run(const LV2_Handle handle, const uint32_t sampleCount)
{
    struct plugin* const plugin = handle;

    if (plugin->outs[0] != plugin->ins[0])
        memcpy(plugin->outs[0], plugin->ins[0], sizeof(float) * sampleCount);

    if (plugin->outs[1] != plugin->ins[1])
        memcpy(plugin->outs[1], plugin->ins[1], sizeof(float) * sampleCount);
}

static void lv2_cleanup(const LV2_Handle handle)
{
    struct plugin* const plugin = handle;

    free(plugin);
}

LV2_SYMBOL_EXPORT
const LV2_Descriptor* lv2_descriptor(const uint32_t index)
{
    static const LV2_Descriptor descriptor = {
        "https://github.com/falkTX/wayland-audio-plugin-test",
        lv2_instantiate,
        lv2_connect_port,
        NULL,
        lv2_run,
        NULL,
        lv2_cleanup,
        NULL,
    };

    return index == 0 ? &descriptor : NULL;
}

// --------------------------------------------------------------------------------------------------------------------

struct ui {
    // NOTE created during show() if non-embed/top-level
    struct app* app;
    // NOTE only used if non-embed/top-level
    char* title;
    float scaleFactor;
    float r, g, b;
};

static LV2UI_Handle lv2ui_instantiate(const LV2UI_Descriptor* const descriptor,
                                      const char* const uri,
                                      const char* const bundlePath,
                                      const LV2UI_Write_Function writeFunction,
                                      const LV2UI_Controller controller,
                                      LV2UI_Widget* const widget,
                                      const LV2_Feature* const* const features)
{
    const LV2_Options_Option* options = NULL;
    const LV2_URID_Map* uridMap = NULL;
    void* parent = NULL;

    for (int i = 0; features[i] != NULL; ++i)
    {
        /**/ if (strcmp(features[i]->URI, LV2_OPTIONS__options) == 0)
            options = features[i]->data;
        else if (strcmp(features[i]->URI, LV2_URID__map) == 0)
            uridMap = features[i]->data;
        else if (strcmp(features[i]->URI, LV2_UI__parent) == 0)
            parent = features[i]->data;
    }

    float scaleFactor = 1.0f;
    const char* title = "test";
    struct wl_display* wl_display = NULL;

    if (options != NULL && uridMap != NULL)
    {
        const LV2_URID uridAtomFloat = uridMap->map(uridMap->handle, LV2_ATOM__Float);
        const LV2_URID uridAtomString = uridMap->map(uridMap->handle, LV2_ATOM__String);
        const LV2_URID uridScaleFactor = uridMap->map(uridMap->handle, LV2_UI__scaleFactor);
        const LV2_URID uridWindowTitle = uridMap->map(uridMap->handle, LV2_UI__windowTitle);
        const LV2_URID uridWaylandDisplay = uridMap->map(uridMap->handle, "urn:wayland:display");

        for (int i = 0; options[i].key != 0; ++i)
        {
            /**/ if (options[i].key == uridScaleFactor)
            {
                if (options[i].type == uridAtomFloat)
                    scaleFactor = *(const float*)options[i].value;
                else
                    fprintf(stderr, "Host provides UI scale factor but has wrong value type\n");
            }
            else if (options[i].key == uridWindowTitle)
            {
                if (options[i].type == uridAtomString)
                    title = options[i].value;
                else
                    fprintf(stderr, "Host provides windowTitle but has wrong value type\n");
            }
            else if (options[i].key == uridWaylandDisplay)
            {
                wl_display = *(struct wl_display**)options[i].value;
            }

            // TODO check if we can use transient window with xdg_toplevel_set_parent
        }
    }
    else if (parent != NULL)
    {
        parent = NULL;
        fprintf(stderr, "Host provides UI parent but not options or urid-map, which are required\n");
    }

    if (parent != NULL && wl_display == NULL)
    {
        parent = NULL;
        fprintf(stderr, "Host provides UI parent but not wayland display, which is required\n");
    }

    struct ui* const ui = calloc(1, sizeof(struct ui));
    assert(ui != NULL);

    struct wl_surface* const wl_surface = parent;

    if (wl_display != NULL && wl_surface != NULL)
    {
        ui->app = app_init(wl_display, wl_surface, NULL, title, scaleFactor);
        assert(ui->app != NULL);

        *widget = ui->app->wl_subsurface;
    }
    else
    {
        ui->title = strdup(title);
        ui->scaleFactor = scaleFactor;
    }

    return ui;

    // unused
    (void)descriptor;
    (void)uri;
    (void)bundlePath;
    (void)writeFunction;
    (void)controller;
}

static void lv2ui_cleanup(const LV2UI_Handle handle)
{
    struct ui* const ui = handle;

    if (ui->app != NULL)
    {
        struct app* const app = ui->app;
        ui->app = NULL;
        app_destroy(app);
    }

    free(ui->title);
    free(ui);
}

static void lv2ui_port_event(const LV2UI_Handle handle,
                             const uint32_t portIndex,
                             const uint32_t bufferSize,
                             const uint32_t format,
                             const void* const buffer)
{
    struct ui* const ui = handle;

    if (bufferSize != sizeof(float))
        return;
    if (format != 0)
        return;

    const float value = *(const float*)buffer;

    switch (portIndex)
    {
    case 4:
        if (ui->app != NULL)
            ui->app->r = value;
        else
            ui->r = value;
        break;

    case 5:
        if (ui->app != NULL)
            ui->app->g = value;
        else
            ui->g = value;
        break;

    case 6:
        if (ui->app != NULL)
            ui->app->b = value;
        else
            ui->b = value;
        break;
    }

    if (ui->app != NULL)
        app_update(ui->app);
}

static int lv2ui_idle(LV2UI_Handle handle)
{
    struct ui* const ui = handle;

    if (ui->app != NULL)
    {
        app_idle(ui->app);
        return ui->app->closing ? 1 : 0;
    }

    return 1;
}

static int lv2ui_show(LV2UI_Handle handle)
{
    struct ui* const ui = handle;

    assert(ui->app == NULL);

    ui->app = app_init(NULL, NULL, EGL_NO_DISPLAY, ui->title, ui->scaleFactor);
    assert(ui->app != NULL);

    ui->app->r = ui->r;
    ui->app->g = ui->g;
    ui->app->b = ui->b;
    app_update(ui->app);

    return 0;
}

static int lv2ui_hide(LV2UI_Handle handle)
{
    struct ui* const ui = handle;

    assert(ui->app != NULL);

    ui->r = ui->app->r;
    ui->g = ui->app->g;
    ui->b = ui->app->b;

    struct app* const app = ui->app;
    ui->app = NULL;
    app_destroy(app);

    return 0;
}

static const void* lv2ui_extension_data(const char* const uri)
{
    static const LV2UI_Idle_Interface idle = { lv2ui_idle };
    static const LV2UI_Show_Interface show = { lv2ui_show, lv2ui_hide };

    if (strcmp(uri, LV2_UI__idleInterface) == 0)
        return &idle;
    if (strcmp(uri, LV2_UI__showInterface) == 0)
        return &show;

    return NULL;
}

LV2_SYMBOL_EXPORT
const LV2UI_Descriptor* lv2ui_descriptor(const uint32_t index)
{
    static const LV2UI_Descriptor descriptor = {
        "https://github.com/falkTX/wayland-audio-plugin-test#ui",
        lv2ui_instantiate,
        lv2ui_cleanup,
        lv2ui_port_event,
        lv2ui_extension_data,
    };

    return index == 0 ? &descriptor : NULL;
}

// --------------------------------------------------------------------------------------------------------------------
