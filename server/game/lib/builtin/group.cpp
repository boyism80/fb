#include <fb/game/server.h>
#include <fb/game/builtin/group.h>

using namespace fb::game;

// clang-format off
IMPLEMENT_LUA_EXTENSION(group, "fb.game.group")
{"master",              builtin::group::builtin_master},
{"members",             builtin::group::builtin_members},
{"nears",               builtin::group::builtin_nears},
{"message",             builtin::group::builtin_message},
{"toggle",              builtin::group::builtin_toggle},
END_LUA_EXTENSION; // clang-format on

int builtin::group::builtin_master(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto group = lua->touserdata<fb::game::group>(1);
    if (group == nullptr)
        return 0;

    auto master = group->master();
    lua->pushstring(master);
    return 1;
}

int builtin::group::builtin_members(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto group = lua->touserdata<fb::game::group>(1);
    if (group == nullptr)
        return 0;

    lua->new_table();
    auto members = group->members();
    for (int i = 0, n = members.size(); i < n; i++)
    {
        lua->pushstring(members[i]);
        lua_rawseti(L, -2, i + 1);
    }
    return 1;
}

int builtin::group::builtin_nears(lua_State* L)
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

int builtin::group::builtin_message(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto group  = lua->touserdata<fb::game::group>(1);
    if (group == nullptr)
        return 0;

    auto message  = lua->tostring(2);
    auto type     = lua->toenum(3, MESSAGE_TYPE::STATE);
    auto group_id = group->id();

    static auto fn = [](fb::lua::context*  lua,
                        fb::game::server*  server,
                        uint32_t           group_id,
                        const std::string& message,
                        MESSAGE_TYPE       type) -> async::task<void> {
        try
        {
            co_await server->broadcast_group(group_id, message, type);
            lua->pushnil();
        }
        catch (std::exception& e)
        {
            lua->pushstring(e.what());
        }

        lua->resume(1);
    };

    // Use group's master character to determine the thread
    auto master_name = group->master();
    return server->characters.read([=](auto& container) {
        auto master_ch = container.find(master_name);
        if (master_ch == nullptr)
        {
            // If master not found, fall back to current thread dispatch
            auto thread = server->threads.current();
            if (thread == nullptr)
            {
                lua->pushstring("thread not found");
                return 1;
            }

            std::ignore = thread->dispatch([=](auto&) -> async::task<void> {
                co_await fn(lua, server, group_id, message, type);
            });
            return lua->yield(1);
        }
        else
        {
            auto weak = master_ch->template weak_from_this_as<fb::game::character>();
            server->threads.enqueue(weak, [=](auto&) -> async::task<void> {
                co_await fn(lua, server, group_id, message, type);
            });
            return lua->yield(1);
        }
    });
}

int builtin::group::builtin_toggle(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto group  = lua->touserdata<fb::game::group>(1);
    if (group == nullptr)
        return 0;

    auto actor = lua->touserdata<fb::game::character>(2);
    if (actor == nullptr)
        return 0;

    auto target_name = lua->tostring(3);
    if (target_name.empty())
        return 0;

    static auto fn = [](fb::lua::context*                    lua,
                        fb::game::server*                    server,
                        std::shared_ptr<fb::game::character> actor_shared,
                        const std::string&                   target_name) -> async::task<void> {
        try
        {
            if (actor_shared == nullptr)
                throw std::runtime_error("actor character is not alive");

            co_await server->toggle_group_member(*actor_shared, target_name);
            lua->pushnil();
        }
        catch (std::exception& e)
        {
            lua->pushstring(e.what());
        }

        lua->resume(1);
    };

    auto actor_shared = actor->shared_from_this_as<fb::game::character>();
    auto actor_weak   = actor->weak_from_this_as<fb::game::character>();
    server->threads.enqueue(actor_weak, [=](auto&) -> async::task<void> {
        co_await fn(lua, server, actor_shared, target_name);
    });

    return lua->yield(1);
}