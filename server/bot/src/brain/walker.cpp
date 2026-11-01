#include <fb/bot/brain/brain.h>

#include <fb/bot/game_controller.h>
#include <fb/logger.h>

#include <chrono>
#include <cstdlib>
#include <algorithm>
#include <exception>
#include <optional>
#include <string>
#include <unordered_set>
#include <utility>

using namespace fb::bot;
using namespace std::chrono_literals;

namespace game_reqs = fb::protocol::game::request;
namespace game_resp = fb::protocol::game::response;

constexpr auto STEP_TIMEOUT = 5s;
constexpr auto RUSH_YIELD   = 1ms;

void brain::act(uint32_t bot, std::string_view activity)
{
    auto lock  = std::lock_guard(this->_mutex);
    auto found = this->_minds.find(bot);
    if (found == this->_minds.end())
        return;

    auto& activities = found->second.activities;
    if (activities.empty() || activities.back() != activity)
        activities.emplace_back(activity);
}

async::task<brain::event> brain::walk(std::shared_ptr<game_bot> bot,
                                      const destination&        destination,
                                      const detour&             detour)
{
    constexpr auto CONFIRM  = game_resp::move_confirm<fb::protocol::CLIENT_VERSION::v550>::opcode;
    constexpr auto POSITION = game_resp::position::opcode;
    constexpr auto MAP      = game_resp::map_config<fb::protocol::CLIENT_VERSION::v550>::opcode;
    constexpr auto WORLDS   = game_resp::map_worlds::opcode;
    constexpr auto OPTIONS  = game_resp::option<fb::protocol::CLIENT_VERSION::v550>::opcode;

    auto& atlas     = atlas::get();
    auto  visited   = std::unordered_set<uint16_t>{};
    auto  corrected = std::unordered_set<uint64_t>{};
    auto  spent     = std::unordered_set<size_t>{};
    auto  checked   = std::optional<uint16_t>{};
    if (bot->fast_move() != 0)
    {
        try
        {
            std::ignore = co_await bot->first_of(
                {OPTIONS},
                *bot->make<game_reqs::update_option>(fb::model::enum_value::OPTION::FAST_MOVE, false),
                STEP_TIMEOUT);
        }
        catch (const std::exception&)
        { }
    }
    if (bot->fast_move() != 0)
        co_return event{"FAST_MOVE", bot->map(), bot->position()};

    auto occupant = [&bot, &atlas](uint16_t map, const atlas::position& at, const atlas::position& target) {
        auto result = std::optional<event>{};
        for (auto& [oid, entry] : bot->sight.entries())
        {
            if (entry.position != at || entry.look >= game_bot::sight::ITEM_LOOK)
                continue;

            auto spawns = fb::model::table::npc_spawn->find(map);
            auto npc    = spawns != nullptr && std::any_of(spawns->begin(), spawns->end(), [&at](const auto& spawn) {
                           return spawn.position == at;
                       });
            auto kind   = atlas.mob(map, entry.look, entry.color).has_value() ? "MOB" : npc ? "NPC" : "CHARACTER";
            result      = event{"OCCUPIED", map, at, kind, oid, target};
            break;
        }
        return result;
    };

    auto waiting = clock::now();
    auto sent    = [&waiting]() {
        waiting = clock::now();
    };
    auto answered = [this, &bot, &waiting]() {
        this->wait(bot->id, std::chrono::duration_cast<std::chrono::milliseconds>(clock::now() - waiting));
    };

    while (true)
    {
        auto map  = bot->map();
        auto here = bot->position();
        if (this->active(bot->id) == false)
            co_return event{"INTERRUPTED", map, here};
        else if (this->stalled(bot->id))
            co_return event{"STALLED", map, here};

        auto level = bot->level();
        auto plan  = co_await this->offload<std::optional<atlas::plan>>(
            bot,
            [&atlas, map, here, &destination, level, &detour]() {
                return atlas.route(map, here, destination.maps, level, detour.closed);
            });
        if (plan.has_value() == false)
            co_return event{"NO_ROUTE", map, here};

        auto final = plan->hops.empty();
        if (final && destination.at.has_value() == false)
            co_return event{"ARRIVED", map, here};

        auto target   = final ? destination.at.value() : plan->hops.front().before;
        auto range    = final ? destination.range : uint16_t{0};
        auto distance = std::abs(here.x - target.x) + std::abs(here.y - target.y);
        if (final && distance <= range)
            co_return event{"ARRIVED", map, here};

        if (checked != map && detour.jumps.empty() == false)
        {
            checked        = map;
            auto teleports = std::vector<atlas::teleport>{};
            for (auto index = size_t{0}; index < detour.jumps.size(); index++)
            {
                teleports.push_back(spent.contains(index) ? atlas::teleport{} : detour.jumps[index].teleport);
            }

            auto leap = co_await this->offload<std::optional<atlas::leap>>(
                bot,
                [&atlas, map, here, &destination, level, &detour, teleports]() {
                    return atlas.shortcut(map, here, destination.maps, destination.at, level, detour.closed, teleports);
                });
            if (leap.has_value())
            {
                auto& jump   = detour.jumps[leap->index];
                auto  input  = jump.inputs.contains(leap->side) ? jump.inputs.at(leap->side) : std::string{};
                auto  spell  = bot->get_spell(bot->get_spell_slot_by_name(jump.spell));
                auto  slot   = jump.spell.empty() ? bot->get_item_slot_by_name(jump.item)
                                                  : bot->get_spell_slot_by_name(jump.spell);
                auto  usable = jump.spell.empty() ? bot->has_item(slot) : spell.has_value();
                auto  landed = std::pair<uint8_t, std::shared_ptr<fb::protocol::header>>{};
                if (usable)
                {
                    sent();
                    this->jumping(bot->id, jump.spell.empty() ? jump.item : jump.spell);
                    try
                    {
                        if (jump.spell.empty())
                            landed = co_await bot->first_of({MAP, POSITION},
                                                            *bot->make<game_reqs::item_active>(slot),
                                                            STEP_TIMEOUT);
                        else
                            landed =
                                co_await bot->first_of({MAP, POSITION},
                                                       *bot->make<game_reqs::spell_cast>(
                                                           static_cast<fb::model::enum_value::SPELL_TYPE>(spell->type),
                                                           slot,
                                                           input,
                                                           0u,
                                                           here),
                                                       STEP_TIMEOUT);
                    }
                    catch (const std::exception&)
                    {
                        if (bot->aborted())
                            co_return event{"TRANSFERRED", map, here, jump.spell.empty() ? jump.item : jump.spell};
                    }
                }

                if (landed.first == MAP)
                {
                    try
                    {
                        std::ignore = co_await bot->request_by_opcode(
                            POSITION,
                            nullptr,
                            [](const fb::protocol::header&) {
                                return true;
                            },
                            STEP_TIMEOUT);
                    }
                    catch (const std::exception&)
                    {
                        if (bot->aborted())
                            co_return event{"TRANSFERRED", map, here, jump.spell.empty() ? jump.item : jump.spell};
                    }
                }

                if (usable)
                    answered();
                if (landed.first != 0)
                {
                    this->progress(bot->id);
                    co_return event{"JUMPED", bot->map(), bot->position(), jump.spell.empty() ? jump.item : jump.spell};
                }

                spent.insert(leap->index);
                continue;
            }
        }

        auto occupied = std::unordered_set<uint64_t>{};
        for (auto& [_, entry] : bot->sight.entries())
        {
            if (entry.look < game_bot::sight::ITEM_LOOK)
                occupied.insert(atlas::gate(map, entry.position));
        }
        auto directions = atlas.path(map, here, target, range, [&](const atlas::position& at) {
            auto key = atlas::gate(map, at);
            return occupied.contains(key) || corrected.contains(key) || detour.avoid.contains(key);
        });
        if (directions.has_value() == false || directions->empty())
        {
            auto open = atlas.path(map, here, target, range, [&](const atlas::position& at) {
                auto key = atlas::gate(map, at);
                return corrected.contains(key) || detour.avoid.contains(key);
            });
            auto at   = here;
            for (auto direction : open.value_or(std::vector<atlas::DIRECTION>{}))
            {
                at = atlas::forward(at, direction);
                if (occupied.contains(atlas::gate(map, at)))
                    break;
            }

            auto blocker = at != here ? occupant(map, at, target) : std::nullopt;
            co_return blocker.value_or(event{"BLOCKED", map, target});
        }

        auto direction = directions->front();
        auto next      = atlas::forward(here, direction);
        auto gate      = final == false && next == target;
        if (next == target && occupied.contains(atlas::gate(map, next)))
        {
            auto blocker = occupant(map, next, target);
            if (blocker.has_value())
                co_return blocker.value();
        }
        sent();
        if (detour.rush && gate == false)
        {
            bot->send(*bot->make<game_reqs::move>(direction, 0, here));
            bot->set_position(next);
            this->progress(bot->id);
            co_await bot->thread()->sleep(RUSH_YIELD);
            continue;
        }

        auto step    = std::pair<uint8_t, std::shared_ptr<fb::protocol::header>>{};
        auto failure = std::string{};
        auto started = clock::now();
        try
        {
            step = co_await bot->first_of({CONFIRM, POSITION, MAP, WORLDS},
                                          *bot->make<game_reqs::move>(direction, 0, here),
                                          STEP_TIMEOUT);
        }
        catch (const std::exception&)
        {
            failure = bot->aborted() ? "TRANSFERRED" : this->active(bot->id) ? "TIMEOUT" : "INTERRUPTED";
        }

        if (failure.empty() && step.first == WORLDS)
        {
            auto& hop = plan->hops.front();
            if (gate && hop.world != atlas::NO_WORLD)
            {
                try
                {
                    step = co_await bot->first_of(
                        {MAP},
                        *bot->make<game_reqs::map_world>(hop.world, hop.world_from, hop.world_to),
                        STEP_TIMEOUT);
                }
                catch (const std::exception&)
                {
                    failure = bot->aborted() ? "TRANSFERRED" : this->active(bot->id) ? "TIMEOUT" : "INTERRUPTED";
                }
            }
            else
            {
                failure = "OFF_ROUTE";
            }
        }

        answered();
        if (failure.empty() == false)
        {
            co_return event{failure, map, next};
        }
        else if (step.first == CONFIRM)
        {
            bot->set_position(next);
            this->measure(bot->id,
                          "step",
                          std::chrono::duration_cast<std::chrono::milliseconds>(clock::now() - started));
            this->progress(bot->id);
        }
        else if (step.first == POSITION)
        {
            if (gate)
                co_return event{"DENIED", map, next};
            corrected.insert(atlas::gate(map, next));
        }
        else
        {
            if (visited.insert(map).second == false)
                co_return event{"LOOP", bot->map(), bot->position()};

            try
            {
                std::ignore = co_await bot->request_by_opcode(
                    POSITION,
                    nullptr,
                    [](const fb::protocol::header&) {
                        return true;
                    },
                    STEP_TIMEOUT);
            }
            catch (const std::exception&)
            {
                if (bot->aborted())
                    co_return event{"TRANSFERRED", map, next};
            }
            corrected.clear();
            this->measure(bot->id,
                          "hop",
                          std::chrono::duration_cast<std::chrono::milliseconds>(clock::now() - started));
            this->progress(bot->id);
        }

        if (detour.pace > 0ms)
            co_await bot->sleep(detour.pace);
    }
}
