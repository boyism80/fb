#include <boost/program_options.hpp>
#include "resource.h"
#include "model/acceptor/acceptor.internal.h"
#include "model/table/table.internal.h"
#include "module/leak.h"
#include "module/console/console.h"
#include "module/config/config.h"

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
        auto desc = boost::program_options::options_description("fb internal");
        desc.add_options()
            ("env,e", boost::program_options::value<std::string>(), "environment");

        boost::program_options::variables_map vmap;
        boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vmap);
        boost::program_options::notify(vmap);

        auto env = vmap.count("env") ? vmap["env"].as<std::string>().c_str() : 
#if defined DEBUG | defined _DEBUG
            "dev";
#else
            nullptr;
#endif
        auto& config = fb::config::get(env);

        auto height = 8;
        c.box(0, 0, c.width()-1, height);

        auto header = "The Kingdom of the wind [INTERNAL]";
        c.cursor((c.width()-1 - strlen(header)) / 2, 2).puts(header);

        auto github = "https://github.com/boyism80/fb";
        c.cursor(c.width()-1 - strlen(github) - 3, 4).puts(github);

        auto madeby = "made by cshyeon";
        c.cursor(c.width()-1 - strlen(madeby) - 3, 5).puts(madeby);

        auto pivot = height + 1;
        auto stack = 0;
        fb::internal::table::hosts.load
        (
            config["table"]["host"].asString(), 
            [&] (const std::string& name, double percentage)
            {
                c.cursor(0, pivot)
                 .puts(" * [%0.2lf%%] 호스트 정보를 읽었습니다. (%s)", percentage, name.c_str());
            },
            [&] (const std::string& name, const std::string& error)
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
        auto acceptor = std::make_unique<fb::internal::acceptor>
        (
            io_context, 
            config["port"].asInt(), 
            config["delay"].asInt()
        );

        io_context.run();
    }
    catch(std::exception& e)
    {
        c.puts(e.what());
    }

    // Clean up
    return 0;
}