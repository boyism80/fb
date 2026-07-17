#include <fb/game/ai/no_move.h>
#include <fb/game/character.h>
#include <fb/game/map.h>

using namespace fb::game;

bool no_move_ai::execute(mob& mob_obj, const datetime& now)
{
    // NO_MOVE never follows the owner or walks toward targets
    super::cleanup_expired_damage(now);

    // Check if current target is still valid
    auto target = mob_obj.target();
    if (target == nullptr || !super::should_maintain_target(mob_obj, now))
    {
        // Try to find a new target in attack range
        target = super::find_target_in_range(mob_obj, now);
        if (target != nullptr)
        {
            mob_obj.target(target);
            this->_target_lock_time = now;
        }
    }

    // No movement if no target, just stay in place
    if (target == nullptr)
        return true;

    // If target is in range, attack without moving
    DIRECTION attack_dir;
    if (mob_obj.near_target(target, attack_dir))
    {
        mob_obj.direction(attack_dir);
        mob_obj.attack();
    }

    return true;
}

MOB_ATTACK_TYPE no_move_ai::get_type() const
{
    return MOB_ATTACK_TYPE::NO_MOVE;
}