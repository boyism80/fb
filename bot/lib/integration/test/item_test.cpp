#include <fb/bot/integration/item_test.h>
#include <fb/bot/integration/game_controller.h>
#include <fb/bot/integration/gateway_controller.h>

using namespace std::chrono_literals;

namespace fb::bot::integration {

item_test::item_test(game_bot_controller& controller) :
    bot_integration_test(controller, 99)
{ }

async::task<void> item_test::on_initialize(game_bot_controller& controller)
{
    super::on_initialize(controller);

    auto bots = this->get_test_bots();

    // Position bots in a 2D grid from (5,5) to (15,13)
    for (int i = 0; i < static_cast<int>(bots.size()); ++i)
    {
        auto& bot = bots[i];

        // Calculate grid position: bot 0 = (5,5), bot 1 = (6,5), etc.
        // Grid size: 11x9 (5 to 15 for x, 5 to 13 for y)
        int grid_x = 5 + (i % 11); // 5 to 15
        int grid_y = 5 + (i / 11); // 5 to 13

        // Move bot to calculated position
        if (bot == bots.back())
            co_await bot->map_move("낙랑의방", grid_x, grid_y, DEFAULT_TIMEOUT);
        else
            std::ignore = bot->map_move("낙랑의방", grid_x, grid_y, DEFAULT_TIMEOUT);

        fb::logger::debug("Bot {} positioned at ({}, {}) facing BOTTOM",
                          bot->fd(),
                          bot->position().x,
                          bot->position().y);
    }

    co_return;
}

async::task<void> item_test::on_scenario_started(uint32_t scenario_index)
{
    co_return;
}

async::task<void> item_test::on_scenario_finished(uint32_t scenario_index)
{
    co_return;
}

async::task<void> item_test::on_parallel_scenario_started(uint32_t id)
{
    co_return;
}

async::task<void> item_test::on_parallel_scenario_finished(uint32_t id)
{
    auto  bots = this->get_test_bots();
    auto& bot  = bots[id];

    fb::logger::debug("Resetting bot {} state to clean initial conditions", bot->oid());

    // Clear all items from inventory
    co_await bot->clear_all_drop_items(DEFAULT_TIMEOUT);
    co_await bot->clear_inventory(DEFAULT_TIMEOUT);
    co_await this->sleep(500ms);

    // Reset money to 0
    co_await bot->change_money(0, DEFAULT_TIMEOUT);

    // Reset to default level (level 1)
    co_await bot->change_level(1, DEFAULT_TIMEOUT);

    // Reset all stats to 1
    co_await bot->change_stats(1, 1, 1, DEFAULT_TIMEOUT);

    // Reset sex to male
    co_await bot->change_sex(fb::model::enum_value::SEX::MAN, DEFAULT_TIMEOUT);

    // Reset base HP and MP to reasonable defaults
    co_await bot->change_base_hp(1000, DEFAULT_TIMEOUT);
    co_await bot->change_base_mp(1000, DEFAULT_TIMEOUT);
    co_return;
}

generator<bot_integration_test::scenario_t> item_test::on_generate_scenario()
{
    auto scenarios = std::vector<std::pair<uint32_t, scenario_t>>{};
    for (int i = 0; i < this->bot_count; i++)
    {
        scenarios.push_back({i, [this, i]() -> async::task<bool> {
                                 co_return co_await this->test_equipment(i);
                             }});
    }

    auto scenarios_item_combine = std::vector<std::pair<uint32_t, scenario_t>>{};
    for (int i = 0; i < this->bot_count; i++)
    {
        scenarios_item_combine.push_back({i, [this, i]() -> async::task<bool> {
                                              co_return co_await this->test_item_combine(i);
                                          }});
    }

    co_yield [this, scenarios]() -> async::task<bool> {
        co_return co_await this->parallel_scenarios(scenarios);
    };

    co_yield [this]() -> async::task<bool> {
        co_return co_await this->test_equipment_overflow();
    };

    co_yield [this, scenarios_item_combine]() -> async::task<bool> {
        co_return co_await this->parallel_scenarios(scenarios_item_combine);
    };

    co_return;
}

std::string item_test::name() const
{
    return "Item Test";
}

} // namespace fb::bot::integration