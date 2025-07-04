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
    struct app* app;
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
            uridMap = (const LV2_URID_Map*)features[i]->data;
        else if (strcmp(features[i]->URI, LV2_UI__parent) == 0)
            parent = features[i]->data;
    }

    // host must either provide options (for ui-title) or parent
    if (options == NULL && parent == NULL)
    {
        fprintf(stderr, "Options feature missing (needed for show-interface), cannot continue!\n");
        return NULL;
    }

    if (uridMap == NULL)
    {
        fprintf(stderr, "URID Map feature missing, cannot continue!\n");
        return NULL;
    }

    float scaleFactor = 1.0f;
    const char* title = NULL;
    if (options != NULL)
    {
        const LV2_URID uridAtomFloat = uridMap->map(uridMap->handle, LV2_ATOM__Float);
        const LV2_URID uridAtomString = uridMap->map(uridMap->handle, LV2_ATOM__String);
        const LV2_URID uridScaleFactor = uridMap->map(uridMap->handle, LV2_UI__scaleFactor);
        const LV2_URID uridWindowTitle = uridMap->map(uridMap->handle, LV2_UI__windowTitle);

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
        }
    }

    struct ui* const ui = calloc(1, sizeof(struct ui));
    assert(ui != NULL);

    struct wl_surface* const wl_surface = parent;

    ui->app = app_init(wl_surface, title, scaleFactor);
    assert(ui->app != NULL);

    *widget = app_get_subsurface(ui->app);

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

    app_destroy(ui->app);
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
        ui->r = value;
        break;
    case 5:
        ui->g = value;
        break;
    case 6:
        ui->b = value;
        break;
    }
}

static int lv2ui_idle(LV2UI_Handle handle)
{
    struct ui* const ui = handle;

    return ui->app->closing ? 1 : 0;
}

static int lv2ui_show(LV2UI_Handle handle)
{
    struct ui* const ui = handle;

    return 0;

    // TODO
    (void)ui;
}

static int lv2ui_hide(LV2UI_Handle handle)
{
    struct ui* const ui = handle;

    return 0;

    // TODO
    (void)ui;
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
