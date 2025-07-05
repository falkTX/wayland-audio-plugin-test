// Wayland audio plugin test
// Copyright (C) 2025 Filipe Coelho <falktx@falktx.com>
// SPDX-License-Identifier: ISC

#include <QApplication>
#include <QMainWindow>
#include <QTimerEvent>

#include <dlfcn.h>

#include "app.h"

class HostWindow : public QMainWindow
{
    struct app* plugin = nullptr;
    int timerId = -1;

public:
    HostWindow() : QMainWindow()
    {
        resize(INITIAL_WIDTH + 40, INITIAL_HEIGHT + 40);
    }

    ~HostWindow() override
    {
        killTimer(timerId);
        app_destroy(plugin);
    }

    void showAndSetupPlugin()
    {
        // force Qt to create underlying wayland window and surface
        winId();

        // fetch lower level details
        QWindow* const window = windowHandle();
        struct wl_display* wl_display = nullptr;
        struct wl_surface* wl_surface = nullptr;

        // WARNING using private APIs!
        if (void* const libwl = dlopen("libQt6WaylandClient.so.6", RTLD_NOW))
        {
            // Qt6.4 private API
            using QPlatformNativeInterface_platformNativeInterface_t = QPlatformNativeInterface* (*)();
            using QPlatformNativeInterface__nativeResourceForIntegration_t = void* (*)(QPlatformNativeInterface*, const QByteArray&);
            using QPlatformNativeInterface__nativeResourceForWindow_t = void* (*)(QPlatformNativeInterface*, const QByteArray&, QWindow*);

            auto platformNativeInterfaceFn = reinterpret_cast<QPlatformNativeInterface_platformNativeInterface_t>(dlsym(nullptr, "_ZN15QGuiApplication23platformNativeInterfaceEv"));
            auto nativeResourceForIntegrationFn = reinterpret_cast<QPlatformNativeInterface__nativeResourceForIntegration_t>(dlsym(libwl, "_ZN15QtWaylandClient23QWaylandNativeInterface28nativeResourceForIntegrationERK10QByteArray"));
            auto nativeResourceForWindowFn = reinterpret_cast<QPlatformNativeInterface__nativeResourceForWindow_t>(dlsym(libwl, "_ZN15QtWaylandClient23QWaylandNativeInterface23nativeResourceForWindowERK10QByteArrayP7QWindow"));

            if (platformNativeInterfaceFn != nullptr && nativeResourceForIntegrationFn != nullptr && nativeResourceForWindowFn != nullptr)
            {
                QPlatformNativeInterface* const nativeInterface = platformNativeInterfaceFn();
                wl_display = static_cast<struct wl_display*>(nativeResourceForIntegrationFn(nativeInterface, "wl_display"));
                wl_surface = static_cast<struct wl_surface*>(nativeResourceForWindowFn(nativeInterface, "surface", window));
            }

            // TODO support new Qt6.5 interface
            // see https://blog.david-redondo.de/qt/kde/2022/12/09/wayland-native-interface.html

            dlclose(libwl);
        }

        // TODO fetch scale factor
        plugin = app_init(wl_display, wl_surface, "testing with qt", 1.0f);
        timerId = startTimer(16);

        // move plugin surface to center
        if (plugin->wl_subsurface != NULL)
            wl_subsurface_set_position(plugin->wl_subsurface, 20, 20);

        show();
    }

protected:
    void timerEvent(QTimerEvent* const ev) override
    {
        if (ev->timerId() == timerId)
            app_idle(plugin);

        QMainWindow::timerEvent(ev);
    }
};

int main(int argc, char* argv[])
{
    // init host stuff first
    QApplication app(argc, argv);

    qWarning() << app.platformName();
    if (app.platformName() != "wayland")
        return 1;

    HostWindow win;

    win.showAndSetupPlugin();
    return app.exec();
}
