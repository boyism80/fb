#ifndef FB_GAME_AI_INERT_H_
#define FB_GAME_AI_INERT_H_

#include <fb/game/ai.h>

namespace fb::game {

class inert_ai : public ai
{
private:
    using datetime = fb::model::datetime;

public:
    // clang-format off
    bool            execute(mob& mob_obj, const datetime& now) override final;
    MOB_ATTACK_TYPE get_type() const override final;
    void            on_damage(mob& mob_obj, std::shared_ptr<life> attacker, const datetime& now) override final;
    // clang-format on
};

} // namespace fb::game

#endif // !FB_GAME_AI_INERT_H_
