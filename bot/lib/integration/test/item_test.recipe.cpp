#include <fb/bot/integration/item_test.h>
#include <fb/bot/integration/game_controller.h>

using namespace std::chrono_literals;
using namespace fb::bot::integration;
using namespace fb;

async::task<bool> item_test::test_item_combine(uint32_t index)
{
    fb::logger::debug("Starting scenario 3-1: Item combine");

    auto& recipes = this->controller.container.model.recipe;
    auto  bots    = this->get_test_bots();
    auto& bot     = bots[index];
    auto  passed  = true;
    for (int i = 0; i < recipes.size(); i++)
    {
        if (i % bot_count != index)
            continue;

        co_await bot->clear_inventory(DEFAULT_TIMEOUT);
        co_await this->sleep(DEFAULT_INTERVAL);

        auto& recipe                 = recipes[i];
        auto  expected_success_items = std::vector<std::string>{};
        auto  expected_failed_items  = std::vector<std::string>{};
        auto  slots                  = std::vector<uint8_t>{};

        for (auto& dsl : recipe.success)
        {
            auto params = dsl::item(dsl.params);
            expected_success_items.push_back(this->controller.container.model.item[params.id].name);
        }

        for (auto& dsl : recipe.failed)
        {
            auto params = dsl::item(dsl.params);
            expected_failed_items.push_back(this->controller.container.model.item[params.id].name);
        }

        bot->chat(std::format("Scenario 3-1: Expected items: {} or {}",
                              boost::algorithm::join(expected_success_items, ", "),
                              boost::algorithm::join(expected_failed_items, ", ")));

        for (int i2 = 0; i2 < recipe.source.size(); i2++)
        {
            auto& source = recipe.source[i2];
            auto  params = dsl::item(source.params);
            auto& item   = this->controller.container.model.item[params.id];

            co_await bot->create_item(item.name, params.count, DEFAULT_TIMEOUT);
            slots.push_back(i2);
        }

        auto combine_success = false;
        std::ignore          = co_await bot->request<fb::protocol::game::response::message>(
            fb::protocol::game::request::item_combine(slots),
            [&combine_success](auto& resp) -> bool {
                if (resp.type != MESSAGE_TYPE::STATE)
                    return false;

                combine_success = resp.text.find(_TEXT(MESSAGE_MIX_SUCCESS)) != std::string::npos;
                return true;
            },
            DEFAULT_TIMEOUT);

        if (combine_success)
        {
            for (auto& dsl : recipe.success)
            {
                auto  params = dsl::item(dsl.params);
                auto& item   = this->controller.container.model.item[params.id];
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
                auto& item   = this->controller.container.model.item[params.id];
                auto  count  = params.count;

                if (bot->has_item_by_name(item.name) == false)
                {
                    fb::logger::fatal("Scenario 3-1: Item not found in inventory: {}", item.name);
                    passed = false;
                }
            }
        }

        bot->chat(std::format("combine result : {}", combine_success ? "success" : "failed"));
    }
    co_return passed;
}