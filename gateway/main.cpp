#include <stdio.h>
#include <iostream>
#include <iomanip>
#include "resource.h"
#include <fb/core/socket.h>
#include <fb/gateway/context.h>
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

        const char* env = std::getenv("KINGDOM_OF_WIND_ENVIRONMENT");
        if(env == nullptr)
            env = "dev";

        auto height = 8;
        c.box(0, 0, c.width()-1, height);

        auto header = "The Kingdom of the wind [GATEWAY]";
        c.cursor((c.width()-1 - strlen(header)) / 2, 2).put(header);

        auto github = "https://github.com/boyism80/fb";
        c.cursor(c.width()-1 - strlen(github) - 3, 4).put(github);

        auto madeby = "made by cshyeon";
        c.cursor(c.width()-1 - strlen(madeby) - 3, 5).put(madeby);

        c.cursor(0, height + 1);

        // Execute acceptor
        boost::asio::io_context io_context;
        auto& config = fb::config::get(env);

        const auto connection = INTERNAL_CONNECTION
        {
            config["internal"]["ip"].asString(),
            (uint16_t)config["internal"]["port"].asInt(),
            [&] (fb::base::socket<>& socket, bool success)
            {
                if(success)
                {
                    socket.send(fb::protocol::internal::request::subscribe(config["id"].asString(), fb::protocol::internal::services::GATEWAY, 0xFF));
                }
                else
                {
                    auto& c = fb::console::get();
                    auto t = std::time(nullptr);
                    auto tm = *std::localtime(&t);

                    std::ostringstream sstream;
                    sstream << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
                    c.puts(" * [ERROR] Failed connect to internal server. (%s)", sstream.str().c_str());
                }
            },
            [&]
            {
                // on disconnected
                auto& c = fb::console::get();
                auto t = std::time(nullptr);
                auto tm = *std::localtime(&t);

                std::ostringstream sstream;
                sstream << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
                c.puts(" * [ERROR] Failed connect to internal server. (%s)", sstream.str().c_str());
            }
        };
        auto context = std::make_unique<fb::gateway::context>
        (
            io_context, 
            config["port"].asInt(), 
            std::chrono::seconds(config["delay"].asInt()),
            connection
        );

        boost::asio::signal_set signal(io_context, SIGINT, SIGTERM);
        signal.async_wait
        (
            [&context](const boost::system::error_code& ec, int signal)
            {
                context.get()->exit();
            }
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
        c.puts(e.what());
    }

    // Clean up
    return 0;
}