#include <fb/game/ai/counter.h>
#include <fb/game/mob.h>
#include <fb/game/map.h>
#include <tuple>

using namespace fb::game;

async::task<bool> counter_ai::execute(mob& mob_obj, const datetime& now)
{
    // Try owner following first
    if (co_await super::execute(mob_obj, now))
        co_return true;

    // Clean up expired damage records
    super::cleanup_expired_damage(now);

    // If no target, move randomly
    auto target = mob_obj.target();
    if (target == nullptr)
    {
        std::ignore = co_await mob_obj.move(DIRECTION(std::rand() % 4));
        co_return true;
    }

    // If target is in range, attack
    DIRECTION attack_dir;
    if (mob_obj.near_target(target, attack_dir))
    {
        std::ignore = co_await mob_obj.direction(attack_dir);
        co_await mob_obj.attack();
    }
    else
    {
        // Move towards target
        if (co_await mob_obj.move_step(target->position()) == false)
            std::ignore = co_await mob_obj.move(DIRECTION(std::rand() % 4));
    }

    co_return true;
}

MOB_ATTACK_TYPE counter_ai::get_type() const
{
    return MOB_ATTACK_TYPE::COUNTER;
}
