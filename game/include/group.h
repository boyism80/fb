#ifndef __GROUP_H__
#define __GROUP_H__

#include <character.h>

namespace fb { namespace game {

/**
 * @brief      This class describes a character.
 */
class character;

/**
 * @brief      This class describes a group.
 */
class group : public lua::luable
{
public:
    LUA_PROTOTYPE

private:
    character*           _leader;
    character::container _members;

private:
    /**
     * @brief      Constructs a new instance.
     *
     * @param      leader  The leader
     */
    group(character& leader);

public:
    /**
     * @brief      Destroys the object.
     */
    ~group();

public:
    /**
     * @brief      { function_description }
     *
     * @param      session  The session
     *
     * @return     { description_of_the_return_value }
     */
    character* enter(character& session);
    /**
     * @brief      { function_description }
     *
     * @param      session  The session
     *
     * @return     { description_of_the_return_value }
     */
    character* leave(character& session);
    /**
     * @brief      { function_description }
     *
     * @param      session  The session
     *
     * @return     { description_of_the_return_value }
     */
    bool contains(character& session);
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    character& leader() const;
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    const character::container& members() const;

public:
    /**
     * @brief      { function_description }
     *
     * @param      leader  The leader
     *
     * @return     { description_of_the_return_value }
     */
    static fb::game::group* create(character& leader);
    /**
     * @brief      Destroys the given group.
     *
     * @param      group  The group
     */
    static void destroy(fb::game::group& group);

public:
    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_members(lua_State* lua);
    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_leader(lua_State* lua);
};

}} // namespace fb::game

#endif // !__GROUP_H__
