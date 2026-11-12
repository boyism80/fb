#include <fb/game/ai.h>

#include <fb/game/ai/aggressive.h>
#include <fb/game/ai/inert.h>
#include <fb/game/ai/none.h>
#include <fb/game/ai/retaliate.h>
#include <fb/game/ai/run_away.h>
#include <fb/game/ai/stationary.h>
#include <fb/game/character.h>
#include <fb/game/clan.h>
#include <fb/game/life.h>
#include <fb/game/map.h>
#include <fb/game/server.h>
#include <random.h>

#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <limits>
#include <memory>

using namespace fb::game;

std::unique_ptr<ai> ai::create(MOB_ATTACK_TYPE attack_type)
{
    switch (attack_type)
    {
    case MOB_ATTACK_TYPE::AGGRESSIVE:
        return std::make_unique<aggressive_ai>();

    case MOB_ATTACK_TYPE::RETALIATE:
        return std::make_unique<retaliate_ai>();

    case MOB_ATTACK_TYPE::STATIONARY:
        return std::make_unique<stationary_ai>();

    case MOB_ATTACK_TYPE::RUN_AWAY:
        return std::make_unique<run_away_ai>();

    case MOB_ATTACK_TYPE::NONE:
        return std::make_unique<none_ai>();

    case MOB_ATTACK_TYPE::INERT:
        return std::make_unique<inert_ai>();

    default:
        return nullptr;
    }
}

async::task<bool> ai::execute(mob& mob_obj, const datetime& now)
{
    if (mob_obj.type() != mob::spawn_type::SUMMONED)
        co_return false;

    auto owner = mob_obj.owner.lock();
    if (owner == nullptr || owner->map() != mob_obj.map() || !mob_obj.sight(*owner))
    {
        mob_obj.target(nullptr);
        co_return true;
    }

    auto target = mob_obj.target();
    if (target != nullptr)
    {
        co_await this->chase(mob_obj, target, now);
    }
    else
    {
        DIRECTION direction;
        if (mob_obj.near_target(owner, direction))
        {
            mob_obj.direction(direction);
        }
        else
        {
            if (co_await mob_obj.move_step(owner->position()) == false)
                co_await mob_obj.move(DIRECTION(random(0, 3))); // Random move if can't step towards owner
        }
    }

    co_return true;
}

void ai::on_damage(mob& mob_obj, std::shared_ptr<life> attacker, uint64_t amount, const datetime& now)
{
    if (attacker == nullptr || amount == 0 || this->should_ignore_attacker(mob_obj, attacker))
        return;

    this->_threats[attacker] += amount;
    this->forget_untargetable(mob_obj);
    if (this->should_maintain_target(mob_obj, now))
        return;

    mob_obj.target(this->retarget(mob_obj));
}

void ai::provoke(mob& mob_obj, std::shared_ptr<life> target, const datetime& now)
{
    mob_obj.target(target);
    this->_provoked = provocation{target, now};
}

uint64_t ai::threat(const std::shared_ptr<life>& attacker) const
{
    auto found = this->_threats.find(attacker);
    if (found == this->_threats.end())
        return 0;

    return found->second;
}

std::shared_ptr<life> ai::strongest_attacker(mob& mob_obj, bool adjacent) const
{
    auto                  current     = mob_obj.target();
    std::shared_ptr<life> best_target = nullptr;
    uint64_t              best_threat = 0;
    for (const auto& [weak, amount] : this->_threats)
    {
        auto attacker = weak.lock();
        if (attacker == nullptr || mob_obj.targetable(*attacker) == false)
            continue;

        DIRECTION attack_dir;
        if (adjacent && !mob_obj.near_target(attacker, attack_dir))
            continue; // Skip if not in attack range

        if (best_target == nullptr || amount > best_threat || (amount == best_threat && attacker == current))
        {
            best_threat = amount;
            best_target = attacker;
        }
    }

    return best_target;
}

