#include <fb/game/ai/counter.h>
#include <fb/game/mob.h>
#include <fb/game/map.h>

using namespace fb::game;

bool counter_ai::execute(mob& mob_obj, const datetime& now)
{
    // Try owner following first
    if (super::execute(mob_obj, now))
        return true;

    // Clean up expired damage records
    super::cleanup_expired_damage(now);

    // If no target, move randomly
    auto target = mob_obj.target();
    if (target == nullptr)
    {
        mob_obj.move(DIRECTION(std::rand() % 4));
        return true;
    }

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

    return true;
}

MOB_ATTACK_TYPE counter_ai::get_type() const
{
    return MOB_ATTACK_TYPE::COUNTER;
}