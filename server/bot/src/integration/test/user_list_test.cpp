#include <fb/bot/integration/user_list_test.h>
#include <fb/bot/integration/game_controller.h>

using namespace std::chrono_literals;
using namespace fb::bot::integration;
using namespace fb;

namespace game_reqs = fb::protocol::game::request;
namespace game_resp = fb::protocol::game::response;

user_list_test::user_list_test(game_bot_controller& controller) :
    bot_integration_test(controller, 21) // Use 21 bots (4x5+1) for user list testing
{ }

async::task<void> user_list_test::on_initialize(game_bot_controller& controller)
{
    co_await this->super::on_initialize(controller);
    co_await this->super::arrange_bots_in_grid_formation(5, 5, 15, 13); // Arrange in grid from (5,5) to (15,13)
}

generator<bot_integration_test::scenario_t> user_list_test::on_generate_scenario()
{
    co_yield [this]() -> async::task<bool> {
        co_return co_await this->test_scenario_1();
    };
}

std::string user_list_test::name() const
{
    return "User List Test";
}

async::task<bool> user_list_test::test_scenario_1()
{
    fb::logger::debug("Starting user list scenario 1 test");

    auto  bots   = this->get_test_bots();
    auto& bot    = bots.front();
    auto  passed = true;
    auto  names  = std::unordered_set<std::string>();

    try
    {
        for (auto& bot : bots)
        {
            names.insert(bot->name());
        }

        auto bot_index = 0;
        auto counts    = std::unordered_map<CLASS, int>();
        for (auto& [cls, promotions] : table::promotion)
        {
            counts[cls] = 0;
            for (auto& [_, promotion] : promotions)
            {
                auto& bot = bots[bot_index++];
                co_await bot->change_class(promotion.name, DEFAULT_TIMEOUT);
                counts[cls]++;
                bot->chat(std::format("Changed class to {}", promotion.name));
            }
        }

        auto&& resp = co_await bot->request<game_resp::user_list>(game_reqs::user_list(), DEFAULT_TIMEOUT);

        if (resp.users.size() < bots.size())
        {
            throw std::runtime_error(std::format("User list scenario 1 failed: user count mismatch: {} < {}",
                                                 resp.users.size(),
                                                 bots.size()));
        }

        for (auto& user : resp.users)
        {
            if (!names.contains(user.name))
                continue;

            names.erase(user.name);
            counts[static_cast<CLASS>(user.cls)]--;
        }

        for (auto& [cls, count] : counts)
        {
            if (count != 0)
                throw std::runtime_error("User list scenario 1 failed: class count mismatch");
        }

        fb::logger::debug("User list scenario 1 passed");
        co_return true;
    }
    catch (const std::exception& e)
    {
        fb::logger::fatal("User list scenario 1 failed: {}", e.what());
        passed = false;
    }

    co_return passed;
}
