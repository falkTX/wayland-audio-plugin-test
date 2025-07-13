// Wayland audio plugin test
// Copyright (C) 2025 Filipe Coelho <falktx@falktx.com>
// SPDX-License-Identifier: ISC

#include <QApplication>
#include <QDebug>
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
        QPlatformNativeInterface* const nativeInterface = QGuiApplication::platformNativeInterface();
        struct wl_display* wl_display = nullptr;
        struct wl_surface* wl_surface = nullptr;

        if (window != nullptr && nativeInterface != nullptr)
        {
            // WARNING using private APIs!
#if QT_VERSION >= 0x060000
            if (void* const libwl = dlopen("libQt6WaylandClient.so.6", RTLD_NOW))
#else
            if (void* const libwl = dlopen("libQt5WaylandClient.so.5", RTLD_NOW))
#endif
            {
                using QPlatformNativeInterface__nativeResourceForIntegration_t = void* (*)(QPlatformNativeInterface*, const QByteArray&);
                using QPlatformNativeInterface__nativeResourceForWindow_t = void* (*)(QPlatformNativeInterface*, const QByteArray&, QWindow*);

                auto nativeResourceForIntegrationFn = reinterpret_cast<QPlatformNativeInterface__nativeResourceForIntegration_t>(dlsym(libwl, "_ZN15QtWaylandClient23QWaylandNativeInterface28nativeResourceForIntegrationERK10QByteArray"));
                auto nativeResourceForWindowFn = reinterpret_cast<QPlatformNativeInterface__nativeResourceForWindow_t>(dlsym(libwl, "_ZN15QtWaylandClient23QWaylandNativeInterface23nativeResourceForWindowERK10QByteArrayP7QWindow"));

                if (nativeResourceForIntegrationFn != nullptr && nativeResourceForWindowFn != nullptr)
                {
                    wl_display = static_cast<struct wl_display*>(nativeResourceForIntegrationFn(nativeInterface, "wl_display"));
                    wl_surface = static_cast<struct wl_surface*>(nativeResourceForWindowFn(nativeInterface, "surface", window));
                }

                dlclose(libwl);
            }
        }

        // TODO fetch scale factor
        plugin = app_init(wl_display, wl_surface, EGL_NO_DISPLAY, "testing with qt", 1.0f);
        timerId = startTimer(16);

        // move plugin surface to center
        if (plugin->wl_subsurface != NULL)
        {
            int x = 20;
            int y = 20;
            if (!plugin->supports_decorations)
            {
                // TODO fetch titlebar size
                x += 0;
                y += 0;
            }
            wl_subsurface_set_position(plugin->wl_subsurface, x, y);
        }

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
