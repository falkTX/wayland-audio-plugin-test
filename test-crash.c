// Wayland audio plugin test
// Copyright (C) 2025 Filipe Coelho <falktx@falktx.com>
// SPDX-License-Identifier: ISC

// ensure assert does its thing
#undef NDEBUG
#define DEBUG
#include <assert.h>

#include <dlfcn.h>
#include <stddef.h>

#include <lv2/ui/ui.h>

void test()
{
    void* lib = dlopen("./wayland-audio-plugin-test.lv2/plugin.so", RTLD_NOW|RTLD_NODELETE);
    assert(lib != NULL);

    const LV2UI_DescriptorFunction lv2ui_descriptor_fn = dlsym(lib, "lv2ui_descriptor");
    assert(lv2ui_descriptor_fn != NULL);

    const LV2UI_Descriptor* const lv2ui_descriptor = lv2ui_descriptor_fn(0);
    assert(lv2ui_descriptor != NULL);

    const LV2_Feature* const features[] = { NULL };

    LV2UI_Widget widget;
    const LV2UI_Handle uihandle = lv2ui_descriptor->instantiate(lv2ui_descriptor,
                                                                "https://github.com/falkTX/wayland-audio-plugin-test#ui",
                                                                NULL,
                                                                NULL,
                                                                NULL,
                                                                &widget,
                                                                features);
    assert(uihandle != NULL);

    const LV2UI_Idle_Interface* const uiidle = lv2ui_descriptor->extension_data(LV2_UI__idleInterface);
    assert(uiidle != NULL);

    const LV2UI_Show_Interface* const uishow = lv2ui_descriptor->extension_data(LV2_UI__showInterface);
    assert(uishow != NULL);

    uishow->show(uihandle);
    uiidle->idle(uihandle);
    uishow->hide(uihandle);

    lv2ui_descriptor->cleanup(uihandle);
}

int main()
{
    test();
    test();
    return 0;
}
