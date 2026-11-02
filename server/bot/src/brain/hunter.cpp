#include <fb/bot/brain/brain.h>

#include <fb/bot/game_controller.h>

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <exception>
#include <functional>
#include <optional>
#include <string>

using namespace fb::bot;
using namespace std::chrono_literals;

namespace game_reqs = fb::protocol::game::request;
namespace game_resp = fb::protocol::game::response;

constexpr auto SWING_TIMEOUT = 5s;

namespace {

enum class STATUS
{
    SUCCESS,
    FAILURE,
    RUNNING,
};

}

async::task<brain::event> brain::attack(std::shared_ptr<game_bot> bot, uint32_t oid, const tactic& tactic)
{
    using DIRECTION = fb::model::enum_value::DIRECTION;

    if (tactic.tree.has_value() == false)
        co_return event{"NO_PLAN", bot->map(), bot->position()};

    auto swings = uint32_t{0};
    auto opened = false;
    auto last   = atlas::position{};
    auto map    = bot->map();
    auto here   = bot->position();
    auto target = atlas::position{};
    auto result = std::optional<event>{};
    auto cast   = [this, &bot](const skill& skill, uint32_t target, const atlas::position& at) -> async::task<void> {
        auto learned = bot->get_spell(skill.slot);
        if (learned.has_value())
        {
            bot->send(*bot->make<game_reqs::spell_cast>(static_cast<fb::model::enum_value::SPELL_TYPE>(learned->type),
                                                        skill.slot,
                                                        std::string{},
                                                        target,
                                                        at));
            this->act(bot->id, "cast");
            this->progress(bot->id);
        }
        std::ignore = co_await this->synced(bot);
    };
    auto use = [this, &bot](const std::vector<std::string>& names) -> async::task<bool> {
        for (auto& name : names)
        {
            auto slot = bot->get_item_slot_by_name(name);
            if (bot->has_item(slot))
            {
                bot->send(*bot->make<game_reqs::item_active>(slot));
                this->act(bot->id, "potion");
                this->progress(bot->id);
                std::ignore = co_await this->synced(bot);
                co_return true;
            }
        }
        co_return false;
    };
    auto leaf = [&](NODE kind) -> async::task<STATUS> {
        switch (kind)
        {
        case NODE::CHECK:
        {
            auto found = bot->sight.entries().find(oid);
            if (this->active(bot->id) == false)
                result = event{"INTERRUPTED", map, here};
            else if (bot->sight.dead(oid))
                result = event{"KILLED", map, here, {}, oid, last, swings};
            else if (found == bot->sight.entries().end())
                result = event{"LOST", map, here};
            else
            {
                target = found->second.position;
                last   = target;
            }
            co_return STATUS::FAILURE;
        }
        case NODE::HEAL:
            if (bot->hp() <= tactic.mend && tactic.heal.has_value() && bot->mp() >= tactic.heal->mp)
            {
                co_await cast(tactic.heal.value(), bot->oid(), here);
                co_return STATUS::SUCCESS;
            }
            co_return STATUS::FAILURE;
        case NODE::POTION:
            if (bot->hp() <= tactic.mend && co_await use(tactic.potions))
                co_return STATUS::SUCCESS;
            co_return STATUS::FAILURE;
        case NODE::HURT:
            if (bot->hp() <= tactic.guard)
                result = event{"HURT", map, here, {}, oid, {}, swings};
            co_return STATUS::FAILURE;
        case NODE::BUFF:
            for (auto& buff : tactic.buffs)
            {
                if (bot->has_buff(buff.name) == false && bot->mp() >= buff.mp && bot->get_spell(buff.slot).has_value())
                {
                    co_await cast(buff, bot->oid(), here);
                    co_return STATUS::SUCCESS;
                }
            }
            co_return STATUS::FAILURE;
        case NODE::SPELL:
        {
            auto ready = std::find_if(tactic.skills.begin(), tactic.skills.end(), [&bot](const skill& skill) {
                return bot->mp() >= skill.mp && bot->get_spell(skill.slot).has_value();
            });
            if (ready == tactic.skills.end())
                co_return STATUS::FAILURE;
            co_await cast(*ready, oid, target);
            swings++;
            co_return STATUS::SUCCESS;
        }
        case NODE::MANA:
            if (tactic.melee)
                co_return STATUS::FAILURE;
            if (co_await use(tactic.drinks))
                co_return STATUS::SUCCESS;
            result = event{"NO_MP", map, here, {}, oid, {}, swings};
            co_return STATUS::FAILURE;
        case NODE::APPROACH:
        {
            if (std::abs(target.x - here.x) + std::abs(target.y - here.y) == 1)
                co_return STATUS::SUCCESS;
            auto moved = co_await this->walk(bot, destination{{map}, target, 1}, detour{});
            if (moved.type == "BLOCKED" || moved.type == "NO_ROUTE")
                result = event{"UNREACHABLE", map, target};
            else if (moved.type != "ARRIVED")
                result = moved;
            co_return STATUS::RUNNING;
        }
        case NODE::FACE:
        {
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
            co_return STATUS::SUCCESS;
        }
        case NODE::OPENER:
            if (opened || tactic.opener.has_value() == false || bot->mp() < tactic.opener->mp ||
                bot->hp() <= tactic.opener->floor)
                co_return STATUS::FAILURE;
            opened = true;
            co_await cast(tactic.opener.value(), oid, target);
            swings++;
            co_return STATUS::SUCCESS;
        case NODE::SWING:
        {
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
            {
                result = event{failure, map, here};
                co_return STATUS::FAILURE;
            }

            swings++;
            this->act(bot->id, "swing");
            this->measure(bot->id,
                          "swing",
                          std::chrono::duration_cast<std::chrono::milliseconds>(clock::now() - started));
            this->progress(bot->id);
            co_return STATUS::SUCCESS;
        }
        default:
            co_return STATUS::FAILURE;
        }
    };
    auto run = std::function<async::task<STATUS>(const node&)>{};
    run      = [&](const node& current) -> async::task<STATUS> {
        if (current.kind != NODE::SELECTOR && current.kind != NODE::SEQUENCE)
            co_return co_await leaf(current.kind);

        for (auto& child : current.children)
        {
            auto status = co_await run(child);
            if (result.has_value())
                co_return STATUS::FAILURE;
            if (current.kind == NODE::SELECTOR && status != STATUS::FAILURE)
                co_return status;
            if (current.kind == NODE::SEQUENCE && status != STATUS::SUCCESS)
                co_return status;
        }
        co_return current.kind == NODE::SELECTOR ? STATUS::FAILURE : STATUS::SUCCESS;
    };

    while (true)
    {
        map         = bot->map();
        here        = bot->position();
        auto status = co_await run(tactic.tree.value());
        if (result.has_value())
            co_return result.value();
        if (status == STATUS::FAILURE)
            co_return event{"IDLE", map, here, {}, oid, {}, swings};
    }
}
