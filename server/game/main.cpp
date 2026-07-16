#include <fb/game/server.h>
#include <fb/leak.h>
#include <fb/mst.h>
#include <boost/program_options.hpp>
#include <filesystem>
#ifdef _WIN32
#include <Windows.h>
#include "resource.h"
#else
#include <execinfo.h>
#endif

using namespace fb;

int main(int argc, char* argv[])
{
    namespace po = boost::program_options;

    try
    {
        po::options_description desc("Game Server Options");
        desc.add_options()("help,h", "Show help message")("config,c",
                                                          po::value<std::string>()->default_value("config.json"),
                                                          "Configuration file path");

        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);

        if (vm.count("help"))
            return std::cout << desc << std::endl, 0;

        auto config_path = vm["config"].as<std::string>();

        // Initialize config system
        if (!fb::init_config(config_path))
        {
            std::cerr << "Failed to initialize config from: " << config_path << std::endl;
            return -1;
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "Command line parsing error: " << e.what() << std::endl;
        return -1;
    }

    try
    {
        //_CrtSetBreakAlloc(7997394);

#ifdef _WIN32
        ::set_console_icon(IDI_BARAM);
        ::SetConsoleTitle(CONSOLE_TITLE);
#endif

        auto io_context = boost::asio::io_context{};
        auto server     = std::make_shared<fb::game::server>(io_context, fb::config<uint16_t>("port"));
        auto signals    = boost::asio::signal_set(io_context, SIGINT, SIGTERM);
        signals.async_wait([&server](const boost::system::error_code& ec, int signal_number) {
            server->exit();
        });

        server->run();
    }
    catch (std::exception& e)
    {
        std::cerr << "unhandled exception catched in main : " << e.what() << std::endl;
    }

    // Release
    return 0;
}
