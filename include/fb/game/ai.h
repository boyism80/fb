#ifndef __AI_H__
#define __AI_H__

#include <async/task.h>
#include <fb/model/model.h>

#include <chrono>
#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <unordered_map>
#include <utility>

namespace fb::game {

using namespace std::chrono_literals;
using namespace fb::model::enum_value;

class mob;
class life;
class character;

class ai
{
public:
    static constexpr auto     TARGET_LOCK_DURATION  = std::chrono::seconds(5);
    static constexpr uint64_t MELEE_SWITCH_PERCENT  = 110;
    static constexpr uint64_t RANGED_SWITCH_PERCENT = 130;

private:
    using datetime    = fb::model::datetime;
    using threat_map  = std::map<std::weak_ptr<life>, uint64_t, std::owner_less<std::weak_ptr<life>>>;
    using provocation = std::pair<std::weak_ptr<life>, datetime>;

private:
    threat_map                 _threats;
    std::optional<provocation> _provoked;

public:
    virtual ~ai() = default;
    // clang-format off
    [[nodiscard]] virtual async::task<bool> execute(mob& mob_obj, const datetime& now);
    virtual MOB_ATTACK_TYPE                 get_type() const = 0;
    virtual void                            on_damage(mob& mob_obj, std::shared_ptr<life> attacker, uint64_t amount, const datetime& now);
    void                                    provoke(mob& mob_obj, std::shared_ptr<life> target, const datetime& now);
    static std::unique_ptr<ai>              create(MOB_ATTACK_TYPE attack_type);
    // clang-format on

protected:
    // clang-format off
    virtual std::shared_ptr<life>   find_target(mob& mob_obj, const datetime& now);
    virtual std::shared_ptr<life>   find_adjacent_target(mob& mob_obj, const datetime& now);
    std::shared_ptr<life>           select_target(mob& mob_obj, const datetime& now);
    std::shared_ptr<life>           find_target_in_sight(mob& mob_obj, const datetime& now);
    std::shared_ptr<life>           find_target_in_range(mob& mob_obj, const datetime& now);
    void                            forget_untargetable(mob& mob_obj);
    bool                            should_ignore_attacker(const mob& mob_obj, std::shared_ptr<life> attacker) const;
    bool                            should_maintain_target(const mob& mob_obj, const datetime& now) const;
    uint64_t                        threat(const std::shared_ptr<life>& attacker) const;
    std::shared_ptr<life>           strongest_attacker(mob& mob_obj, bool adjacent) const;
    std::shared_ptr<life>           retarget(mob& mob_obj) const;
    [[nodiscard]] async::task<bool> attack_adjacent(mob& mob_obj, std::shared_ptr<life> target);
    [[nodiscard]] async::task<void> chase(mob& mob_obj, std::shared_ptr<life> target, const datetime& now);
    [[nodiscard]] async::task<void> run_from_target(mob& mob_obj, std::shared_ptr<life> target);
    // clang-format on
};

} // namespace fb::game

#endif // !__AI_H__