std::shared_ptr<life> ai::retarget(mob& mob_obj) const
{
    auto current = mob_obj.target();
    auto leader  = this->strongest_attacker(mob_obj, false);
    if (leader == nullptr)
        return current;

    if (current == nullptr || current == leader)
        return leader;

    DIRECTION direction;
    auto      percent = mob_obj.near_target(leader, direction) ? MELEE_SWITCH_PERCENT : RANGED_SWITCH_PERCENT;
    if (this->threat(leader) * 100 > this->threat(current) * percent)
        return leader;

    return current;
}

std::shared_ptr<life> ai::find_target(mob&, const datetime&)
{
    return nullptr;
}

std::shared_ptr<life> ai::find_adjacent_target(mob& mob_obj, const datetime& now)
{
    return this->strongest_attacker(mob_obj, true);
}

std::shared_ptr<life> ai::select_target(mob& mob_obj, const datetime& now)
{
    this->forget_untargetable(mob_obj);
    if (this->should_maintain_target(mob_obj, now))
        return mob_obj.target();

    auto target = this->retarget(mob_obj);
    if (target == nullptr)
        target = this->find_target(mob_obj, now);

    mob_obj.target(target);
    return target;
}

async::task<bool> ai::attack_adjacent(mob& mob_obj, std::shared_ptr<life> target)
{
    DIRECTION direction;
    if (target == nullptr || mob_obj.near_target(target, direction) == false)
        co_return false;

    mob_obj.direction(direction);
    co_await mob_obj.attack(DURATION::ATTACK, target);
    co_return true;
}

async::task<void> ai::chase(mob& mob_obj, std::shared_ptr<life> target, const datetime& now)
{
    if (co_await this->attack_adjacent(mob_obj, target))
        co_return;

    if (co_await mob_obj.move_step(target->position()))
        co_return;

    if (co_await this->attack_adjacent(mob_obj, this->find_adjacent_target(mob_obj, now)))
        co_return;

    co_await mob_obj.move(DIRECTION(random(0, 3)));
}

std::shared_ptr<life> ai::find_target_in_sight(mob& mob_obj, const datetime& now)
{
    auto map = mob_obj.map();
    if (map == nullptr)
        return nullptr;

    if (mob_obj.type() == mob::spawn_type::SUMMONED)
        return nullptr;

    // First check attackers that are in sight
    auto best_target = this->strongest_attacker(mob_obj, false);

    // If we found an attacker in sight, use them
    if (best_target != nullptr)
        return best_target;

    // Otherwise check for any target in sight
    auto mob_pos       = mob_obj.position();
    auto best_distance = std::numeric_limits<int>::max();
    for (const auto& obj : mob_obj.sight_in(OBJECT_TYPE::CHARACTER))
    {
        auto potential_target = std::static_pointer_cast<life>(obj);
        if (mob_obj.targetable(*potential_target) == false)
            continue;

        auto target_pos = potential_target->position();
        auto dx         = std::abs(static_cast<int>(target_pos.x) - static_cast<int>(mob_pos.x));
        auto dy         = std::abs(static_cast<int>(target_pos.y) - static_cast<int>(mob_pos.y));
        auto distance   = dx + dy;
        if (distance < best_distance)
        {
            best_distance = distance;
            best_target   = potential_target;
        }
    }

    return best_target;
}

std::shared_ptr<life> ai::find_target_in_range(mob& mob_obj, const datetime& now)
{
    auto map = mob_obj.map();
    if (map == nullptr)
        return nullptr;

    // First check attackers that are in range
    auto best_target = this->strongest_attacker(mob_obj, true);

    // If we found an attacker in range, use them
    if (best_target != nullptr)
        return best_target;

    if (mob_obj.type() == mob::spawn_type::SUMMONED)
        return nullptr;

    // Otherwise check for any target in range
    for (const auto& obj : mob_obj.sight_in(OBJECT_TYPE::CHARACTER))
    {
        auto potential_target = std::static_pointer_cast<life>(obj);
        if (mob_obj.targetable(*potential_target) == false)
            continue;

        DIRECTION attack_dir;
        if (!mob_obj.near_target(potential_target, attack_dir))
            continue; // Skip if not in attack range

        // Take the first valid target in range
        best_target = potential_target;
        break;
    }

    return best_target;
}

