#include <boost/program_options.hpp>
#include "resource.h"
#include "module/db/db.h"
#include "model/acceptor/acceptor.login.h"
#include "module/leak.h"
#include "module/console/console.h"
#include "module/config/config.h"
#include "protocol/internal.h"

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

        auto desc = boost::program_options::options_description("fb login");
        desc.add_options()
            ("env,e", boost::program_options::value<std::string>(), "environment");

        boost::program_options::variables_map vmap;
        boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vmap);
        boost::program_options::notify(vmap);

        auto height = 8;
        c.box(0, 0, c.width()-1, height);

        auto header = "The Kingdom of the wind [LOGIN]";
        c.cursor((c.width()-1 - strlen(header)) / 2, 2).puts(header);

        auto github = "https://github.com/boyism80/fb";
        c.cursor(c.width()-1 - strlen(github) - 3, 4).puts(github);

        auto madeby = "made by cshyeon";
        c.cursor(c.width()-1 - strlen(madeby) - 3, 5).puts(madeby);

        c.cursor(0, height + 1);
    
        // Execute acceptor
        boost::asio::io_context io_context;
        fb::db::bind(io_context);
    
        // Execute acceptor
        auto env = vmap.count("env") ? vmap["env"].as<std::string>().c_str() : 
#if defined DEBUG | defined _DEBUG
            "dev";
#else
            nullptr;
#endif
        auto& config = fb::config::get(env);

        const auto connection = INTERNAL_CONNECTION
        {
            config["internal"]["ip"].asString(),
            (uint16_t)config["internal"]["port"].asInt(),
            [&] (fb::base::socket<>& socket, bool success)
            {
                if(success)
                {
                    socket.send(fb::protocol::internal::request::subscribe(config["id"].asString()));
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
            [&] ()
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
        auto acceptor = std::make_unique<fb::login::acceptor>
        (
            io_context, 
            config["port"].asInt(), 
            config["delay"].asInt(), 
            connection
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