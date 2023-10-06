#include "resource.h"
#include <fb/core/db.h>
#include <fb/login/context.h>
#include <fb/core/leak.h>
#include <fb/core/console.h>
#include <fb/core/config.h>
#include <fb/protocol/internal.h>

int main(int argc, const char** argv)
{
    auto& c = fb::console::get();

    try
    {
        //_CrtSetBreakAlloc(165);

#ifdef _WIN32
        ::SetConsoleIcon(IDI_BARAM);
        ::SetConsoleTitle(CONSOLE_TITLE);
#endif

        auto height = 8;
        c.box(0, 0, c.width()-1, height);

        auto header = "The Kingdom of the wind [LOGIN]";
        c.cursor((c.width()-1 - strlen(header)) / 2, 2).put(header);

        auto github = "https://github.com/boyism80/fb";
        c.cursor(c.width()-1 - strlen(github) - 3, 4).put(github);

        auto madeby = "made by cshyeon";
        c.cursor(c.width()-1 - strlen(madeby) - 3, 5).put(madeby);

        c.cursor(0, height + 1);
    
        // Execute acceptor
        auto& config = fb::config::get();

        // Execute acceptor
        boost::asio::io_context io_context;
        auto context = std::make_unique<fb::login::context>
        (
            io_context, 
            config["port"].asInt(), 
            std::chrono::seconds(config["delay"].asInt())
        );

        int count = fb::config::get()["thread"].isNull() ? std::thread::hardware_concurrency() : fb::config::get()["thread"].asInt();
        context->run(count);
        while (context->running())
        {
            std::this_thread::sleep_for(100ms);
        }
    }
    catch(std::exception& e)
    {
        fb::logger::fatal(e.what());
    }

    // Clean up
    return 0;
}