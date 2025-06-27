#include <thread>
#include <iostream>
#include <fb/bot/bot.container.h>
#include <fb/config.h>
#include <boost/program_options.hpp>

using namespace std;
using namespace boost::asio;
using namespace std::chrono_literals;
namespace po = boost::program_options;

enum class test_mode
{
    LOAD_TEST,       // 부하 테스트 (기존 동작)
    INTEGRATION_TEST // 통합 테스트 (새로운 기능)
};

int main(int argc, char** argv)
{
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
        std::cerr << "Error parsing command line: " << e.what() << std::endl;
        std::cerr << desc << std::endl;
        return -1;
    }

    if (vm.count("help"))
        return std::cout << desc << std::endl, 0;

    // Get config file path
    auto config_path = vm["config"].as<string>();

    // Initialize config system with direct path
    if (!fb::init_config(config_path))
    {
        std::cerr << "Failed to initialize config system with file: " << config_path << std::endl;
        return -1;
    }

    // Determine test mode
    auto mode     = test_mode::LOAD_TEST;
    auto mode_str = vm["mode"].as<string>();
    if (mode_str == "integration")
        mode = test_mode::INTEGRATION_TEST;
    else if (mode_str == "load")
        mode = test_mode::LOAD_TEST;
    else
    {
        std::cerr << "Invalid test mode: " << mode_str << std::endl;
        std::cerr << "Valid modes: 'load', 'integration'" << std::endl;
        return -1;
    }

    std::cout << "Starting bot system in " << (mode == test_mode::LOAD_TEST ? "load test" : "integration test")
              << " mode" << std::endl;
    std::cout << "Using config file: " << config_path << std::endl;

    // For now, both modes run the same load test
    // TODO: Implement integration test mode when test system is ready
    using guard_type = executor_work_guard<io_context::executor_type>;

    auto io_size        = fb::config<uint32_t>("io_size");
    auto ios            = std::vector<std::unique_ptr<io_context>>{};
    auto guards         = std::vector<std::unique_ptr<guard_type>>();
    auto bot_containers = std::vector<std::shared_ptr<fb::bot::bot_container>>();

    for (auto i = 0; i < io_size; i++)
    {
        auto io = std::make_unique<io_context>();
        guards.push_back(std::make_unique<guard_type>(io->get_executor()));
        auto container = std::make_shared<fb::bot::bot_container>(*io.get());
        container->initialize();
        bot_containers.push_back(container);
        ios.push_back(std::move(io));
    }

    auto exit           = false;
    auto display_thread = std::thread([&exit]() {
        while (!exit)
        {
            fb::bot::bot_container::display_spawned_bots();
            std::this_thread::sleep_for(100ms);
        }
    });

    auto threads = boost::asio::thread_pool{io_size};
    for (auto& io : ios)
    {
        post(threads, [&io] {
            io->run();
        });
    }
    threads.join();
    exit = true;
    return 0;
}