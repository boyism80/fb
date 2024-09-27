#include <stdio.h>
#include <iostream>
#include <iomanip>
#include "resource.h"
#include <fb/socket.h>
#include <fb/gateway/context.h>
// #include <fb/leak.h>
#include <fb/console.h>
#include <fb/config.h>
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
        c.box(c.width()-1, height);

        auto header = "The Kingdom of the wind [GATEWAY]";
        c.cursor((c.width()-1 - strlen(header)) / 2, 2).put(header);

        auto github = "https://github.com/boyism80/fb";
        c.cursor(c.width()-1 - strlen(github) - 3, 4).put(github);

        auto madeby = "made by cshyeon";
        c.cursor(c.width()-1 - strlen(madeby) - 3, 5).put(madeby);

        c.cursor(0, height + 1);

        // Execute acceptor
        boost::asio::io_context io_context;
        auto& config = fb::config::get();
        auto context = std::make_unique<fb::gateway::context>(io_context, config["port"].asInt());

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