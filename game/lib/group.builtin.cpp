#include <fb/game/group.h>
#include <fb/game/context.h>

using namespace fb::game;

// clang-format off
IMPLEMENT_LUA_EXTENSION(fb::game::group, "fb.game.group")
{"master",              fb::game::group::builtin_master},
{"members",             fb::game::group::builtin_members},
{"nears",               fb::game::group::builtin_nears},
{"message",             fb::game::group::builtin_message},
END_LUA_EXTENSION; // clang-format on

int group::builtin_master(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto argc  = lua->argc();
    auto group = lua->touserdata<fb::game::group>(1);
    if (group == nullptr)
        return 0;

    lua->pushstring(group->_master);
    return 1;
}

int group::builtin_members(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

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

int group::builtin_nears(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

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
    for (int i = 0; i < nears.size(); i++)
    {
        lua->pushobject(nears[i]);
        lua_rawseti(L, -2, i + 1);
    }

    return 1;
}

int group::builtin_message(lua_State* L)
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
    auto type    = argc < 3 ? MESSAGE_TYPE::STATE : static_cast<MESSAGE_TYPE>(lua->tointeger(3));

    static auto fn = [](fb::game::context* context,
                        fb::lua::context*  lua,
                        fb::game::group*   group,
                        const std::string& message,
                        MESSAGE_TYPE       type) -> async::task<void> {
        try
        {
            co_await context->broadcast(*group, message, type);
            lua->pushnil();
        }
        catch (std::exception& e)
        {
            lua->pushstring(e.what());
        }

        lua->resume(1);
    };

    std::ignore = context->threads.current()->dispatch([=](auto&) -> async::task<void> {
        co_await fn(context, lua, group, message, type);
    });

    return lua->yield(1);
}