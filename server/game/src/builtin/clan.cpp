#include <fb/game/clan.h>
#include <fb/game/server.h>
#include <fb/game/builtin/clan.h>
#include <string_view>

using namespace fb::game;

// clang-format off
IMPLEMENT_LUA_EXTENSION(clan, "fb.game.clan")
{"name",                builtin::clan::builtin_name},
{"members",             builtin::clan::builtin_members},
{"nears",               builtin::clan::builtin_nears},
{"title",               builtin::clan::builtin_title},
{"join",                builtin::clan::builtin_join},
{"leave",               builtin::clan::builtin_leave},
{"kick",                builtin::clan::builtin_kick},
{"change_role",         builtin::clan::builtin_change_role},
{"message",             builtin::clan::builtin_message},
END_LUA_EXTENSION; // clang-format on

int builtin::clan::builtin_name(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto clan = lua->touserdata<fb::game::clan>(1);
    if (clan == nullptr)
        return 0;

    lua->pushstring(clan->name());
    return 1;
}

int builtin::clan::builtin_members(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto argc = lua->argc();
    auto clan = lua->touserdata<fb::game::clan>(1);
    if (clan == nullptr)
        return 0;

    lua->new_table();
    auto  i       = 0;
    auto& members = clan->members();
    for (auto& [name, member] : members)
    {
        lua->pushobject(member);
        lua_rawseti(L, -2, i + 1);
        i++;
    }

    return 1;
}

int builtin::clan::builtin_nears(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto argc = lua->argc();
    auto clan = lua->touserdata<fb::game::clan>(1);
    if (clan == nullptr)
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

    auto nears = clan->nears(*map, fb::model::point16_t{x, y});
    lua->new_table();
    for (int i = 0; i < nears.size(); i++)
    {
        lua->pushobject(nears[i]);
        lua_rawseti(L, -2, i + 1);
    }

    return 1;
}

int builtin::clan::builtin_title(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto clan   = lua->touserdata<fb::game::clan>(1);
    if (clan == nullptr)
        return 0;

    if (argc == 1)
    {
        auto& title = clan->title();
        if (title.has_value())
            lua->pushstring(title.value());
        else
            lua->pushnil();

        return 1;
    }
    else if (argc == 3)
    {
        static auto fn = [](fb::lua::context*                  lua,
                            fb::game::server*                  server,
                            std::weak_ptr<fb::game::character> weak_ptr,
                            std::string                        title) -> async::task<void> {
            try
            {
                auto shared_ptr = weak_ptr.lock();
                if (shared_ptr == nullptr)
                    throw std::runtime_error("character is not alive");

                co_await server->clan.set_title(*shared_ptr, title);
                co_await lua->switching();
                lua->pushnil();
                lua->resume(1);
            }
            catch (std::exception& e)
            {
                auto what = std::string(e.what());
                async::awaitable_then(lua->switching(), [lua, what](auto result) {
                    result();
                    lua->pushstring(what.c_str());
                    lua->resume(1);
                });
            }
        };

        auto changer = lua->touserdata<fb::game::character>(2);
        if (changer == nullptr)
            return 0;

        auto weak_ptr = changer->weak_from_this_as<fb::game::character>();
        auto title    = lua->tostring(3);
        auto builder  = server->threads.new_builder(weak_ptr);
        builder.func  = [=](auto&) -> async::task<void> {
            co_await fn(lua, server, weak_ptr, title);
        };
        builder.enqueue();

        return lua->yield(1);
    }
    else
    {
        return 0;
    }
}

int builtin::clan::builtin_join(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto clan   = lua->touserdata<fb::game::clan>(1);
    if (clan == nullptr)
        return 0;

    auto inviter = lua->touserdata<fb::game::character>(2);
    if (inviter == nullptr)
        return 0;

    auto target_name = lua->tostring(3);
    if (target_name.empty())
        return 0;

    if (clan->member(inviter->name()) == nullptr)
    {
        lua->pushstring("inviter is not a member of the clan");
        return 1;
    }

    static auto fn = [](fb::lua::context*                  lua,
                        fb::game::server*                  server,
                        std::weak_ptr<fb::game::character> weak_ptr,
                        std::string_view                   target_name) -> async::task<void> {
        try
        {
            auto shared_ptr = weak_ptr.lock();
            if (shared_ptr == nullptr)
                throw std::runtime_error("inviter character is not alive");

            co_await server->clan.join_member(*shared_ptr, target_name);
            co_await lua->switching();
            lua->pushnil();
            lua->resume(1);
        }
        catch (std::exception& e)
        {
            auto what = std::string(e.what());
            async::awaitable_then(lua->switching(), [lua, what](auto result) {
                result();
                lua->pushstring(what.c_str());
                lua->resume(1);
            });
        }
    };

    auto weak_ptr = inviter->weak_from_this_as<fb::game::character>();
    auto builder  = server->threads.new_builder(weak_ptr);
    builder.func  = [=](auto&) -> async::task<void> {
        co_await fn(lua, server, weak_ptr, target_name);
    };
    builder.enqueue();

    return lua->yield(1);
}

