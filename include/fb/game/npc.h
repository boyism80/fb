#ifndef __NPC_H__
#define __NPC_H__

#include <fb/game/item.h>
#include <fb/game/appearance.h>
#include <format>
#include <optional>

namespace fb::game {

class character;

class npc : public object
{
public:
    using model_type     = fb::model::npc;
    using appearance_ptr = std::shared_ptr<fb::game::appearance>;

public:
    LUA_PROTOTYPE

private:
    uint16_t _soliloquy_time;

public:
    // clang-format off
    npc(fb::game::server& server, const fb::model::npc& model);
    npc(const npc& right);
    ~npc();
    // clang-format on

public:
    // clang-format off
    void           soliloquy();
    void           assert_thread() const override final;
    appearance_ptr appearance() const override;
    // clang-format on
};

} // namespace fb::game

#endif // !__NPC_H__
