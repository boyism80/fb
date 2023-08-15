#include "resource.h"
#include <fb/internal/context.h>
#include <fb/internal/data_set.h>
#include <fb/core/leak.h>
#include <fb/core/console.h>
#include <fb/core/config.h>

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

        auto& config = fb::config::get(env);

        auto height = 8;
        c.box(0, 0, c.width()-1, height);

        auto header = "The Kingdom of the wind [INTERNAL]";
        c.cursor((c.width()-1 - strlen(header)) / 2, 2).put(header);

        auto github = "https://github.com/boyism80/fb";
        c.cursor(c.width()-1 - strlen(github) - 3, 4).put(github);

        auto madeby = "made by cshyeon";
        c.cursor(c.width()-1 - strlen(madeby) - 3, 5).put(madeby);

        auto pivot = height + 1;
        auto stack = 0;
        fb::internal::table::hosts.load
        (
            config["table"]["host"].asString(), 
            [&] (const auto& name, auto percentage)
            {
                c.cursor(0, pivot)
                 .puts(" * [%0.2lf%%] 호스트 정보를 읽었습니다. (%s)", percentage, name.c_str());
            },
            [&] (const auto& name, const auto& error)
            {
                c.cursor(0, pivot + (++stack))
                 .puts("    - %s (%s)", error.c_str(), name.c_str());
            },
            [&] (uint32_t count)
            {
                c.cursor(0, pivot)
                 .puts(" * [100%%] 총 %d개의 호스트 정보를 읽었습니다.", count);
            }
        );

        // Execute acceptor
        boost::asio::io_context io_context;
        auto context = std::make_unique<fb::internal::context>
        (
            io_context, 
            config["port"].asInt(), 
            std::chrono::seconds(config["delay"].asInt())
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
        boost::asio::thread_pool boost_thread_pool(count);
        for(int i = 0; i < count; i++)
        {
            post(boost_thread_pool, [&io_context] { io_context.run(); });
        }
        boost_thread_pool.join();
    }
    catch(std::exception& e)
    {
        c.puts(e.what());
    }

    // Clean up
    return 0;
}