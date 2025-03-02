#ifndef __CLAN_MEMBER_H__
#define __CLAN_MEMBER_H__

#include <fb/lua.h>
#include <fb/model/model.h>

using namespace fb::model::enum_value;

namespace fb::game {

/**
 * @brief      This class describes a clan member.
 */
class clan_member : public lua::luable
{
public:
    LUA_PROTOTYPE

public:
    std::string   name;
    CLAN_POSITION position;

public:
    clan_member(const std::string& name, CLAN_POSITION position);
    ~clan_member() = default;

public:
    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_name(lua_State* L);
    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_position(lua_State* L);
};

} // namespace fb::game

#endif