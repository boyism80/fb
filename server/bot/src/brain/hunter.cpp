#include <fb/bot/brain/brain.h>

#include <fb/bot/game_controller.h>

#include <chrono>
#include <cstdlib>
#include <exception>
#include <optional>
#include <string>

using namespace fb::bot;
using namespace std::chrono_literals;

namespace game_reqs = fb::protocol::game::request;
namespace game_resp = fb::protocol::game::response;

constexpr auto SWING_TIMEOUT = 5s;

async::task<brain::event> brain::engage(std::shared_ptr<game_bot> bot, uint32_t oid, const maneuver& maneuver)
{
    using DIRECTION = fb::model::enum_value::DIRECTION;

    auto swings = uint32_t{0};
    auto map    = bot->map();
    auto here   = bot->position();
    auto hp     = bot->hp();
    auto target = atlas::position{};
    auto check  = [this, &bot, oid, &map, &here, &target, &swings]() -> std::optional<event> {
        map        = bot->map();
        here       = bot->position();
        auto found = bot->sight.entries().find(oid);
        if (this->active(bot->id) == false)
            return event{"INTERRUPTED", map, here};
        else if (bot->sight.dead(oid))
            return event{"KILLED", map, here, {}, oid, target, swings};
        else if (found == bot->sight.entries().end())
            return event{"LOST", map, here};

        target = found->second.position;
        return std::nullopt;
    };
    auto face = [&bot, &here, &target]() {
        auto dx        = target.x - here.x;
        auto dy        = target.y - here.y;
        auto direction = dx < 0   ? DIRECTION::LEFT
                         : dx > 0 ? DIRECTION::RIGHT
                         : dy < 0 ? DIRECTION::TOP
                                  : DIRECTION::BOTTOM;
        if (bot->direction() != direction)
        {
            bot->send(*bot->make<game_reqs::direction>(direction));
            bot->set_direction(direction);
        }
    };

    if (auto ended = check(); ended.has_value())
        co_return ended.value();

    if (maneuver.kind == MANEUVER::CAST)
    {
        auto learned = bot->get_spell(maneuver.slot);
        if (learned.has_value() == false)
            co_return event{"REFUSED", map, here};

        auto mp      = bot->mp();
        auto casting = clock::now();
        if (maneuver.self == false)
            face();
        bot->send(*bot->make<game_reqs::spell_cast>(static_cast<fb::model::enum_value::SPELL_TYPE>(learned->type),
                                                    maneuver.slot,
                                                    std::string{},
                                                    maneuver.self ? bot->oid() : oid,
                                                    maneuver.self ? here : target));
        this->act(bot->id, "cast");
        this->casting(bot->id);
        this->progress(bot->id);
        std::ignore = co_await this->synced(bot);
        this->measure(bot->id, "cast", std::chrono::duration_cast<std::chrono::milliseconds>(clock::now() - casting));
        if (auto ended = check(); ended.has_value())
            co_return ended.value();
        co_return event{bot->mp() < mp || maneuver.cost == 0 ? "DONE" : "REFUSED", map, here};
    }
    else if (maneuver.kind == MANEUVER::USE)
    {
        auto slot = bot->get_item_slot_by_name(maneuver.item);
        if (bot->has_item(slot) == false)
            co_return event{"REFUSED", map, here};

        bot->send(*bot->make<game_reqs::item_active>(slot));
        this->act(bot->id, "potion");
        this->progress(bot->id);
        std::ignore = co_await this->synced(bot);
        if (auto ended = check(); ended.has_value())
            co_return ended.value();
        co_return event{"DONE", map, here};
    }

    while (true)
    {
        if (auto ended = check(); ended.has_value())
            co_return ended.value();

        if (std::abs(target.x - here.x) + std::abs(target.y - here.y) != 1)
        {
            auto moved = co_await this->walk(bot, destination{{map}, target, 1}, detour{});
            if (moved.type == "BLOCKED" || moved.type == "NO_ROUTE")
                co_return event{"UNREACHABLE", map, target};
            else if (moved.type != "ARRIVED")
                co_return moved;
        }
        else
        {
            face();
            auto swing   = bot->make<game_reqs::attack>();
            auto failure = std::string{};
            auto started = clock::now();
            try
            {
                std::ignore = co_await bot->request_by_opcode(
                    game_resp::action::opcode,
                    swing.get(),
                    [me = bot->oid()](const fb::protocol::header& header) {
                        return static_cast<const game_resp::action&>(header).oid == me;
                    },
                    SWING_TIMEOUT);
            }
            catch (const std::exception&)
            {
                failure = bot->aborted() ? "TRANSFERRED" : this->active(bot->id) ? "TIMEOUT" : "INTERRUPTED";
            }
            if (failure.empty() == false)
                co_return event{failure, map, here};

            swings++;
            this->act(bot->id, "swing");
            this->measure(bot->id,
                          "swing",
                          std::chrono::duration_cast<std::chrono::milliseconds>(clock::now() - started));
            this->progress(bot->id);
        }

        if (bot->hp() < hp)
            co_return event{"STRUCK", bot->map(), bot->position(), {}, oid, target, swings};
    }
}
