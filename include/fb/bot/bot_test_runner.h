#ifndef FB_BOT_BOT_TEST_RUNNER_H
#define FB_BOT_BOT_TEST_RUNNER_H

#include <thread>
#include <vector>
#include <memory>
#include <boost/asio.hpp>
#include <fb/console.h>
#include <fb/config.h>
#include <fb/bot/container.h>
#include <fb/bot/test_mode.h>
#include <fb/bot/bot_controller_factory.h>

using namespace std::chrono_literals;

inline void display_spawned_bots(const std::vector<std::shared_ptr<fb::bot::bot_container>>& containers)
{
    size_t total_gateway_count = 0;
    size_t total_login_count   = 0;
    size_t total_game_count    = 0;

    for (const auto& container : containers)
    {
        total_gateway_count += container->gateway->bot_count();
        total_login_count   += container->login->bot_count();
        total_game_count    += container->game->bot_count();
    }

    fb::console::puts("gateway\t\t{}", total_gateway_count);
    fb::console::puts("login\t\t{}", total_login_count);
    fb::console::puts("game\t\t{}", total_game_count);
    fb::console::up(3);
}

template <fb::bot::test_mode Mode>
void run_bot_test(const std::string& config_path)
{
    if (!fb::init_config(config_path))
    {
        fb::console::puts("Failed to initialize config system with file: {}", config_path);
        return;
    }

    using guard_type = boost::asio::executor_work_guard<boost::asio::io_context::executor_type>;

    auto ios            = std::vector<std::unique_ptr<boost::asio::io_context>>{};
    auto guards         = std::vector<std::unique_ptr<guard_type>>();
    auto bot_containers = std::vector<std::shared_ptr<fb::bot::bot_container>>();

    uint32_t io_size      = bot_controller_factory<Mode>::get_io_size();
    uint32_t thread_count = bot_controller_factory<Mode>::get_thread_count();

    for (auto i = 0; i < io_size; i++)
    {
        auto io = std::make_unique<boost::asio::io_context>();
        guards.push_back(std::make_unique<guard_type>(io->get_executor()));

        auto container = std::make_shared<fb::bot::bot_container>(*io.get(), thread_count);
        container->set_gateway_bot_controller(bot_controller_factory<Mode>::create_gateway_controller(*container));
        container->set_login_bot_controller(bot_controller_factory<Mode>::create_login_controller(*container));
        container->set_game_bot_controller(bot_controller_factory<Mode>::create_game_controller(*container));

        container->initialize();
        bot_containers.push_back(container);
        ios.push_back(std::move(io));
    }

    fb::console::set_mode(fb::console::mode::plain);

    auto exit           = false;
    auto display_thread = std::unique_ptr<std::thread>();

    if (bot_controller_factory<Mode>::should_create_display_thread())
    {
        display_thread = std::make_unique<std::thread>([&exit, &bot_containers]() {
            while (!exit)
            {
                display_spawned_bots(bot_containers);
                std::this_thread::sleep_for(100ms);
            }
        });
    }

    auto threads = boost::asio::thread_pool{io_size};
    for (auto& io : ios)
    {
        boost::asio::post(threads, [&io] {
            io->run();
        });
    }
    threads.join();

    exit = true;

    if (display_thread)
    {
        display_thread->join();
    }
}

#endif // FB_BOT_BOT_TEST_RUNNER_H
