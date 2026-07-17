#include <fb/game/ai/stationary.h>
#include <fb/game/mob.h>

using namespace fb::game;

bool stationary_ai::execute(mob& mob_obj, const datetime& now)
{
    super::cleanup_expired_damage(now);

    auto target = mob_obj.target();
    if (target == nullptr || !super::should_maintain_target(mob_obj, now))
    {
        target = super::find_target_in_range(mob_obj, now);
        if (target != nullptr)
        {
            mob_obj.target(target);
            this->_target_lock_time = now;
        }
    }

    if (target == nullptr)
        return true;

    DIRECTION attack_dir;
    if (mob_obj.near_target(target, attack_dir))
    {
        mob_obj.direction(attack_dir);
        mob_obj.attack();
    }

    return true;
}

MOB_ATTACK_TYPE stationary_ai::get_type() const
{
    return MOB_ATTACK_TYPE::STATIONARY;
}
