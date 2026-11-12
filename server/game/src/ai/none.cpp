#include <fb/game/ai/none.h>

#include <fb/game/mob.h>
#include <random.h>

#include <cstdlib>

using namespace fb::game;

async::task<bool> none_ai::execute(mob& mob_obj, const datetime& now)
{
    // Try owner following first
    if (co_await super::execute(mob_obj, now))
        co_return true;

    auto target = super::select_target(mob_obj, now);
    if (target == nullptr)
    {
        co_await mob_obj.move(DIRECTION(random(0, 3)));
        co_return true;
    }

    // Run away from target
    co_await super::run_from_target(mob_obj, target);

    co_return true;
}

MOB_ATTACK_TYPE none_ai::get_type() const
{
    return MOB_ATTACK_TYPE::NONE;
}
