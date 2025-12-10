#include <fb/bot/integration/door_test.h>
#include <fb/bot/integration/game_controller.h>
#include <fb/bot/integration/gateway_controller.h>
#include <fb/logger.h>
#include <fb/game/protocol.h>
#include <fb/model/model.h>
#include <chrono>

using namespace fb::model::enum_value;
using namespace std::chrono_literals;
using namespace fb::bot::integration;

namespace game_reqs = fb::protocol::game::request;
namespace game_resp = fb::protocol::game::response;

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

    fb::logger::debug("Door test: Bot {} moving to 국내성", bots.front()->name());
    auto bot = co_await bots.front()->transfer(game_reqs::chat(false, "/맵이동 국내성 109 13"), DEFAULT_TIMEOUT);
    co_await bot->direction(DIRECTION::TOP, DEFAULT_TIMEOUT);
    std::ignore = co_await bot->request<game_resp::message>(
        game_reqs::door(),
        [](auto& resp) {
            if (resp.type != MESSAGE_TYPE::STATE)
                return false;

            return resp.text == _TEXT(MESSAGE_DOOR_CLOSE);
        },
        DEFAULT_TIMEOUT);

    std::ignore = co_await bot->request<game_resp::message>(
        game_reqs::door(),
        [](auto& resp) {
            if (resp.type != MESSAGE_TYPE::STATE)
                return false;

            return resp.text == _TEXT(MESSAGE_DOOR_OPEN);
        },
        DEFAULT_TIMEOUT);

    fb::logger::debug("Door test: Bot {} creating 파란열쇠", bot->name());
    co_await bot->create_item("파란열쇠", 1, DEFAULT_TIMEOUT);

    std::ignore = co_await bot->request<game_resp::message>(
        game_reqs::item_active(0),
        [](auto& resp) {
            if (resp.type != MESSAGE_TYPE::STATE)
                return false;

            return resp.text == _TEXT(MESSAGE_DOOR_LOCK);
        },
        DEFAULT_TIMEOUT);

    fb::logger::debug("Door test: Bot {} dropping key", bot->name());
    co_await bot->drop_item(0, false, DEFAULT_TIMEOUT);

    std::ignore = co_await bot->request<game_resp::message>(
        game_reqs::door(),
        [](auto& resp) {
            if (resp.type != MESSAGE_TYPE::STATE)
                return false;

            return resp.text == _TEXT(MESSAGE_DOOR_LOCKED);
        },
        DEFAULT_TIMEOUT);

    std::ignore = co_await bot->request<game_resp::item_update>(
        game_reqs::loot(false),
        [](auto& resp) {
            return resp.name.find("파란열쇠") != std::string::npos;
        },
        DEFAULT_TIMEOUT);

    fb::logger::debug("Door test: Bot {} moving to new door location", bot->name());
    co_await bot->map_move("국내성", 110, 13, DEFAULT_TIMEOUT);

    std::ignore = co_await bot->request<game_resp::message>(
        game_reqs::door(),
        [](auto& resp) {
            if (resp.type != MESSAGE_TYPE::STATE)
                return false;

            return resp.text == _TEXT(MESSAGE_DOOR_CLOSE);
        },
        DEFAULT_TIMEOUT);

    std::ignore = co_await bot->request<game_resp::message>(
        game_reqs::door(),
        [](auto& resp) {
            if (resp.type != MESSAGE_TYPE::STATE)
                return false;

            return resp.text == _TEXT(MESSAGE_DOOR_OPEN);
        },
        DEFAULT_TIMEOUT);

    std::ignore = co_await bot->request<game_resp::message>(
        game_reqs::item_active(0),
        [](auto& resp) {
            if (resp.type != MESSAGE_TYPE::STATE)
                return false;

            return resp.text == _TEXT(MESSAGE_DOOR_OPEN);
        },
        DEFAULT_TIMEOUT);

    co_return true;
}

std::string door_test::name() const
{
    return "Door Test";
}