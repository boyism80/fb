#include <fb/game/ai/retaliate.h>
#include <fb/game/mob.h>

using namespace fb::game;

bool retaliate_ai::execute(mob& mob_obj, const datetime& now)
{
    if (super::execute(mob_obj, now))
        return true;

    super::cleanup_expired_damage(now);

    auto target = mob_obj.target();
    if (target == nullptr)
    {
        mob_obj.move(DIRECTION(std::rand() % 4));
        return true;
    }

    DIRECTION attack_dir;
    if (mob_obj.near_target(target, attack_dir))
    {
        mob_obj.direction(attack_dir);
        mob_obj.attack();
    }
    else
    {
        if (!mob_obj.move_step(target->position()))
            mob_obj.move(DIRECTION(std::rand() % 4));
    }

    return true;
}

MOB_ATTACK_TYPE retaliate_ai::get_type() const
{
    return MOB_ATTACK_TYPE::RETALIATE;
}
