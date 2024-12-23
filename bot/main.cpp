#include <thread>
#include <bot.h>

using namespace std;
using namespace boost::asio;

int main(int, char**)
{
    using guard_type = executor_work_guard<io_context::executor_type>;

    auto io_size = fb::config<uint32_t>("io_size");
    auto count   = fb::config<uint32_t>("count");

    auto ios            = std::vector<std::unique_ptr<io_context>>{};
    auto guards         = vector<unique_ptr<guard_type>>();
    auto bot_containers = vector<unique_ptr<fb::bot::bot_container>>();
    for (int i = 0; i < io_size; i++)
    {
        auto io = std::make_unique<io_context>();
        guards.push_back(std::make_unique<guard_type>(io->get_executor()));
        bot_containers.push_back(make_unique<fb::bot::bot_container>(*io.get()));
        ios.push_back(std::move(io));
    }

    auto endpoint =
        ip::tcp::endpoint(ip::address::from_string(fb::config<std::string>("ip")), fb::config<uint16_t>("port"));
    for (auto& bots : bot_containers)
    {
        for (int i = 0; i < count / io_size; i++)
        {
            auto bot = bots->create<fb::bot::gateway_bot>();
            bot->connect(endpoint);
        }
    }

    auto threads = boost::asio::thread_pool{io_size};
    for (auto& io : ios)
    {
        post(threads, [&io] {
            io->run();
        });
    }

    threads.join();
    return 0;
}