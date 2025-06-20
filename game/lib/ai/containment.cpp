#include <fb/game/ai/containment.h>
#include <fb/game/mob.h>
#include <fb/game/map.h>
#include <fb/game/character.h>

using namespace fb::game;
using namespace fb::model::enum_value;

bool containment_ai::execute(mob& mob_obj, const datetime& now)
{
    if (super::execute(mob_obj, now))
        return true;

    super::cleanup_expired_damage(now);

    auto target = mob_obj.target();
    if (target == nullptr || !super::should_maintain_target(mob_obj, now))
    {
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

    // If target is in range, attack
    DIRECTION attack_dir;
    if (mob_obj.near_target(*target, attack_dir))
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

MOB_ATTACK_TYPE containment_ai::get_type() const
{
    return MOB_ATTACK_TYPE::CONTAINMENT;
}