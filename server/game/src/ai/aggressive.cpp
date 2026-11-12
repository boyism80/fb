#include <fb/game/ai/aggressive.h>

#include <fb/game/mob.h>
#include <random.h>

#include <cstdlib>

using namespace fb::game;

async::task<bool> aggressive_ai::execute(mob& mob_obj, const datetime& now)
{
    if (co_await super::execute(mob_obj, now))
        co_return true;

    auto target = super::select_target(mob_obj, now);
    if (target == nullptr)
    {
        co_await mob_obj.move(DIRECTION(random(0, 3)));
        co_return true;
    }

    DIRECTION attack_dir;
    if (mob_obj.near_target(target, attack_dir))
    {
        mob_obj.direction(attack_dir);
        co_await mob_obj.attack(DURATION::ATTACK, target);
        co_return true;
    }

    if (co_await mob_obj.move_step(target->position()))
        co_return true;

    auto adjacent = super::find_target_in_range(mob_obj, now);
    if (adjacent != nullptr && mob_obj.near_target(adjacent, attack_dir))
    {
        mob_obj.direction(attack_dir);
        co_await mob_obj.attack(DURATION::ATTACK, adjacent);
    }
    else
    {
        co_await mob_obj.move(DIRECTION(random(0, 3)));
    }

    co_return true;
}

std::shared_ptr<life> aggressive_ai::find_target(mob& mob_obj, const datetime& now)
{
    return super::find_target_in_sight(mob_obj, now);
}

MOB_ATTACK_TYPE aggressive_ai::get_type() const
{
    return MOB_ATTACK_TYPE::AGGRESSIVE;
}
