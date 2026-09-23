#include <fb/game/ai/aggressive.h>
#include <fb/game/mob.h>

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
        {
            mob_obj.target(target);
            this->_target_lock_time = now;
        }
    }

    if (target == nullptr)
    {
        co_await mob_obj.move(DIRECTION(std::rand() % 4));
        co_return true;
    }

    DIRECTION attack_dir;
    if (mob_obj.near_target(target, attack_dir))
    {
        mob_obj.direction(attack_dir);
        co_await mob_obj.attack();
    }
    else
    {
        if (co_await mob_obj.move_step(target->position()) == false)
            co_await mob_obj.move(DIRECTION(std::rand() % 4));
    }

    co_return true;
}

MOB_ATTACK_TYPE aggressive_ai::get_type() const
{
    return MOB_ATTACK_TYPE::AGGRESSIVE;
}
