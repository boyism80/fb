#include <fb/bot/integration/door_test.h>
#include <fb/bot/integration/game_controller.h>
#include <fb/bot/integration/gateway_controller.h>
#include <fb/logger.h>
#include <fb/game/protocol.h>
#include <chrono>

using namespace std::chrono_literals;

namespace fb::bot::integration {

door_test::door_test(game_bot_controller& controller) :
    bot_integration_test(controller, 1) // Spawn 1 bot
{ }

generator<bot_integration_test::scenario_t> door_test::on_generate_scenario()
{
    co_yield [this]() -> async::task<bool> {
        co_return co_await this->door_scenario();
    };
}

async::task<bool> door_test::door_scenario()
{
    auto bots = this->get_test_bots();
    if (bots.empty())
        throw std::runtime_error("No bots available for door test");

    auto bot = co_await bots.front()->transfer(fb::protocol::game::request::chat(false, "/맵이동 국내성 109 13"),
                                               DEFAULT_TIMEOUT);
    co_await bot->direction(DIRECTION::TOP, DEFAULT_TIMEOUT);
    std::ignore = co_await bot->request<fb::protocol::game::response::message>(
        fb::protocol::game::request::door(),
        [](auto& resp) {
            if (resp.type != MESSAGE_TYPE::STATE)
                return false;

            return resp.text == "문을 닫았습니다.";
        },
        DEFAULT_TIMEOUT);

    std::ignore = co_await bot->request<fb::protocol::game::response::message>(
        fb::protocol::game::request::door(),
        [](auto& resp) {
            if (resp.type != MESSAGE_TYPE::STATE)
                return false;

            return resp.text == "문을 열었습니다.";
        },
        DEFAULT_TIMEOUT);

    co_await bot->create_item("파란열쇠", 1, DEFAULT_TIMEOUT);

    std::ignore = co_await bot->request<fb::protocol::game::response::message>(
        fb::protocol::game::request::item_active(0),
        [](auto& resp) {
            if (resp.type != MESSAGE_TYPE::STATE)
                return false;

            return resp.text == "문을 잠궜습니다.";
        },
        DEFAULT_TIMEOUT);

    co_await bot->drop_item(0, false, DEFAULT_TIMEOUT);

    std::ignore = co_await bot->request<fb::protocol::game::response::message>(
        fb::protocol::game::request::door(),
        [](auto& resp) {
            if (resp.type != MESSAGE_TYPE::STATE)
                return false;

            return resp.text == "문이 잠겨있습니다.";
        },
        DEFAULT_TIMEOUT);

    std::ignore = co_await bot->request<fb::protocol::game::response::item_update>(
        fb::protocol::game::request::loot(false),
        [](auto& resp) {
            return resp.name.find("파란열쇠") != std::string::npos;
        },
        DEFAULT_TIMEOUT);

    co_await bot->map_move("국내성", 110, 13, DEFAULT_TIMEOUT);

    std::ignore = co_await bot->request<fb::protocol::game::response::message>(
        fb::protocol::game::request::door(),
        [](auto& resp) {
            if (resp.type != MESSAGE_TYPE::STATE)
                return false;

            return resp.text == "문을 닫았습니다.";
        },
        DEFAULT_TIMEOUT);

    std::ignore = co_await bot->request<fb::protocol::game::response::message>(
        fb::protocol::game::request::door(),
        [](auto& resp) {
            if (resp.type != MESSAGE_TYPE::STATE)
                return false;

            return resp.text == "문을 열었습니다.";
        },
        DEFAULT_TIMEOUT);

    std::ignore = co_await bot->request<fb::protocol::game::response::message>(
        fb::protocol::game::request::item_active(0),
        [](auto& resp) {
            if (resp.type != MESSAGE_TYPE::STATE)
                return false;

            return resp.text == "문을 열었습니다.";
        },
        DEFAULT_TIMEOUT);

    co_return true;
}

std::string door_test::name() const
{
    return "Door Test";
}

} // namespace fb::bot::integration