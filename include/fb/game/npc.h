#ifndef __NPC_H__
#define __NPC_H__

#include <fb/game/item.h>
#include <format>
#include <optional>

namespace fb::game {

/**
 * @brief      This class describes a character.
 */
class character;

/**
 * @brief      This class describes a npc.
 */
class npc : public object
{
public:
    using model_type = fb::model::npc;

public:
    LUA_PROTOTYPE

public:
    struct builtin;

private:
    uint16_t _soliloquy_time;

public:
    /**
     * @brief      Constructs a new instance.
     *
     * @param      context  The context
     * @param[in]  model    The model
     */
    npc(fb::game::context& context, const fb::model::npc& model);
    /**
     * @brief      Constructs a new instance.
     *
     * @param[in]  right  The right
     */
    npc(const npc& right);
    /**
     * @brief      Destroys the object.
     */
    ~npc();

public:
    /**
     * @brief      { function_description }
     *
     * @param      context  The context
     *
     * @return     { description_of_the_return_value }
     */
    npc* make(fb::game::context& context);

    /**
     * @brief      { function_description }
     */
    void soliloquy();

    /**
     * @brief      { function_description }
     */
    void assert_thread() const override final;
};

struct npc::builtin
{ };

} // namespace fb::game

#endif // !__NPC_H__
