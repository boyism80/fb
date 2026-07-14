#include <fb/game/ai/no_move.h>
#include <fb/game/character.h>
#include <fb/game/map.h>
#include <tuple>

using namespace fb::game;

async::task<bool> no_move_ai::execute(mob& mob_obj, const datetime& now)
{
    // Try owner following first
    if (co_await super::execute(mob_obj, now))
        co_return true;

    // Clean up expired damage records
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
        co_return true;

    // If target is in range, attack without moving
    DIRECTION attack_dir;
    if (mob_obj.near_target(target, attack_dir))
    {
        std::ignore = co_await mob_obj.direction(attack_dir);
        co_await mob_obj.attack();
    }

    co_return true;
}

MOB_ATTACK_TYPE no_move_ai::get_type() const
{
    return MOB_ATTACK_TYPE::NO_MOVE;
}
