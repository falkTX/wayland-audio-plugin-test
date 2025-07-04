#include <QApplication>
#include <QMainWindow>
#include <QTimerEvent>

#include "app.h"

class HostWindow : public QMainWindow
{
    struct app* app;
    int timerId;

public:
    HostWindow() : QMainWindow()
    {
        resize(INITIAL_WIDTH, INITIAL_HEIGHT);

        // TODO fetch surface
        // TODO fetch scale factor
        app = app_init(nullptr, "testing with qt", 1.0f);
        timerId = startTimer(16);
    }

    ~HostWindow() override
    {
        killTimer(timerId);
        app_destroy(app);
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
    HostWindow win;

    win.show();
    return app.exec();
}
