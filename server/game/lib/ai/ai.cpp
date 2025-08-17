#include <fb/game/ai.h>
#include <fb/game/ai/containment.h>
#include <fb/game/ai/counter.h>
#include <fb/game/ai/run_away.h>
#include <fb/game/ai/no_move.h>
#include <fb/game/ai/none.h>
#include <fb/game/character.h>
#include <fb/game/map.h>

using namespace fb::game;

std::unique_ptr<ai> ai::create(MOB_ATTACK_TYPE attack_type)
{
    switch (attack_type)
    {
    case MOB_ATTACK_TYPE::CONTAINMENT:
        return std::make_unique<containment_ai>();

    case MOB_ATTACK_TYPE::COUNTER:
        return std::make_unique<counter_ai>();

    case MOB_ATTACK_TYPE::NO_MOVE:
        return std::make_unique<no_move_ai>();

    case MOB_ATTACK_TYPE::RUN_AWAY:
        return std::make_unique<run_away_ai>();

    case MOB_ATTACK_TYPE::NONE:
        return std::make_unique<none_ai>();

    default:
        return nullptr;
    }
}

bool ai::execute(mob& mob_obj, const datetime& now)
{
    auto owner = mob_obj.owner.lock();
    if (owner == nullptr || owner->map() != mob_obj.map())
        return false;

    auto target = mob_obj.target();
    if (target != nullptr)
    {
        DIRECTION direction;
        if (mob_obj.near_target(target, direction))
        {
            mob_obj.direction(direction);
            mob_obj.attack();
        }
        else
        {
            if (!mob_obj.move_step(target->position()))
                mob_obj.move(DIRECTION(std::rand() % 4)); // Random move if can't step towards owner
        }
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
            if (!mob_obj.move_step(owner->position()))
                mob_obj.move(DIRECTION(std::rand() % 4)); // Random move if can't step towards owner
        }
    }

    return owner != nullptr;
}

void ai::on_damage(mob& mob_obj, std::shared_ptr<life> attacker, const datetime& now)
{
    if (attacker == nullptr || this->should_ignore_attacker(mob_obj, attacker))
        return;

    this->record_damage(attacker, now);

    // If we don't have a target or our target lock expired,
    // immediately switch to this attacker
    if (mob_obj.target() == nullptr || !this->should_maintain_target(mob_obj, now))
    {
        mob_obj.target(attacker);
        this->_target_lock_time = now;
    }
}

std::shared_ptr<life> ai::find_target_in_sight(mob& mob_obj, const datetime& now)
{
    auto map = mob_obj.map();
    if (map == nullptr)
        return nullptr;

    // Skip targeting if mob has an owner
    auto owner = mob_obj.owner.lock();
    if (owner != nullptr)
        return nullptr;

    std::shared_ptr<life> best_target = nullptr;
    uint32_t              best_damage = 0;

    // First check recent attackers that are in sight
    for (const auto& [attacker, record] : this->_recent_damage)
    {
        if (!attacker->alive() || attacker->hidden(mob_obj) || !mob_obj.sight(*attacker))
            continue;

        // Select the attacker that dealt the most damage recently
        if (record.first > best_damage)
        {
            best_damage = record.first;
            best_target = attacker;
        }
    }

    // If we found a recent attacker in sight, use them
    if (best_target != nullptr)
        return best_target;

    // Otherwise check for any target in sight
    for (auto obj : mob_obj.sight_in(OBJECT_TYPE::CHARACTER))
    {
        auto potential_target = std::static_pointer_cast<life>(obj);

        if (potential_target.get() == mob_obj.oblivion().get())
            continue;
        if (!potential_target->alive())
            continue;
        if (potential_target->hidden(mob_obj))
            continue;

        // Take the first valid target in sight
        best_target = potential_target;
        break;
    }

    return best_target;
}

std::shared_ptr<life> ai::find_target_in_range(mob& mob_obj, const datetime& now)
{
    auto map = mob_obj.map();
    if (map == nullptr)
        return nullptr;

    // Skip targeting if mob has an owner
    auto owner = mob_obj.owner.lock();
    if (owner != nullptr)
        return nullptr;

    std::shared_ptr<life> best_target = nullptr;
    uint32_t              best_damage = 0;

    // First check recent attackers that are in range
    for (const auto& [attacker, record] : this->_recent_damage)
    {
        if (!attacker->alive() || attacker->hidden(mob_obj) || !mob_obj.sight(*attacker))
            continue;

        DIRECTION attack_dir;
        if (!mob_obj.near_target(attacker, attack_dir))
            continue; // Skip if not in attack range

        // Select the attacker that dealt the most damage recently
        if (record.first > best_damage)
        {
            best_damage = record.first;
            best_target = attacker;
        }
    }

    // If we found a recent attacker in range, use them
    if (best_target != nullptr)
        return best_target;

    // Otherwise check for any target in range
    for (auto obj : mob_obj.sight_in(OBJECT_TYPE::CHARACTER))
    {
        auto potential_target = std::static_pointer_cast<life>(obj);

        if (potential_target == mob_obj.oblivion())
            continue;
        if (!potential_target->alive())
            continue;
        if (potential_target->hidden(mob_obj))
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

void ai::cleanup_expired_damage(const datetime& now)
{
    for (auto it = this->_recent_damage.begin(); it != this->_recent_damage.end();)
    {
        if ((now - it->second.second) > DAMAGE_MEMORY_DURATION)
            it = this->_recent_damage.erase(it);
        else
            ++it;
    }
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
            // Check map PK settings
            auto map = mob_obj.map();
            if (map != nullptr && !ENUM_IN(map->model.option, MAP_OPTION::ENABLE_PK))
                return true;
        }
    }

    return false;
}

bool ai::should_maintain_target(const mob& mob_obj, const datetime& now) const
{
    auto target = mob_obj.target();
    if (target == nullptr)
        return false;

    // Keep current target for at least TARGET_LOCK_DURATION
    return (now - this->_target_lock_time) <= TARGET_LOCK_DURATION;
}

void ai::record_damage(std::shared_ptr<life> attacker, const datetime& now)
{
    auto& record   = this->_recent_damage[attacker];
    record.first  += 1; // Could be enhanced to track actual damage amount
    record.second  = now;
}

void ai::run_from_target(mob& mob_obj, std::shared_ptr<life> target)
{
    if (target == nullptr)
        return;

    auto repeat = std::rand() % 2;
    auto count  = repeat ? 2 : 1;

    for (int i = 0; i < count; i++)
    {
        // Calculate opposite direction from target
        auto target_pos = target->position();
        auto mob_pos    = mob_obj.position();

        DIRECTION run_dir;
        if (std::abs(target_pos.x - mob_pos.x) > std::abs(target_pos.y - mob_pos.y))
        {
            // Run horizontally
            run_dir = (target_pos.x > mob_pos.x) ? DIRECTION::LEFT : DIRECTION::RIGHT;
        }
        else
        {
            // Run vertically
            run_dir = (target_pos.y > mob_pos.y) ? DIRECTION::TOP : DIRECTION::BOTTOM;
        }

        // Move in chosen direction
        if (!mob_obj.move(run_dir))
            mob_obj.move(DIRECTION(std::rand() % 4));
    }
}