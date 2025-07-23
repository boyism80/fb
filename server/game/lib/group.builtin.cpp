#include <fb/game/group.h>
#include <fb/game/server.h>

using namespace fb::game;

// clang-format off
IMPLEMENT_LUA_EXTENSION(group, "fb.game.group")
{"master",              group::builtin::builtin_master},
{"members",             group::builtin::builtin_members},
{"nears",               group::builtin::builtin_nears},
{"message",             group::builtin::builtin_message},
{"kick",                group::builtin::builtin_kick},
END_LUA_EXTENSION; // clang-format on

int group::builtin::builtin_master(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

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

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto group  = lua->touserdata<fb::game::group>(1);
    if (group == nullptr)
        return 0;

    auto message = lua->tostring(2);
    auto type    = lua->toenum(3, MESSAGE_TYPE::STATE);
    async::awaitable_then(server->broadcast(*group, message, type), [=](auto result) {
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

int group::builtin::builtin_kick(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto group  = lua->touserdata<fb::game::group>(1);
    if (group == nullptr)
        return 0;

    auto kicker = lua->tostring(2);
    auto target = lua->tostring(3);

    static auto fn = [](fb::game::server*  server,
                        fb::lua::context*  lua,
                        fb::game::group*   group,
                        const std::string& kicker,
                        const std::string& target) -> async::task<void> {
        try
        {
            co_await server->kick_group_member(*group, kicker, target);
            lua->pushnil();
        }
        catch (std::exception& e)
        {
            lua->pushstring(e.what());
        }

        lua->resume(1);
    };

    std::ignore = server->threads.current()->dispatch([=](auto&) -> async::task<void> {
        co_await fn(server, lua, group, kicker, target);
    });

    return lua->yield(1);
}