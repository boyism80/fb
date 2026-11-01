#include <fb/bot/brain/brain.h>

#include <fb/bot/game_controller.h>

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <exception>
#include <string>

using namespace fb::bot;
using namespace std::chrono_literals;

namespace game_reqs = fb::protocol::game::request;
namespace game_resp = fb::protocol::game::response;

constexpr auto SWING_TIMEOUT = 5s;

async::task<brain::event> brain::attack(std::shared_ptr<game_bot> bot, uint32_t oid, const tactic& tactic)
{
    using DIRECTION = fb::model::enum_value::DIRECTION;

    auto swings = uint32_t{0};
    auto last   = atlas::position{};
    while (true)
    {
        auto map  = bot->map();
        auto here = bot->position();
        if (this->active(bot->id) == false)
            co_return event{"INTERRUPTED", map, here};

        if (bot->sight.dead(oid))
            co_return event{"KILLED", map, here, {}, oid, last, swings};

        auto found = bot->sight.entries().find(oid);
        if (found == bot->sight.entries().end())
            co_return event{"LOST", map, here};

        auto target = found->second.position;
        last        = target;
        if (bot->hp() <= tactic.guard)
            co_return event{"HURT", map, here, {}, oid, {}, swings};

        auto dx = target.x - here.x;
        auto dy = target.y - here.y;
        if (std::abs(dx) + std::abs(dy) != 1)
        {
            auto moved = co_await this->walk(bot, destination{{map}, target, 1}, detour{{}, {}, tactic.pace});
            if (moved.type == "BLOCKED" || moved.type == "NO_ROUTE")
                co_return event{"UNREACHABLE", map, target};
            else if (moved.type != "ARRIVED")
                co_return moved;
            continue;
        }

        auto direction = dx < 0   ? DIRECTION::LEFT
                         : dx > 0 ? DIRECTION::RIGHT
                         : dy < 0 ? DIRECTION::TOP
                                  : DIRECTION::BOTTOM;
        if (bot->direction() != direction)
        {
            bot->send(*bot->make<game_reqs::direction>(direction));
            bot->set_direction(direction);
        }

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
        this->measure(bot->id, "swing", std::chrono::duration_cast<std::chrono::milliseconds>(clock::now() - started));
        this->progress(bot->id);

        if (tactic.swing > 0ms)
            co_await bot->sleep(tactic.swing);
    }
}
