#ifndef __NPC_H__
#define __NPC_H__

/**
 * @file    npc.h
 * @brief   Non-Player Character system for interactive game entities
 * @author  FB Development Team
 *
 * @details This file implements the Non-Player Character (NPC) system that handles
 *          all interactive NPCs in the game world. NPCs serve as merchants, quest
 *          givers, guards, story characters, and other interactive entities that
 *          provide services and content to players.
 *
 *          Key features:
 *          - Interactive dialogue and conversation system with scripted responses
 *          - Automated soliloquy system for ambient atmospheric speech
 *          - Merchant and service provider functionality for trading and services
 *          - Quest and story progression integration for narrative content
 *          - Lua scripting integration for dynamic and customizable behavior
 *          - Thread-safe operations with proper thread assertion and management
 *          - Cloning support for creating multiple instances of the same NPC type
 *          - Integration with the dialog system for rich player interactions
 *          - Support for various NPC roles and behaviors through model configuration
 *          - Event-driven architecture for responding to player actions
 *
 * @note    NPCs are essential for creating an immersive game world and provide
 *          the primary means for players to access services, quests, and story
 *          content throughout their gameplay experience.
 */

#include <fb/game/item.h>
#include <format>
#include <optional>

namespace fb::game {

/**
 * @brief      Forward declaration of the character class.
 */
class character;

/**
 * @brief      Represents a Non-Player Character (NPC) in the game world.
 *
 *             This class extends the object class to provide functionality specific to NPCs
 *             that players can interact with. NPCs serve various purposes including merchants,
 *             quest givers, guards, and other interactive entities. They support scripted
 *             behavior, dialogue systems, and automated actions like soliloquies.
 *
 *             Key features:
 *             - Interactive dialogue and scripting system
 *             - Automated soliloquy (random speech) system
 *             - Merchant and service provider functionality
 *             - Quest and story progression integration
 *             - Lua scripting integration for dynamic behavior
 *             - Thread-safe operations with proper assertions
 *             - Cloning support for multiple instances
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
     * @param      context  The game context managing this NPC
     * @param[in]  model    The NPC model data containing name, dialogue, and other settings
     */
    npc(fb::game::context& context, const fb::model::npc& model);
    /**
     * @brief      Constructs a new instance.
     *
     * @param[in]  right  The source NPC to copy from
     */
    npc(const npc& right);
    /**
     * @brief      Destroys the object.
     */
    ~npc();

public:
    /**
     * @brief      Performs a soliloquy (random speech) action.
     *
     *             Makes the NPC speak a random message from their dialogue pool,
     *             typically used for ambient atmosphere and immersion.
     */
    void soliloquy();

    /**
     * @brief      Asserts that the current thread is the correct thread for this NPC.
     *
     *             Validates thread safety by ensuring NPC operations are performed
     *             on the appropriate game thread.
     */
    void assert_thread() const override final;
};

struct npc::builtin
{ };

} // namespace fb::game

#endif // !__NPC_H__
