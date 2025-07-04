// ensure assert does its thing
#undef NDEBUG
#define DEBUG
#include <assert.h>

#include "app.h"

// --------------------------------------------------------------------------------------------------------------------

int main()
{
    struct app* const app = app_init(NULL, "testing", 1.0f);
    assert(app != NULL);

    app_run(app);
    app_destroy(app);
    return 0;
}
