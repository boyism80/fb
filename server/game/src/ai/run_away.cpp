#include <fb/game/ai/run_away.h>

#include <fb/game/map.h>
#include <fb/game/mob.h>
#include <random.h>

#include <cstdlib>

using namespace fb::game;

async::task<bool> run_away_ai::execute(mob& mob_obj, const datetime& now)
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

    // Run away if HP is low, otherwise act like aggressive AI
    auto hp_ratio = static_cast<float>(mob_obj.stat.hp()) / mob_obj.stat.base_hp();
    if (hp_ratio <= LOW_HP_THRESHOLD)
    {
        co_await super::run_from_target(mob_obj, target);
    }
    else
    {
        co_await super::chase(mob_obj, target, now);
    }

    co_return true;
}

std::shared_ptr<life> run_away_ai::find_target(mob& mob_obj, const datetime& now)
{
    return super::find_target_in_sight(mob_obj, now);
}

MOB_ATTACK_TYPE run_away_ai::get_type() const
{
    return MOB_ATTACK_TYPE::RUN_AWAY;
}
