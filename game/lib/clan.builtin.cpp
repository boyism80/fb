#include <clan.h>

using namespace fb::game;

// clang-format off
IMPLEMENT_LUA_EXTENSION(fb::game::clan, "fb.game.clan")
{"name",                fb::game::clan::builtin_name},
{"members",             fb::game::clan::builtin_members},
{"nears",               fb::game::clan::builtin_nears},
END_LUA_EXTENSION; // clang-format on

int clan::builtin_name(lua_State* lua)
{
    auto thread = lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto clan = thread->touserdata<fb::game::clan>(1);
    if (clan == nullptr)
        return 0;

    thread->pushstring(clan->name());
    return 1;
}

int clan::builtin_members(lua_State* lua)
{
    auto thread = lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto argc = thread->argc();
    auto clan = thread->touserdata<fb::game::clan>(1);
    if (clan == nullptr)
        return 0;

    thread->new_table();
    for (int i = 0, n = clan->_members.size(); i < n; i++)
    {
        thread->pushobject(clan->_members[i]);
        lua_rawseti(lua, -2, i + 1);
    }

    return 1;
}

int clan::builtin_nears(lua_State* lua)
{
    auto thread = lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto argc = thread->argc();
    auto clan = thread->touserdata<fb::game::clan>(1);
    if (clan == nullptr)
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

    auto nears = clan->nears(*map, point16_t{x, y});
    thread->new_table();
    for (int i = 0; i < nears.size(); i++)
    {
        thread->pushobject(nears[i]);
        lua_rawseti(lua, -2, i + 1);
    }

    return 1;
}