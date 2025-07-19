#include <fb/bot/integration/worldmap_test.h>
#include <fb/bot/integration/game_controller.h>
#include <fb/bot/integration/gateway_controller.h>
#include <fb/logger.h>
#include <fb/game/protocol.h>
#include <chrono>

using namespace std::chrono_literals;

namespace fb::bot::integration {

worldmap_test::worldmap_test(game_bot_controller& controller) :
    bot_integration_test(controller, 1)
{ }

generator<bot_integration_test::scenario_t> worldmap_test::on_generate_scenario()
{
    co_yield [this]() -> async::task<bool> {
        co_return co_await this->worldmap_scenario();
    };
}

async::task<bool> worldmap_test::worldmap_scenario()
{
    auto bots = this->get_test_bots();
    if (bots.empty())
        throw std::runtime_error("No bots available for worldmap test");

    auto bot = co_await bots.front()->transfer(fb::protocol::game::request::chat(false, "/맵이동 국내성진입로 8 1"),
                                               DEFAULT_TIMEOUT);

    co_await bot->request<fb::protocol::game::response::map_worlds>(
        fb::protocol::game::request::move(DIRECTION::TOP, bot->oid(), bot->position()),
        DEFAULT_TIMEOUT);

    bot = co_await bot->transfer(fb::protocol::game::request::map_world(0, 1, 0), DEFAULT_TIMEOUT);

    auto& after = this->controller.container.model.world[0][0];
    if (bot->map() != after.map)
    {
        fb::logger::fatal("Bot is not in the map : {}", after.map);
        co_return false;
    }

    if (bot->position() != after.position)
    {
        fb::logger::fatal("Bot is not in the position : {}, {}", after.position.x, after.position.y);
        co_return false;
    }

    co_return true;
}

std::string worldmap_test::name() const
{
    return "Worldmap Test";
}

} // namespace fb::bot::integration