#include <fb/game/ai/run_away.h>
#include <fb/game/mob.h>
#include <fb/game/map.h>

using namespace fb::game;

bool run_away_ai::execute(mob& mob_obj, const datetime& now)
{
    // Try owner following first
    if (super::execute(mob_obj, now))
        return true;

    // Clean up expired damage records
    super::cleanup_expired_damage(now);

    // Check if current target is still valid
    auto target = mob_obj.target();
    if (target == nullptr || !super::should_maintain_target(mob_obj, now))
    {
        // Try to find a new target in sight
        target = super::find_target_in_sight(mob_obj, now);
        if (target != nullptr)
        {
            mob_obj.target(target);
            this->_target_lock_time = now;
        }
    }

    if (target == nullptr)
    {
        mob_obj.move(DIRECTION(std::rand() % 4));
        return true;
    }

    // Run away if HP is low, otherwise act like containment AI
    auto hp_ratio = static_cast<float>(mob_obj.stat.hp()) / mob_obj.stat.base_hp();
    if (hp_ratio <= LOW_HP_THRESHOLD)
    {
        super::run_from_target(mob_obj, target);
    }
    else
    {
        // If target is in range, attack
        DIRECTION attack_dir;
        if (mob_obj.near_target(target, attack_dir))
        {
            mob_obj.direction(attack_dir);
            mob_obj.attack();
        }
        else
        {
            // Move towards target
            if (!mob_obj.move_step(target->position()))
                mob_obj.move(DIRECTION(std::rand() % 4));
        }
    }

    return true;
}

MOB_ATTACK_TYPE run_away_ai::get_type() const
{
    return MOB_ATTACK_TYPE::RUN_AWAY;
}