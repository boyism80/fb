#include <fb/game/ai/none.h>
#include <fb/game/mob.h>

using namespace fb::game;

bool none_ai::execute(mob& mob_obj, const datetime& now)
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
        // Clear target and move randomly when no target
        mob_obj.target(nullptr);
        mob_obj.move(DIRECTION(std::rand() % 4));
        return true;
    }

    // Run away from target
    super::run_from_target(mob_obj, target);

    return true;
}

MOB_ATTACK_TYPE none_ai::get_type() const
{
    return MOB_ATTACK_TYPE::NONE;
}