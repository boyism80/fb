#include <group.h>

using namespace fb::game;

// clang-format off
IMPLEMENT_LUA_EXTENSION(fb::game::group, "fb.game.group")
{"master",				fb::game::group::builtin_master},
{"members",				fb::game::group::builtin_members},
{"nears",				fb::game::group::builtin_nears},
END_LUA_EXTENSION; // clang-format on

int group::builtin_master(lua_State* lua)
{
    auto thread = lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto argc  = thread->argc();
    auto group = thread->touserdata<fb::game::group>(1);
    if (group == nullptr)
        return 0;

    thread->pushstring(group->_master);
    return 1;
}

int group::builtin_members(lua_State* lua)
{
    auto thread = lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto argc  = thread->argc();
    auto group = thread->touserdata<fb::game::group>(1);
    if (group == nullptr)
        return 0;

    thread->new_table();
    for (int i = 0, n = group->_members.size(); i < n; i++)
    {
        thread->pushstring(group->_members[i]);
        lua_rawseti(lua, -2, i + 1);
    }

    return 1;
}

int group::builtin_nears(lua_State* lua)
{
    auto thread = lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto argc  = thread->argc();
    auto group = thread->touserdata<fb::game::group>(1);
    if (group == nullptr)
        return 0;

    auto map = thread->touserdata<fb::game::map>(2);
    if (map == nullptr)
        return 0;

    uint16_t x, y;
    if (!thread->is_table(3))
        return 0;

    thread->rawgeti(3, 1);
    x = (uint16_t)thread->tointeger(-1);
    thread->remove(-1);
    thread->rawgeti(3, 2);
    y = (uint16_t)thread->tointeger(-1);
    thread->remove(-1);

    auto nears = group->nears(*map, point16_t{x, y});
    thread->new_table();
    for (int i = 0; i < nears.size(); i++)
    {
        thread->pushobject(nears[i]);
        lua_rawseti(lua, -2, i + 1);
    }

    return 1;
}