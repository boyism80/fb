#include <fb/game/ai/aggressive.h>

#include <fb/game/mob.h>
#include <random.h>

#include <cstdlib>

using namespace fb::game;

async::task<bool> aggressive_ai::execute(mob& mob_obj, const datetime& now)
{
    if (co_await super::execute(mob_obj, now))
        co_return true;

    super::cleanup_expired_damage(now);

    auto target = mob_obj.target();
    if (target == nullptr || !super::should_maintain_target(mob_obj, now))
    {
        target = super::find_target_in_sight(mob_obj, now);
        if (target != nullptr)
            super::lock_target(mob_obj, target, now);
    }

    if (target == nullptr)
    {
        co_await mob_obj.move(DIRECTION(random(0, 3)));
        co_return true;
    }

    DIRECTION attack_dir;
    if (mob_obj.near_target(target, attack_dir))
    {
        mob_obj.direction(attack_dir);
        co_await mob_obj.attack();
        co_return true;
    }

    if (co_await mob_obj.move_step(target->position()))
        co_return true;

    auto adjacent = super::find_target_in_range(mob_obj, now);
    if (adjacent != nullptr && mob_obj.near_target(adjacent, attack_dir))
    {
        mob_obj.direction(attack_dir);
        co_await mob_obj.attack();
    }
    else
    {
        co_await mob_obj.move(DIRECTION(random(0, 3)));
    }

    co_return true;
}

MOB_ATTACK_TYPE aggressive_ai::get_type() const
{
    return MOB_ATTACK_TYPE::AGGRESSIVE;
}
