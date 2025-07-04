#include <QApplication>
#include <QMainWindow>
#include <QTimerEvent>
#include <QWindow>
// #include <qpa/qplatformwindow.h>
// #include <QtWaylandClient>
// #include <QtWaylandClient/private/qwaylandwindow_p.h>

#include <dlfcn.h>

#include "app.h"

class QPlatformWindow
{
public:
    QPlatformWindow();
    ~QPlatformWindow();
};

class QWaylandWindow : public QObject, public QPlatformWindow
{
    Q_OBJECT
public:
    QWaylandWindow(QWindow *window);
    ~QWaylandWindow();
};

class HostWindow : public QMainWindow
{
    struct app* app = nullptr;
    int timerId = -1;

public:
    HostWindow() : QMainWindow()
    {
        resize(INITIAL_WIDTH, INITIAL_HEIGHT);

        // create all wayland platform details
        winId();
    }

    ~HostWindow() override
    {
        killTimer(timerId);
        app_destroy(app);
    }

    void showAndSetupPlugin()
    {
        // fetch lower level details
        QWindow* const wh = windowHandle();
        QPlatformWindow* const h = wh->handle();
        QWaylandWindow* const qqh = (QWaylandWindow*)h;

        // fetch private API symbol
        typedef wl_surface* (*QtWaylandClient_QWaylandWindow_wlSurface_t)(QWaylandWindow*);
        typedef void (*QtWaylandClient_QWaylandWindow_initWindow_t)(void*);
        void* const libshell = dlopen("libQt6WaylandClient.so.6", RTLD_NOW/*|RTLD_GLOBAL*/);

        auto wlSurfaceFn = reinterpret_cast<QtWaylandClient_QWaylandWindow_wlSurface_t>(dlsym(libshell, "_ZN15QtWaylandClient14QWaylandWindow9wlSurfaceEv"));
        fprintf(stderr, "------------- wlSurfaceFn %p\n", wlSurfaceFn);

        auto initWindow = reinterpret_cast<QtWaylandClient_QWaylandWindow_initWindow_t>(dlsym(libshell, "_ZN15QtWaylandClient14QWaylandWindow10initWindowEv"));
        fprintf(stderr, "------------- initWindow %p\n", initWindow);

        initWindow((void*)h);
        fprintf(stderr, "------------- initWindow %p | ok\n", initWindow);

        wl_surface* const wl_surface = wlSurfaceFn(qqh);
        fprintf(stderr, "------------- wlSurfaceFn %p -> %p\n", wlSurfaceFn, wl_surface);

        // auto x = wh->winId();
        // auto waywin = static_cast<QtWaylandClient::QWaylandWindow *>();

        // TODO fetch surface
        // TODO fetch scale factor
        app = app_init(nullptr, "testing with qt", 1.0f);
        timerId = startTimer(16);

        show();
    }

protected:
    void timerEvent(QTimerEvent* const ev) override
    {
        if (ev->timerId() == timerId)
            app_idle(app);

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
