#ifndef FB_GAME_AI_AGGRESSIVE_H_
#define FB_GAME_AI_AGGRESSIVE_H_

#include <fb/game/ai.h>

namespace fb::game {

class aggressive_ai : public ai
{
private:
    using super    = ai;
    using datetime = fb::model::datetime;

public:
    // clang-format off
    [[nodiscard]] async::task<bool> execute(mob& mob_obj, const datetime& now) override;
    MOB_ATTACK_TYPE                 get_type() const override;
    // clang-format on

protected:
    std::shared_ptr<life> find_target(mob& mob_obj, const datetime& now) override;
    std::shared_ptr<life> find_adjacent_target(mob& mob_obj, const datetime& now) override;
};

} // namespace fb::game

#endif // !FB_GAME_AI_AGGRESSIVE_H_
