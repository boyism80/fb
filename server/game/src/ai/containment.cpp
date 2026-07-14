#include <fb/game/ai/containment.h>
#include <fb/game/mob.h>
#include <fb/game/map.h>
#include <fb/game/character.h>
#include <tuple>

using namespace fb::game;
using namespace fb::model::enum_value;

async::task<bool> containment_ai::execute(mob& mob_obj, const datetime& now)
{
    if (co_await super::execute(mob_obj, now))
        co_return true;

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

MOB_ATTACK_TYPE containment_ai::get_type() const
{
    return MOB_ATTACK_TYPE::CONTAINMENT;
}
