#ifndef FB_BOT_BOT_TEST_RUNNER_H
#define FB_BOT_BOT_TEST_RUNNER_H

#include <thread>
#include <memory>
#include <string_view>
#include <boost/asio.hpp>
#include <fb/console.h>
#include <fb/config.h>
#include <fb/bot/container.h>
#include <fb/bot/test_mode.h>
#include <fb/bot/bot_controller_factory.h>

using namespace std::chrono_literals;

inline void display_spawned_bots(const fb::bot::bot_container& container)
{
    fb::console::puts("gateway\t\t{}", container.gateway->bot_count());
    fb::console::puts("login\t\t{}", container.login->bot_count());
    fb::console::puts("game\t\t{}", container.game->bot_count());
    fb::console::up(3);
}

template <fb::bot::test_mode Mode>
void run_bot_test(std::string_view config_path)
{
    if (!fb::init_config(config_path))
    {
        fb::console::puts("Failed to initialize config system with file: {}", config_path);
        return;
    }

    using guard_type = boost::asio::executor_work_guard<boost::asio::io_context::executor_type>;

    auto logic_thread_count = bot_controller_factory<Mode>::get_logic_thread_count();
    auto io_thread_count    = bot_controller_factory<Mode>::get_io_thread_count();

    auto io_context = boost::asio::io_context{};
    auto work_guard = guard_type{io_context.get_executor()};

    auto container = std::make_shared<fb::bot::bot_container>(io_context, logic_thread_count);
    container->set_gateway_bot_controller(bot_controller_factory<Mode>::create_gateway_controller(*container));
    container->set_login_bot_controller(bot_controller_factory<Mode>::create_login_controller(*container));
    container->set_game_bot_controller(bot_controller_factory<Mode>::create_game_controller(*container));
    container->initialize();

    fb::console::set_mode(fb::console::mode::plain);

    auto exit           = false;
    auto display_thread = std::unique_ptr<std::thread>();
    if (bot_controller_factory<Mode>::should_create_display_thread())
    {
        display_thread = std::make_unique<std::thread>([&exit, container]() {
            while (!exit)
            {
                display_spawned_bots(*container);
                std::this_thread::sleep_for(100ms);
            }
        });
    }

    auto io_workers = boost::asio::thread_pool{io_thread_count};
    for (uint32_t i = 0; i < io_thread_count; i++)
    {
        boost::asio::post(io_workers, [&io_context] {
            io_context.run();
        });
    }

    // Release work_guard when the container exits so io_context::run / join can finish.
    auto watcher = std::thread([container, &work_guard, &io_context]() {
        while (container->running())
            std::this_thread::sleep_for(100ms);

        work_guard.reset();
        io_context.stop();
    });

    io_workers.join();

    if (watcher.joinable())
        watcher.join();

    exit = true;
    if (display_thread)
        display_thread->join();
}

#endif // FB_BOT_BOT_TEST_RUNNER_H
