#include <fb/game/ai/stationary.h>

#include <fb/game/mob.h>

using namespace fb::game;

async::task<bool> stationary_ai::execute(mob& mob_obj, const datetime& now)
{
    auto target = super::select_target(mob_obj, now);
    if (target == nullptr)
        co_return true;

    DIRECTION attack_dir;
    if (mob_obj.near_target(target, attack_dir))
    {
        mob_obj.direction(attack_dir);
        co_await mob_obj.attack(DURATION::ATTACK, target);
    }

    co_return true;
}

std::shared_ptr<life> stationary_ai::find_target(mob& mob_obj, const datetime& now)
{
    return super::find_target_in_range(mob_obj, now);
}

MOB_ATTACK_TYPE stationary_ai::get_type() const
{
    return MOB_ATTACK_TYPE::STATIONARY;
}
