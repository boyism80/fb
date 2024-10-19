#include "resource.h"
#include <context.h>
#include <fb/leak.h>
#include <fb/console.h>
#include <fb/config.h>
#include <fb/protocol/flatbuffer/protocol.h>

int main(int argc, const char** argv)
{
    auto& c = fb::console::get();
    
    try
    {
        //_CrtSetBreakAlloc(80);

#ifdef _WIN32
        ::SetConsoleIcon(IDI_BARAM);
        ::SetConsoleTitle(CONSOLE_TITLE);
#endif

        auto height = 9;
        c.box(c.width()-1, height);

        auto header = "The Kingdom of the wind [LOGIN]";
        c.cursor((c.width()-1 - strlen(header)) / 2, 3).render(header);

        auto github = "https://github.com/boyism80/fb";
        c.cursor(c.width()-1 - strlen(github) - 3, 5).render(github);

        auto madeby = "made by cshyeon";
        c.cursor(c.width()-1 - strlen(madeby) - 3, 6).render(madeby);

        c.cursor(0, height + 1);
    
        // Execute acceptor
        auto& config = fb::config::get();

        // Execute acceptor
        boost::asio::io_context io_context;
        auto context = std::make_unique<fb::login::context>(io_context, config["port"].asInt());
        context->run();
    }
    catch(std::exception& e)
    {
        fb::logger::fatal(e.what());
    }

    // Clean up
    return 0;
}