// ensure assert does its thing
#undef NDEBUG
#define DEBUG
#include <assert.h>

#include <unistd.h>

#include "app.h"

// --------------------------------------------------------------------------------------------------------------------

int main()
{
    struct app* const app = app_init(NULL, NULL, "testing", 1.0f);
    assert(app != NULL);

#if 1
    // block-wait
    app_run(app);
#else
    // non-block idle testing
    while (!app->closing)
    {
        app_idle(app);
        usleep(16666);
    }
#endif

    app_destroy(app);
    return 0;
}
