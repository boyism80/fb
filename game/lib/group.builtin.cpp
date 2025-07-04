#include <fb/game/group.h>
#include <fb/game/context.h>

using namespace fb::game;

// clang-format off
IMPLEMENT_LUA_EXTENSION(group, "fb.game.group")
{"master",              group::builtin::builtin_master},
{"members",             group::builtin::builtin_members},
{"nears",               group::builtin::builtin_nears},
{"message",             group::builtin::builtin_message},
END_LUA_EXTENSION; // clang-format on

int group::builtin::builtin_master(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx   = lua->env<fb::game::context>("context");
    auto argc  = lua->argc();
    auto group = lua->touserdata<fb::game::group>(1);
    if (group == nullptr)
        return 0;

    lua->pushstring(group->_master);
    return 1;
}

int group::builtin::builtin_members(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx   = lua->env<fb::game::context>("context");
    auto argc  = lua->argc();
    auto group = lua->touserdata<fb::game::group>(1);
    if (group == nullptr)
        return 0;

    lua->new_table();
    for (int i = 0, n = group->_members.size(); i < n; i++)
    {
        lua->pushstring(group->_members[i]);
        lua_rawseti(L, -2, i + 1);
    }
    return 1;
}

int group::builtin::builtin_nears(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx   = lua->env<fb::game::context>("context");
    auto argc  = lua->argc();
    auto group = lua->touserdata<fb::game::group>(1);
    if (group == nullptr)
        return 0;

    auto map = lua->touserdata<fb::game::map>(2);
    if (map == nullptr)
        return 0;

    uint16_t x, y;
    if (!lua->is_table(3))
        return 0;

    lua->rawgeti(3, 1);
    x = (uint16_t)lua->tointeger(-1);
    lua->remove(-1);
    lua->rawgeti(3, 2);
    y = (uint16_t)lua->tointeger(-1);
    lua->remove(-1);

    auto nears = group->nears(*map, fb::model::point16_t{x, y});
    lua->new_table();
    int i = 0;
    for (auto& weak_ptr : nears)
    {
        auto shared_ptr = weak_ptr.lock();
        if (shared_ptr == nullptr)
            continue;

        lua->pushobject(shared_ptr);
        lua_rawseti(L, -2, i + 1);
        i++;
    }
    return 1;
}

int group::builtin::builtin_message(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto argc    = lua->argc();
    auto group   = lua->touserdata<fb::game::group>(1);
    if (group == nullptr)
        return 0;

    auto message = lua->tostring(2);
    auto type    = lua->toenum(3, MESSAGE_TYPE::STATE);
    async::awaitable_then(context->broadcast(*group, message, type), [=](auto result) {
        try
        {
            result();
            lua->pushnil();
        }
        catch (std::exception& e)
        {
            lua->pushstring(e.what());
        }
        lua->resume(1);
    });

    return lua->yield(1);
}