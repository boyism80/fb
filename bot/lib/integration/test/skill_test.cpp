#include <fb/bot/integration/skill_test.h>
#include <fb/bot/integration/game_controller.h>
#include <fb/bot/integration/gateway_controller.h>
#include <fb/logger.h>
#include <fb/game/protocol/object/chat.h>
#include <fb/game/protocol/spell/spell_cast.h>
#include <fb/game/protocol/spell/spell_update.h>
#include <chrono>

using namespace std::chrono_literals;

namespace fb::bot::integration {

async::task<void> skill_test::initialize(game_bot_controller& controller)
{
    constexpr auto REQUIRED_BOTS = 5;

    auto ip = controller.container.ipv4(fb::config<std::string>("ip"));
    auto endpoint =
        boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(ip), fb::config<uint16_t>("port"));

    fb::logger::info("Skill test initializing and spawning {} bot", REQUIRED_BOTS);

    for (auto i = 0; i < REQUIRED_BOTS; i++)
    {
        auto gateway_bot = controller.container.gateway->create();
        gateway_bot->connect(endpoint);
    }

    fb::logger::info("Skill test initialization completed - {} bot spawned", REQUIRED_BOTS);
    co_return;
}

async::task<bool> skill_test::execute()
{
    constexpr auto interval = 250ms; // Increased from 250ms to reduce server load
    constexpr auto timeout  = 5s;    // Increased from 5s to handle server processing delays

    if (this->_test_running || this->_test_completed)
        co_return false;

    this->_test_running = true;

    auto bots = this->get_test_bots();
    fb::logger::info("Starting skill test with {} bot", bots.size());

    if (bots.empty())
    {
        fb::logger::fatal("No bots available for skill test");
        this->_test_running = false;
        co_return false;
    }

    // Step 1: Select the test bot for skill operations
    auto caster = bots.at(0);
    auto target = bots.at(1);

    fb::logger::info("Bot {} starting skill learning oid", caster->oid());

    // Step 2: Increase bot's MP to ensure sufficient mana for spell learning and casting
    auto thread = caster->thread();
    co_await thread->switching();

    std::ignore = co_await caster->request<fb::protocol::game::response::update_internal>(
        fb::protocol::game::request::chat{false, "/체력바꾸기 100000"},
        timeout);

    std::ignore = co_await caster->request<fb::protocol::game::response::update_internal>(
        fb::protocol::game::request::chat{false, "/현재체력 50"},
        timeout);

    std::ignore = co_await caster->request<fb::protocol::game::response::update_internal>(
        fb::protocol::game::request::chat{false, "/마력바꾸기 100000"},
        timeout);

    // Step 3: Learn the spell "누리의기원" and wait for spell_update response
    std::ignore = co_await caster->request<fb::protocol::game::response::spell_update>(
        fb::protocol::game::request::chat{false, "/마법배우기 누리의기원"},
        timeout);
    fb::logger::debug("Skill test: Bot {} sent spell learning chat message", caster->oid());

    // Step 4: Cast the learned spell 10 times with improved intervals
    for (auto i = 0; i < SPELL_CAST_COUNT; i++)
    {
        auto before_hp = caster->hp();
        std::ignore    = co_await caster->request<fb::protocol::game::response::update_internal>(
            fb::protocol::game::request::spell_cast(1, "", 0, {0, 0}),
            [before_hp](auto& resp) -> bool {
                return resp.ch_hp == before_hp + 50;
            },
            timeout);

        co_await caster->thread()->sleep(interval);
    }

    this->_test_completed = true;
    this->_test_running   = false;

    fb::logger::info("Skill test completed successfully - {} spell casts performed", SPELL_CAST_COUNT);

    // Cleanup bots after test completion
    this->cleanup();

    co_return true; // 성공
}

void skill_test::reset()
{
    this->_test_completed           = false;
    this->_test_running             = false;
    this->_waiting_for_spell_update = false;
    this->_spell_learned_index      = 0;

    fb::logger::info("Skill test reset");
}

bool skill_test::is_ready() const
{
    auto bots = this->get_test_bots();
    if (bots.empty())
        return false;

    // Movement test requires all bots to have non-zero oid
    for (const auto& bot : bots)
    {
        if (bot->oid() == 0)
            return false;
    }

    return true;
}

void skill_test::on_bot_connected(std::shared_ptr<fb::bot::game_bot> bot)
{
    this->_test_bots.push_back(bot);
    fb::logger::debug("Skill test: Bot {} added to collection", bot->fd());
}

void skill_test::on_spell_update_received(std::shared_ptr<fb::bot::game_bot> bot, uint8_t index)
{
    fb::logger::info("Skill test: Bot {} received spell_update with index {}", bot->fd(), index);

    this->_spell_learned_index      = index;
    this->_waiting_for_spell_update = false;
}

} // namespace fb::bot::integration