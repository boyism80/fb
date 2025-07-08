#include <fb/bot/integration/test_case.h>
#include <fb/bot/integration/game_controller.h>
#include <fb/logger.h>
#include <fb/game/protocol.h>
#include <format>

namespace fb::bot::integration {

void bot_integration_test::notify_ready()
{
    this->_controller.notify_test_ready();
}

// Utility functions for common bot operations

async::task<spawned_monster_info> bot_integration_test::spawn_monster_with_validator(
    std::shared_ptr<fb::bot::game_bot>                               bot,
    const std::string&                                               monster_name,
    uint16_t                                                         x,
    uint16_t                                                         y,
    std::function<bool(const fb::protocol::game::response::update&)> validator)
{
    auto thread = bot->thread();
    co_await thread->switching();

    auto&& spawn_response = co_await bot->request<fb::protocol::game::response::update>(
        fb::protocol::game::request::chat{false, std::format("/몬스터생성 {} {} {}", monster_name, x, y)},
        validator,
        DEFAULT_TIMEOUT);

    if (spawn_response.objects_data.empty())
    {
        throw std::runtime_error(std::format("Failed to spawn monster {} at ({}, {})", monster_name, x, y));
    }

    auto&                mob = spawn_response.objects_data.front();
    spawned_monster_info monster_info;
    monster_info.oid      = mob.oid;
    monster_info.position = fb::model::point<uint16_t>(mob.x, mob.y);
    monster_info.look     = mob.look;

    co_return monster_info;
}

async::task<spawned_monster_info> bot_integration_test::spawn_monster_by_look(std::shared_ptr<fb::bot::game_bot> bot,
                                                                              const std::string& monster_name,
                                                                              uint16_t           x,
                                                                              uint16_t           y,
                                                                              uint32_t           expected_look)
{
    auto thread = bot->thread();
    co_await thread->switching();

    auto&& spawn_response = co_await bot->request<fb::protocol::game::response::update>(
        fb::protocol::game::request::chat{false, std::format("/몬스터생성 {} {} {}", monster_name, x, y)},
        [expected_look](auto& resp) -> bool {
            if (resp.objects_data.empty())
                return false;
            auto& mob = resp.objects_data.front();
            return mob.look == expected_look;
        },
        DEFAULT_TIMEOUT);

    if (spawn_response.objects_data.empty())
    {
        throw std::runtime_error(
            std::format("Failed to spawn monster {} with look {} at ({}, {})", monster_name, expected_look, x, y));
    }

    auto&                mob = spawn_response.objects_data.front();
    spawned_monster_info monster_info;
    monster_info.oid      = mob.oid;
    monster_info.position = fb::model::point<uint16_t>(mob.x, mob.y);
    monster_info.look     = mob.look;

    co_return monster_info;
}

async::task<std::vector<spawned_monster_info>> bot_integration_test::spawn_monsters_relative_with_validator(
    std::shared_ptr<fb::bot::game_bot>                               bot,
    const std::string&                                               monster_name,
    const std::vector<std::pair<int, int>>&                          relative_positions,
    std::function<bool(const fb::protocol::game::response::update&)> validator)
{
    auto thread = bot->thread();
    co_await thread->switching();

    auto                              caster_pos = bot->position();
    std::vector<spawned_monster_info> spawned_monsters;

    for (const auto& [rel_x, rel_y] : relative_positions)
    {
        auto monster_x = caster_pos.x + rel_x;
        auto monster_y = caster_pos.y + rel_y;

        auto&& spawn_response = co_await bot->request<fb::protocol::game::response::update>(
            fb::protocol::game::request::chat{false,
                                              std::format("/몬스터생성 {} {} {}", monster_name, monster_x, monster_y)},
            validator,
            DEFAULT_TIMEOUT);

        if (spawn_response.objects_data.empty())
        {
            throw std::runtime_error(
                std::format("Failed to spawn monster {} at ({}, {})", monster_name, monster_x, monster_y));
        }

        auto&                mob = spawn_response.objects_data.front();
        spawned_monster_info monster_info;
        monster_info.oid      = mob.oid;
        monster_info.position = fb::model::point<uint16_t>(mob.x, mob.y);
        monster_info.look     = mob.look;
        spawned_monsters.push_back(monster_info);
    }

    co_return spawned_monsters;
}

async::task<std::vector<spawned_monster_info>>
bot_integration_test::spawn_monsters_relative_by_look(std::shared_ptr<fb::bot::game_bot>      bot,
                                                      const std::string&                      monster_name,
                                                      const std::vector<std::pair<int, int>>& relative_positions,
                                                      uint32_t                                expected_look)
{
    auto thread = bot->thread();
    co_await thread->switching();

    auto                              caster_pos = bot->position();
    std::vector<spawned_monster_info> spawned_monsters;

    for (const auto& [rel_x, rel_y] : relative_positions)
    {
        auto monster_x = caster_pos.x + rel_x;
        auto monster_y = caster_pos.y + rel_y;

        auto&& spawn_response = co_await bot->request<fb::protocol::game::response::update>(
            fb::protocol::game::request::chat{false,
                                              std::format("/몬스터생성 {} {} {}", monster_name, monster_x, monster_y)},
            [expected_look](auto& resp) -> bool {
                if (resp.objects_data.empty())
                    return false;
                auto& mob = resp.objects_data.front();
                return mob.look == expected_look;
            },
            DEFAULT_TIMEOUT);

        if (spawn_response.objects_data.empty())
        {
            throw std::runtime_error(std::format("Failed to spawn monster {} with look {} at ({}, {})",
                                                 monster_name,
                                                 expected_look,
                                                 monster_x,
                                                 monster_y));
        }

        auto&                mob = spawn_response.objects_data.front();
        spawned_monster_info monster_info;
        monster_info.oid      = mob.oid;
        monster_info.position = fb::model::point<uint16_t>(mob.x, mob.y);
        monster_info.look     = mob.look;
        spawned_monsters.push_back(monster_info);
    }

    co_return spawned_monsters;
}

async::task<spawned_monster_info>
bot_integration_test::spawn_monster_relative_by_look(std::shared_ptr<fb::bot::game_bot> bot,
                                                     const std::string&                 monster_name,
                                                     int                                relative_x,
                                                     int                                relative_y,
                                                     uint32_t                           expected_look)
{
    auto thread = bot->thread();
    co_await thread->switching();

    auto caster_pos = bot->position();
    auto monster_x  = caster_pos.x + relative_x;
    auto monster_y  = caster_pos.y + relative_y;

    auto&& spawn_response = co_await bot->request<fb::protocol::game::response::update>(
        fb::protocol::game::request::chat{false,
                                          std::format("/몬스터생성 {} {} {}", monster_name, monster_x, monster_y)},
        [expected_look](auto& resp) -> bool {
            if (resp.objects_data.empty())
                return false;
            auto& mob = resp.objects_data.front();
            return mob.look == expected_look;
        },
        DEFAULT_TIMEOUT);

    if (spawn_response.objects_data.empty())
    {
        throw std::runtime_error(std::format("Failed to spawn monster {} with look {} at relative position ({}, {})",
                                             monster_name,
                                             expected_look,
                                             relative_x,
                                             relative_y));
    }

    auto&                mob = spawn_response.objects_data.front();
    spawned_monster_info monster_info;
    monster_info.oid      = mob.oid;
    monster_info.position = fb::model::point<uint16_t>(mob.x, mob.y);
    monster_info.look     = mob.look;

    co_return monster_info;
}

async::task<bool> bot_integration_test::set_max_hp_mp(std::shared_ptr<fb::bot::game_bot> bot, int max_hp, int max_mp)
{
    auto thread = bot->thread();
    co_await thread->switching();

    auto hp_result = co_await bot->request<fb::protocol::game::response::update_internal>(
        fb::protocol::game::request::chat{false, std::format("/체력바꾸기 {}", max_hp)},
        DEFAULT_TIMEOUT);

    auto mp_result = co_await bot->request<fb::protocol::game::response::update_internal>(
        fb::protocol::game::request::chat{false, std::format("/마력바꾸기 {}", max_mp)},
        DEFAULT_TIMEOUT);

    co_return true; // Both commands should succeed if bot is valid
}

async::task<bool> bot_integration_test::set_current_hp_mp(std::shared_ptr<fb::bot::game_bot> bot,
                                                          int                                current_hp,
                                                          int                                current_mp)
{
    auto thread = bot->thread();
    co_await thread->switching();

    auto hp_result = co_await bot->request<fb::protocol::game::response::update_internal>(
        fb::protocol::game::request::chat{false, std::format("/현재체력 {}", current_hp)},
        [current_hp](auto& resp) -> bool {
            return resp.ch_hp == current_hp;
        },
        DEFAULT_TIMEOUT);

    auto mp_result = co_await bot->request<fb::protocol::game::response::update_internal>(
        fb::protocol::game::request::chat{false, std::format("/현재마력 {}", current_mp)},
        [current_mp](auto& resp) -> bool {
            return resp.ch_mp == current_mp;
        },
        DEFAULT_TIMEOUT);

    co_return true; // Both commands should succeed if bot is valid
}

async::task<bool> bot_integration_test::setup_bot_stats(std::shared_ptr<fb::bot::game_bot> bot,
                                                        int                                max_hp,
                                                        int                                max_mp,
                                                        std::optional<int>                 current_hp,
                                                        std::optional<int>                 current_mp)
{
    auto thread = bot->thread();
    co_await thread->switching();

    // Set max HP/MP
    std::ignore = co_await bot->request<fb::protocol::game::response::update_internal>(
        fb::protocol::game::request::chat{false, std::format("/체력바꾸기 {}", max_hp)},
        DEFAULT_TIMEOUT);

    std::ignore = co_await bot->request<fb::protocol::game::response::update_internal>(
        fb::protocol::game::request::chat{false, std::format("/마력바꾸기 {}", max_mp)},
        DEFAULT_TIMEOUT);

    // Set current HP/MP if specified
    if (current_hp.has_value())
    {
        std::ignore = co_await bot->request<fb::protocol::game::response::update_internal>(
            fb::protocol::game::request::chat{false, std::format("/현재체력 {}", current_hp.value())},
            [current_hp](auto& resp) -> bool {
                return resp.ch_hp == current_hp.value();
            },
            DEFAULT_TIMEOUT);
    }

    if (current_mp.has_value())
    {
        std::ignore = co_await bot->request<fb::protocol::game::response::update_internal>(
            fb::protocol::game::request::chat{false, std::format("/현재마력 {}", current_mp.value())},
            [current_mp](auto& resp) -> bool {
                return resp.ch_mp == current_mp.value();
            },
            DEFAULT_TIMEOUT);
    }

    co_return true;
}

async::task<size_t> bot_integration_test::learn_spells(std::shared_ptr<fb::bot::game_bot> bot,
                                                       const std::vector<std::string>&    spell_names)
{
    auto thread = bot->thread();
    co_await thread->switching();

    size_t learned_count = 0;
    for (const auto& spell_name : spell_names)
    {
        auto result = co_await bot->request<fb::protocol::game::response::spell_update>(
            fb::protocol::game::request::chat{false, std::format("/마법배우기 {}", spell_name)},
            DEFAULT_TIMEOUT);

        if (result.index != 0xFF) // Success if index is not 0xFF
        {
            learned_count++;
        }
        else
        {
            fb::logger::warn("Failed to learn spell: {}", spell_name);
        }
    }

    co_return learned_count;
}

async::task<void> bot_integration_test::clear_all_spells(std::shared_ptr<fb::bot::game_bot>& bot)
{
    auto thread = bot->thread();
    co_await thread->switching();

    auto count = bot->spells().size();
    if (count == 0)
        co_return;

    auto last_slot = uint8_t{0};
    for (auto& [slot, spell] : bot->spells())
    {
        last_slot = std::max<uint8_t>(last_slot, slot);
    }

    bot->send(fb::protocol::game::request::chat{false, "/마법지우기"});
    co_return; // Command should succeed if bot is valid
}

async::task<void> bot_integration_test::clear_all_items(std::shared_ptr<fb::bot::game_bot>& bot)
{
    auto thread = bot->thread();
    co_await thread->switching();

    bot->send(fb::protocol::game::request::chat{false, "/아이템삭제"});
    co_return;
}

async::task<void> bot_integration_test::move_bot_back_to_position(std::shared_ptr<fb::bot::game_bot> bot,
                                                                  const fb::model::point<uint16_t>&  original_position)
{
    auto thread = bot->thread();
    co_await thread->switching();

    auto current_position = bot->position();
    auto move_y_axis      = current_position.y - original_position.y;

    if (move_y_axis > 0)
    {
        for (auto i = 0; i < move_y_axis; i++)
        {
            bot->send(fb::protocol::game::request::move{DIRECTION::TOP, bot->oid(), current_position});
            co_await thread->sleep(DEFAULT_INTERVAL);
            current_position.y--;
            bot->set_position(current_position);
        }
        bot->send(fb::protocol::game::request::direction{DIRECTION::BOTTOM});
    }
}

} // namespace fb::bot::integration