int builtin::clan::builtin_leave(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto clan   = lua->touserdata<fb::game::clan>(1);
    if (clan == nullptr)
        return 0;

    auto leaver = lua->touserdata<fb::game::character>(2);
    if (leaver == nullptr)
        return 0;

    static auto fn = [](fb::lua::context*                  lua,
                        fb::game::server*                  server,
                        std::weak_ptr<fb::game::character> weak_ptr) -> async::task<void> {
        try
        {
            auto shared_ptr = weak_ptr.lock();
            if (shared_ptr == nullptr)
                throw std::runtime_error("leaver character is not alive");

            co_await server->clan.leave_member(*shared_ptr);
            co_await lua->switching();
            lua->pushnil();
            lua->resume(1);
        }
        catch (std::exception& e)
        {
            auto what = std::string(e.what());
            async::awaitable_then(lua->switching(), [lua, what](auto result) {
                result();
                lua->pushstring(what.c_str());
                lua->resume(1);
            });
        }
    };

    auto weak_ptr = leaver->weak_from_this_as<fb::game::character>();
    auto builder  = server->threads.new_builder(weak_ptr);
    builder.func  = [=](auto&) -> async::task<void> {
        co_await fn(lua, server, weak_ptr);
    };
    builder.enqueue();

    return lua->yield(1);
}

int builtin::clan::builtin_kick(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto clan   = lua->touserdata<fb::game::clan>(1);
    if (clan == nullptr)
        return 0;

    auto kicker = lua->touserdata<fb::game::character>(2);
    if (kicker == nullptr)
        return 0;

    auto target = lua->tostring(3);

    static auto fn = [](fb::lua::context*                  lua,
                        fb::game::server*                  server,
                        std::weak_ptr<fb::game::character> weak_ptr,
                        std::string_view                   target) -> async::task<void> {
        try
        {
            auto shared_ptr = weak_ptr.lock();
            if (shared_ptr == nullptr)
                throw std::runtime_error("kicker character is not alive");

            co_await server->clan.kick_member(*shared_ptr, target);
            co_await lua->switching();
            lua->pushnil();
            lua->resume(1);
        }
        catch (std::exception& e)
        {
            auto what = std::string(e.what());
            async::awaitable_then(lua->switching(), [lua, what](auto result) {
                result();
                lua->pushstring(what.c_str());
                lua->resume(1);
            });
        }
    };

    auto weak_ptr = kicker->weak_from_this_as<fb::game::character>();
    auto builder  = server->threads.new_builder(weak_ptr);
    builder.func  = [=](auto&) -> async::task<void> {
        co_await fn(lua, server, weak_ptr, target);
    };
    builder.enqueue();

    return lua->yield(1);
}

int builtin::clan::builtin_change_role(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto clan   = lua->touserdata<fb::game::clan>(1);
    if (clan == nullptr)
    {
        lua->pushstring("clan is not found");
        return 1;
    }

    auto changer = lua->touserdata<fb::game::character>(2);
    if (changer == nullptr)
    {
        lua->pushstring("changer is not found");
        return 1;
    }

    auto target = lua->tostring(3);
    auto role   = lua->toenum(4, CLAN_ROLE::MATE);

    static auto fn = [](fb::lua::context*                  lua,
                        fb::game::server*                  server,
                        std::weak_ptr<fb::game::character> weak_ptr,
                        std::string_view                   target,
                        CLAN_ROLE                          role) -> async::task<void> {
        try
        {
            auto shared_ptr = weak_ptr.lock();
            if (shared_ptr == nullptr)
                throw std::runtime_error("character is not alive");

            co_await server->clan.change_role(*shared_ptr, target, role);
            co_await lua->switching();
            lua->pushnil();
            lua->resume(1);
        }
        catch (std::exception& e)
        {
            auto what = std::string(e.what());
            async::awaitable_then(lua->switching(), [lua, what](auto result) {
                result();
                lua->pushstring(what.c_str());
                lua->resume(1);
            });
        }
    };

    auto weak_ptr = changer->weak_from_this_as<fb::game::character>();
    auto builder  = server->threads.new_builder(weak_ptr);
    builder.func  = [=](auto&) -> async::task<void> {
        co_await fn(lua, server, weak_ptr, target, role);
    };
    builder.enqueue();

    return lua->yield(1);
}

int builtin::clan::builtin_message(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto clan   = lua->touserdata<fb::game::clan>(1);
    if (clan == nullptr)
        return 0;

    auto message = lua->tostring(2);
    auto type    = lua->toenum(3, MESSAGE_TYPE::NOTIFY);
    auto clan_id = clan->id();

    static auto fn = [](fb::lua::context* lua,
                        fb::game::server* server,
                        uint32_t          clan_id,
                        std::string_view  message,
                        MESSAGE_TYPE      type) -> async::task<void> {
        try
        {
            co_await server->clan.broadcast(clan_id, message, type);
            co_await lua->switching();
            lua->pushnil();
            lua->resume(1);
        }
        catch (std::exception& e)
        {
            auto what = std::string(e.what());
            async::awaitable_then(lua->switching(), [lua, what](auto result) {
                result();
                lua->pushstring(what.c_str());
                lua->resume(1);
            });
        }
    };

    auto thread = server->threads.current();
    if (thread == nullptr)
        return 0;

    auto builder = thread->new_builder<void>();
    builder.func = [=](auto&) -> async::task<void> {
        co_await fn(lua, server, clan_id, message, type);
    };
    builder.enqueue();

    return lua->yield(1);
}