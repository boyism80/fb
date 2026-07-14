#include <fb/game/ai/run_away.h>
#include <fb/game/mob.h>
#include <fb/game/map.h>
#include <tuple>

using namespace fb::game;

async::task<bool> run_away_ai::execute(mob& mob_obj, const datetime& now)
{
    // Try owner following first
    if (co_await super::execute(mob_obj, now))
        co_return true;

    // Clean up expired damage records
    super::cleanup_expired_damage(now);

    // Check if current target is still valid
    auto target = mob_obj.target();
    if (target == nullptr || !super::should_maintain_target(mob_obj, now))
    {
        // Try to find a new target in sight
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

    // Run away if HP is low, otherwise act like containment AI
    auto hp_ratio = static_cast<float>(mob_obj.stat.hp()) / mob_obj.stat.base_hp();
    if (hp_ratio <= LOW_HP_THRESHOLD)
    {
        co_await super::run_from_target(mob_obj, target);
    }
    else
    {
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
    }

    co_return true;
}

MOB_ATTACK_TYPE run_away_ai::get_type() const
{
    return MOB_ATTACK_TYPE::RUN_AWAY;
}
