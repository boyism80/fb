#include <fb/bot/integration/attack_test.h>
#include <fb/bot/integration/game_controller.h>

using namespace std::chrono_literals;

namespace fb::bot::integration {

attack_test::attack_test(game_bot_controller& controller) :
    bot_integration_test(controller, 2) // Spawn 2 bots
{
    this->controller.hook(this, this, &attack_test::on_hook_die);
}

async::task<void> attack_test::on_initialize(game_bot_controller& controller)
{
    co_await super::on_initialize(controller);
    co_await super::arrange_bots_in_line_formation();
    co_return;
}

async::task<void> attack_test::on_hook_die(game_bot& bot, const fb::protocol::game::response::die& response)
{
    if (this->_done)
        co_return;

    if (this->_target_oid.has_value() == false)
        co_return;

    if (response.oid != this->_target_oid.value())
        co_return;

    this->_done = true;
    this->_target_oid.reset();
    co_return;
}

async::task<void> attack_test::on_scenario_finished(uint32_t scenario_index)
{
    auto  bots = this->get_test_bots();
    auto& bot  = bots.front();

    co_await bot->clear_all_drop_items(DEFAULT_TIMEOUT);
    this->_done = false;
    co_return;
}

generator<bot_integration_test::scenario_t> attack_test::on_generate_scenario()
{
    co_yield [this]() -> async::task<bool> {
        co_return co_await this->attack_scenario_1();
    };
    co_yield [this]() -> async::task<bool> {
        co_return co_await this->attack_scenario_2();
    };
}

async::task<bool> attack_test::attack_scenario_1()
{
    auto bots = this->get_test_bots();
    if (bots.size() < 2)
        throw std::runtime_error("Need at least 2 bots for enhanced attack test");

    auto& attacker = bots.front();
    auto& observer = bots.back();

    co_await attacker->learn_spell("유인", DEFAULT_TIMEOUT);

    fb::logger::debug("Enhanced attack test: Setting up bot {} for combat", attacker->name());

    // Set bot HP to 100,000 using the dedicated method
    co_await attacker->set_max_hp_mp(100000, 100000, DEFAULT_TIMEOUT);

    // Set bot level to 77 using the dedicated method
    co_await attacker->change_level(77, DEFAULT_TIMEOUT);

    // Create and equip '월아검' weapon
    co_await attacker->create_item("월아검", 1, DEFAULT_TIMEOUT);

    // Equip the weapon using the bot's equip method
    if (co_await attacker->equip(0, DEFAULT_TIMEOUT) == false)
    {
        fb::logger::fatal("Enhanced attack test: Failed to equip 월아검");
        co_return false;
    }

    // Spawn '초급유령' monster below the bot using the dedicated method
    auto bot_position = attacker->position();
    auto monster_info =
        co_await attacker->spawn_monster("초급유령", bot_position.x, bot_position.y + 1, DEFAULT_TIMEOUT);
    this->_target_oid = monster_info.oid;

    std::ignore = co_await attacker->request<fb::protocol::game::response::message>(
        fb::protocol::game::request::click(monster_info.oid),
        [](auto& resp) {
            return resp.text == "초급유령";
        },
        DEFAULT_TIMEOUT);

    std::ignore = co_await attacker->request<fb::protocol::game::response::action>(
        fb::protocol::game::request::spell_cast(SPELL_TYPE::TARGET, 0, "", monster_info.oid, monster_info.position),
        [oid = attacker->oid()](auto& resp) {
            if (resp.oid != oid)
                return false;
            return true;
        },
        DEFAULT_TIMEOUT);

    while (this->_done == false)
    {
        std::ignore = co_await attacker->request<fb::protocol::game::response::action>(
            fb::protocol::game::request::attack{},
            [oid = attacker->oid()](auto& resp) {
                if (resp.oid != oid)
                    return false;

                if (resp.value != ACTION::ATTACK)
                    return false;

                return true;
            },
            DEFAULT_TIMEOUT);

        co_await this->sleep(DEFAULT_INTERVAL);
    }

    fb::logger::debug("Enhanced attack test: Bot {} setup complete, monster spawned at ({}, {})",
                      attacker->name(),
                      monster_info.position.x,
                      monster_info.position.y);

    co_return true;
}

async::task<bool> attack_test::attack_scenario_2()
{
    auto bots     = this->get_test_bots();
    auto attacker = bots[0];
    auto target   = bots[1];

    co_await target->set_max_hp_mp(100000, 100000, DEFAULT_TIMEOUT);
    co_await attacker->direction(DIRECTION::RIGHT, DEFAULT_TIMEOUT);

    while (target->state() != STATE::GHOST)
    {
        std::ignore = co_await attacker->request<fb::protocol::game::response::action>(
            fb::protocol::game::request::attack{},
            [oid = attacker->oid()](auto& resp) {
                if (resp.oid != oid)
                    return false;

                if (resp.value != ACTION::ATTACK)
                    return false;

                return true;
            },
            DEFAULT_TIMEOUT);

        co_await this->sleep(DEFAULT_INTERVAL);
    }

    co_return true;
}

async::task<void> attack_test::on_scenario_started(uint32_t scenario_index)
{
    co_return;
}

std::string attack_test::name() const
{
    return "Attack Test";
}

} // namespace fb::bot::integration