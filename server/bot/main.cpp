#include <iostream>
#include <boost/program_options.hpp>
#include <fb/console.h>
#include <fb/config.h>
#include <fb/encoding.h>
#include <fb/model/model.h>
#include <fb/bot/test_mode.h>
#include <fb/bot/bot_controller_factory.h>
#include <fb/bot/bot_test_runner.h>

using namespace std;
namespace po = boost::program_options;

int main(int argc, char** argv)
{
#ifdef _WIN32
    fb::model::option::decoding(fb::cp949);
#endif

    // Parse command line options
    po::options_description desc("Bot Test Options");
    desc.add_options()("help,h", "Show help message")(
        "mode,m",
        po::value<string>()->default_value("load"),
        "Test mode: 'load' for load testing, 'integration' for integration testing")(
        "config,c",
        po::value<string>()->default_value("config/config.dev.json"),
        "Configuration file path");

    po::variables_map vm;
    try
    {
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);
    }
    catch (const po::error& e)
    {
        fb::console::puts("Error parsing command line: {}", e.what());

        auto sstream = std::stringstream();
        sstream << desc;
        fb::console::puts("{}", sstream.str());
        return -1;
    }

    if (vm.count("help"))
    {
        auto sstream = std::stringstream();
        sstream << desc;
        fb::console::puts("{}", sstream.str());
        return 0;
    }

    auto& config_path = vm["config"].as<string>();
    auto& mode_str    = vm["mode"].as<string>();
    if (mode_str == "integration")
    {
        run_bot_test<fb::bot::test_mode::INTEGRATION_TEST>(config_path);
    }
    else if (mode_str == "load")
    {
        run_bot_test<fb::bot::test_mode::LOAD_TEST>(config_path);
    }
    else
    {
        fb::console::puts("Invalid test mode: {}", mode_str);
        fb::console::puts("Valid modes: 'load', 'integration'");
        return -1;
    }

    return 0;
}