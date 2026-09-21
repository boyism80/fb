#include <fb/game/ai/retaliate.h>
#include <fb/game/mob.h>

using namespace fb::game;

async::task<bool> retaliate_ai::execute(mob& mob_obj, const datetime& now)
{
    if (co_await super::execute(mob_obj, now))
        co_return true;

    super::cleanup_expired_damage(now);

    auto target = mob_obj.target();
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

MOB_ATTACK_TYPE retaliate_ai::get_type() const
{
    return MOB_ATTACK_TYPE::RETALIATE;
}