void ai::forget_untargetable(mob& mob_obj)
{
    std::erase_if(this->_threats, [&mob_obj](const auto& entry) {
        auto attacker = entry.first.lock();
        return attacker == nullptr || mob_obj.targetable(*attacker) == false;
    });
}

bool ai::should_ignore_attacker(const mob& mob_obj, std::shared_ptr<life> attacker) const
{
    // Ignore null attackers
    if (attacker == nullptr)
        return true;

    // Ignore if attacker is our owner
    auto owner = mob_obj.owner.lock();
    if (owner != nullptr)
    {
        if (attacker == owner)
            return true;

        // Check if attacker is a character
        if (attacker->is(OBJECT_TYPE::CHARACTER))
        {
            // Check map PK settings (ENABLE_PK, or SIEGE_CASTLE while siege is active)
            auto map = mob_obj.map();
            if (map != nullptr && map->pk_enabled() == false)
                return true;

            auto& owner_clan    = owner->clan.id();
            auto& attacker_clan = static_cast<character&>(*attacker).clan.id();
            auto  siege         = map != nullptr && ENUM_IN(map->model().option, MAP_OPTION::SIEGE_CASTLE);
            if (siege && owner_clan.has_value() && attacker_clan.has_value())
            {
                if (owner_clan.value() == attacker_clan.value())
                    return true;

                auto guard = mob_obj.server.clans.try_enter_read(owner_clan.value());
                if (guard.has_value() && guard->value() != nullptr && guard->value()->is_allied(attacker_clan.value()))
                    return true;
            }
        }
    }

    return false;
}

bool ai::should_maintain_target(const mob& mob_obj, const datetime& now) const
{
    auto target = mob_obj.target();
    if (target == nullptr || !this->_provoked.has_value() || target != this->_provoked->first.lock())
        return false;

    return (now - this->_provoked->second) <= TARGET_LOCK_DURATION;
}

async::task<void> ai::run_from_target(mob& mob_obj, std::shared_ptr<life> target)
{
    if (target == nullptr)
        co_return;

    auto map = mob_obj.map();
    if (map == nullptr)
        co_return;

    auto repeat = random(0, 1);
    auto count  = repeat ? 2 : 1;

    constexpr DIRECTION all_dirs[] = {DIRECTION::LEFT, DIRECTION::TOP, DIRECTION::RIGHT, DIRECTION::BOTTOM};

    for (int step = 0; step < count; step++)
    {
        const auto mob_pos     = mob_obj.position();
        const auto target_pos  = target->position();
        const int  dist_before = std::abs(static_cast<int>(mob_pos.x) - static_cast<int>(target_pos.x)) +
                                std::abs(static_cast<int>(mob_pos.y) - static_cast<int>(target_pos.y));

        bool moved = false;

        // tier 0: move away (+1 manhattan), tier 1: lateral (0), tier 2: toward (-1, last resort)
        for (int tier = 0; tier < 3 && !moved; tier++)
        {
            const int want_delta = (tier == 0) ? 1 : (tier == 1 ? 0 : -1);

            for (auto dir : all_dirs)
            {
                if (!map->movable(mob_obj, mob_obj.position(), dir))
                    continue;

                const auto next       = mob_obj.side_position(dir);
                const int  dist_after = std::abs(static_cast<int>(next.x) - static_cast<int>(target_pos.x)) +
                                       std::abs(static_cast<int>(next.y) - static_cast<int>(target_pos.y));
                const int delta = dist_after - dist_before;

                if (delta != want_delta)
                    continue;

                if (co_await mob_obj.move(dir))
                {
                    moved = true;
                    break;
                }
            }
        }
    }
}
