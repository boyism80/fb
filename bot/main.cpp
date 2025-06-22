#include <thread>
#include <fb/bot/bot.container.h>
#include <fb/config.h>

using namespace std;
using namespace boost::asio;
using namespace std::chrono_literals;

int main(int argc, char** argv)
{
    // Initialize config system
    if (!fb::init_config(argc, argv))
        return -1;
    using guard_type = executor_work_guard<io_context::executor_type>;

    auto io_size        = fb::config<uint32_t>("io_size");
    auto ios            = std::vector<std::unique_ptr<io_context>>{};
    auto guards         = vector<unique_ptr<guard_type>>();
    auto bot_containers = vector<shared_ptr<fb::bot::bot_container>>();

    for (int i = 0; i < io_size; i++)
    {
        auto io = std::make_unique<io_context>();
        guards.push_back(std::make_unique<guard_type>(io->get_executor()));
        auto container = std::make_shared<fb::bot::bot_container>(*io.get());
        container->initialize();
        bot_containers.push_back(container);
        ios.push_back(std::move(io));
    }

    bool exit           = false;
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