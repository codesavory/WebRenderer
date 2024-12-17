#include "hello_imgui/hello_imgui.h"
#include "webrenderer.h"

#include <fmt/core.h>

int main(int , char *[])
{
    try
    {        
        WebRenderer renderer;
        renderer.run();
    }
    catch (const std::runtime_error &e)
    {
        fmt::print(stderr, "Caught a fatal error: {}\n", e.what());
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
