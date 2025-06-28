#include <fb/gateway/context.h>
#include <fb/socket.h>
#include <iomanip>
#include <iostream>
#include <stdio.h>
#include <fb/leak.h>
#include <fb/config.h>
#include <fb/console.h>
#include <fb/protocol/flatbuffer/protocol.h>
#include <boost/program_options.hpp>
#include <filesystem>
#ifndef _WIN32
#include <execinfo.h>
#else
#include "resource.h"
#endif

using namespace fb;

int main(int argc, char** argv)
{
    namespace po = boost::program_options;

    try
    {
        po::options_description desc("Gateway Server Options");
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
        //_CrtSetBreakAlloc(165);

#ifdef _WIN32
        ::set_console_icon(IDI_BARAM);
        ::SetConsoleTitle(CONSOLE_TITLE);
        flatbuffers::option::encoding(utf8);
        flatbuffers::option::decoding(cp949);
#endif

        auto io_context = boost::asio::io_context{};
        auto context    = std::make_shared<fb::gateway::context>(io_context, fb::config<uint16_t>("port"));
        auto signals    = boost::asio::signal_set(io_context, SIGINT, SIGTERM);
        signals.async_wait([&context](const boost::system::error_code& ec, int signal_number) {
            context->exit();
        });
        context->run();
    }
    catch (std::exception& e)
    {
        std::cerr << "unhandled exception catched in main : " << e.what() << std::endl;
    }

    // Clean up
    return 0;
}