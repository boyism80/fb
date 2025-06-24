#ifndef __TRACE_H__
#define __TRACE_H__

/**
 * @file    achievement.h
 * @brief   Player achievement system for tracking game accomplishments
 * @author  FB Development Team
 *
 * @details This file implements the achievement system that tracks and manages player
 *          accomplishments and milestones throughout their gameplay experience. The
 *          system provides flexible achievement definitions with Lua scripting
 *          integration for dynamic achievement logic and rewards.
 *
 *          Key features:
 *          - Comprehensive achievement tracking for player accomplishments and milestones
 *          - Model-based achievement configuration with flexible parameter support
 *          - Optional custom text descriptions for personalized achievement messages
 *          - Custom icon and color support for visual achievement representation
 *          - Lua scripting integration for dynamic achievement logic and validation
 *          - Support for complex achievement requirements and conditional triggers
 *          - Achievement reward system integration for granting benefits to players
 *          - Real-time achievement progress tracking and notification system
 *          - Persistent achievement storage and retrieval across game sessions
 *          - Event-driven architecture for responsive achievement processing
 *
 * @note    The achievement system enhances player engagement by providing clear
 *          goals and recognition for various gameplay activities, from simple
 *          tasks to complex long-term objectives.
 */

#include <fb/model/model.h>
#include <fb/lua.h>

namespace fb::game {

/**
 * @brief      Represents a player achievement in the game system.
 *
 *             This class encapsulates a player achievement, which tracks specific accomplishments
 *             or milestones reached by players during gameplay. Achievements can have associated
 *             text descriptions and are integrated with the Lua scripting system for dynamic
 *             achievement logic and rewards.
 *
 *             Features:
 *             - Links to achievement model data for configuration
 *             - Optional custom text descriptions
 *             - Lua scripting integration for dynamic behavior
 *             - Support for achievement tracking and validation
 */
class achievement : public lua::luable
{
public:
    LUA_PROTOTYPE

public:
    struct builtin;

public:
    const fb::model::achievement&    model; ///< Reference to the achievement model configuration
    const std::optional<std::string> text;  ///< Optional custom text description for the achievement
    const std::optional<uint8_t>     icon;  ///< Optional custom icon for the achievement
    const std::optional<uint16_t>    color; ///< Optional custom color for the achievement

public:
    /**
     * @brief      Constructs a new achievement instance.
     *
     *             Creates an achievement object linked to the specified model configuration.
     *             Optionally accepts custom text that can override or supplement the default
     *             achievement description from the model.
     *
     * @param[in]  model  The achievement model containing configuration data.
     * @param[in]  text   Optional custom text description for this achievement instance.
     * @param[in]  icon   Optional custom icon for this achievement instance.
     * @param[in]  color  Optional custom color for this achievement instance.
     */
    achievement(const fb::model::achievement&     model,
                const std::optional<std::string>& text  = std::nullopt,
                const std::optional<uint8_t>&     icon  = std::nullopt,
                const std::optional<uint16_t>&    color = std::nullopt);

    /**
     * @brief      Destroys the achievement instance.
     */
    ~achievement() = default;
};

/**
 * @brief      Lua binding functions for the achievement class.
 *
 *             This structure contains static functions that provide Lua script access
 *             to achievement properties and methods, enabling dynamic achievement
 *             management and querying from game scripts.
 */
struct achievement::builtin
{
    /**
     * @brief      Lua binding function to access the achievement model.
     *
     *             Provides Lua scripts with access to the underlying achievement model
     *             data, including configuration parameters, requirements, and rewards.
     *
     * @param      L     The Lua state for script execution.
     *
     * @return     The number of return values pushed to the Lua stack.
     */
    static int builtin_model(lua_State* L);

    /**
     * @brief      Lua binding function to access the achievement text.
     *
     *             Provides Lua scripts with access to the achievement's text description,
     *             which may be custom text or the default description from the model.
     *
     * @param      L     The Lua state for script execution.
     *
     * @return     The number of return values pushed to the Lua stack.
     */
    static int builtin_text(lua_State* L);
};

} // namespace fb::game

#endif
