#ifndef __AI_H__
#define __AI_H__

#include <fb/model/model.h>
#include <async/task.h>
#include <unordered_map>

namespace fb::game {

using namespace std::chrono_literals;
using namespace fb::model::enum_value;

class mob;
class life;
class character;

class ai
{
public:
    static constexpr auto DAMAGE_MEMORY_DURATION = std::chrono::seconds(10);
    static constexpr auto TARGET_LOCK_DURATION   = std::chrono::seconds(5);

private:
    using datetime      = fb::model::datetime;
    using damage_record = std::pair<uint32_t, datetime>;
    using damage_map    = std::unordered_map<std::shared_ptr<life>, damage_record>;

private:
    damage_map _recent_damage;
    datetime   _target_lock_time;

public:
    virtual ~ai() = default;
    virtual async::task<bool>  execute(mob& mob_obj, const datetime& now);
    virtual MOB_ATTACK_TYPE    get_type() const = 0;
    virtual void               on_damage(mob& mob_obj, std::shared_ptr<life> attacker, const datetime& now);
    static std::unique_ptr<ai> create(MOB_ATTACK_TYPE attack_type);

protected:
    std::shared_ptr<life> find_target_in_sight(mob& mob_obj, const datetime& now);
    std::shared_ptr<life> find_target_in_range(mob& mob_obj, const datetime& now);
    void                  cleanup_expired_damage(const datetime& now);
    bool                  should_ignore_attacker(const mob& mob_obj, std::shared_ptr<life> attacker) const;
    bool                  should_maintain_target(const mob& mob_obj, const datetime& now) const;
    void                  record_damage(std::shared_ptr<life> attacker, const datetime& now);
    async::task<void>     run_from_target(mob& mob_obj, std::shared_ptr<life> target);
};

} // namespace fb::game

#endif // !__AI_H__