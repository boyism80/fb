#include <fb/bot/integration/item_test.h>
#include <fb/bot/integration/game_controller.h>

using namespace std::chrono_literals;
using namespace fb::bot::integration;
using namespace fb;

namespace game_reqs = fb::protocol::game::request;
namespace game_resp = fb::protocol::game::response;

async::task<bool> item_test::test_item_combine(uint32_t index)
{
    fb::logger::debug("Starting scenario 3-1: Item combine");

    auto& recipes = table::recipe;
    auto  bots    = this->get_test_bots();
    auto& bot     = bots[index];
    auto  passed  = true;
    for (int i1 = 0; i1 < recipes.size(); i1++)
    {
        if (i1 % bot_count != index)
            continue;

        co_await bot->clear_inventory(DEFAULT_TIMEOUT);
        co_await this->sleep(DEFAULT_INTERVAL);

        auto& recipe                 = recipes[i1];
        auto  expected_success_items = std::vector<std::string>{};
        auto  expected_failed_items  = std::vector<std::string>{};
        auto  slots                  = std::vector<uint8_t>{};

        for (auto& dsl : recipe.success)
        {
            auto params = dsl::item(dsl.params);
            expected_success_items.push_back(table::item[params.id].name);
        }

        for (auto& dsl : recipe.failed)
        {
            auto params = dsl::item(dsl.params);
            expected_failed_items.push_back(table::item[params.id].name);
        }

        bot->chat(std::format("Scenario 3-1: Expected items: {} or {}",
                              boost::algorithm::join(expected_success_items, ", "),
                              boost::algorithm::join(expected_failed_items, ", ")));

        auto slot = 0;
        for (auto& source : recipe.source)
        {
            auto  params = dsl::item(source.params);
            auto& item   = table::item[params.id];
            auto  remain = params.count;

            while (remain > 0)
            {
                auto count  = std::min<uint16_t>(item.capacity, remain);
                remain     -= count;
                co_await bot->create_item(item.name, count, DEFAULT_TIMEOUT);
                slots.push_back(slot++);
            }
        }

        auto   success = false;
        auto&& resp    = co_await bot->request<game_resp::message>(
            game_reqs::item_combine(slots),
            [&success](auto& resp) -> bool {
                if (resp.type != MESSAGE_TYPE::STATE)
                    return false;

                return true;
            },
            DEFAULT_TIMEOUT);

        if (resp.text.find(_TEXT(MESSAGE_NO_RECIPE)) != std::string::npos)
        {
            fb::logger::fatal("Scenario 3-1: No recipe found");
            bot->chat("Scenario 3-1: No recipe found");
            passed = false;
            continue;
        }

        success = resp.text.find(_TEXT(MESSAGE_MIX_SUCCESS)) != std::string::npos;
        if (success)
        {
            for (auto& dsl : recipe.success)
            {
                auto  params = dsl::item(dsl.params);
                auto& item   = table::item[params.id];
                auto  count  = params.count;

                if (bot->has_item_by_name(item.name) == false)
                {
                    fb::logger::fatal("Scenario 3-1: Item not found in inventory: {}", item.name);
                    passed = false;
                }

                if (bot->get_item_count_by_name(item.name) != count)
                {
                    fb::logger::fatal("Scenario 3-1: Item count mismatch: {} != {}",
                                      bot->get_item_count_by_name(item.name),
                                      count);
                    passed = false;
                }
            }
        }
        else
        {
            for (auto& dsl : recipe.failed)
            {
                auto  params = dsl::item(dsl.params);
                auto& item   = table::item[params.id];
                auto  count  = params.count;

                if (bot->has_item_by_name(item.name) == false)
                {
                    fb::logger::fatal("Scenario 3-1: Item not found in inventory: {}", item.name);
                    passed = false;
                }
            }
        }

        bot->chat(std::format("combine result : {}", success ? "success" : "failed"));
    }
    co_return passed;
}

async::task<bool> item_test::test_item_combine_failure()
{
    fb::logger::debug("Starting scenario 3-2: Item combine failure");

    auto  bots = this->get_test_bots();
    auto& bot  = bots.front();

    co_await bot->create_item("목도", 1, DEFAULT_TIMEOUT);
    co_await bot->create_item("목검", 1, DEFAULT_TIMEOUT);
    co_await bot->create_item("사두목도", 1, DEFAULT_TIMEOUT);
    co_await bot->create_item("사두목검", 1, DEFAULT_TIMEOUT);
    co_await bot->create_item("뢰진도", 1, DEFAULT_TIMEOUT);

    bot->chat("Scenario 3-2: Try to combine items with no recipe");
    std::ignore = co_await bot->request<game_resp::message>(
        game_reqs::item_combine({0, 1, 2, 3, 4}),
        [](auto& resp) -> bool {
            if (resp.type != MESSAGE_TYPE::STATE)
                return false;

            return resp.text.find(_TEXT(MESSAGE_NO_RECIPE)) != std::string::npos;
        },
        DEFAULT_TIMEOUT);

    bot->chat("Scenario 3-2: Successfully combined items with no recipe");
    co_return true;
}