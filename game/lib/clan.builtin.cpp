#include <fb/game/clan.h>
#include <fb/game/context.h>

using namespace fb::game;

// clang-format off
IMPLEMENT_LUA_EXTENSION(clan, "fb.game.clan")
{"name",                clan::builtin::builtin_name},
{"members",             clan::builtin::builtin_members},
{"nears",               clan::builtin::builtin_nears},
{"title",               clan::builtin::builtin_title},
{"join",                clan::builtin::builtin_join},
{"leave",               clan::builtin::builtin_leave},
{"kick",                clan::builtin::builtin_kick},
{"change_role",         clan::builtin::builtin_change_role},
{"message",             clan::builtin::builtin_message},
END_LUA_EXTENSION; // clang-format on

int clan::builtin::builtin_name(lua_State* L)
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

int clan::builtin::builtin_members(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto argc = lua->argc();
    auto clan = lua->touserdata<fb::game::clan>(1);
    if (clan == nullptr)
        return 0;

    lua->new_table();
    auto i = 0;
    for (auto& [name, member] : clan->_members)
    {
        lua->pushobject(member);
        lua_rawseti(L, -2, i + 1);
        i++;
    }

    return 1;
}

int clan::builtin::builtin_nears(lua_State* L)
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

int clan::builtin::builtin_title(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto argc    = lua->argc();
    auto clan    = lua->touserdata<fb::game::clan>(1);
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
    else if (argc == 2)
    {
        static auto fn = [](fb::game::context* context,
                            fb::lua::context*  lua,
                            fb::game::clan*    clan,
                            std::string        name) -> async::task<void> {
            try
            {
                co_await context->set_clan_title(*clan, name);
                lua->pushnil();
            }
            catch (std::exception& e)
            {
                lua->pushstring(e.what());
            }

            lua->resume(1);
        };

        auto name   = lua->tostring(2);
        std::ignore = context->threads.current()->dispatch([=](auto&) -> async::task<void> {
            co_await fn(context, lua, clan, name);
        });

        return lua->yield(1);
    }
    else
    {
        return 0;
    }
}

int clan::builtin::builtin_join(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto argc    = lua->argc();
    auto clan    = lua->touserdata<fb::game::clan>(1);
    if (clan == nullptr)
        return 0;

    auto inviter = lua->touserdata<fb::game::character>(2);
    if (inviter == nullptr)
        return 0;

    auto invitee = lua->touserdata<fb::game::character>(3);
    if (invitee == nullptr)
        return 0;

    if (clan->member(inviter->name()) == nullptr)
    {
        lua->pushstring("inviter is not a member of the clan");
        return 1;
    }

    if (invitee->clan_id().has_value())
    {
        lua->pushstring("invitee is already a member of a clan");
        return 1;
    }

    static auto fn = [](fb::game::context*                 context,
                        fb::lua::context*                  lua,
                        std::weak_ptr<fb::game::character> inviter_weak,
                        std::weak_ptr<fb::game::character> invitee_weak) -> async::task<void> {
        try
        {
            auto inviter_shared = inviter_weak.lock();
            if (inviter_shared == nullptr)
                throw std::runtime_error("inviter character is not alive");

            auto invitee_shared = invitee_weak.lock();
            if (invitee_shared == nullptr)
                throw std::runtime_error("invitee character is not alive");

            co_await context->join_clan_member(*inviter_shared, *invitee_shared);
            lua->pushnil();
        }
        catch (std::exception& e)
        {
            lua->pushstring(e.what());
        }

        lua->resume(1);
    };

    auto thread = context->threads.current();
    if (thread == nullptr)
        throw std::runtime_error("thread is not alive");

    std::ignore = thread->dispatch([=](auto&) -> async::task<void> {
        co_await fn(context, lua, inviter, invitee);
    });

    return lua->yield(0);
}

int clan::builtin::builtin_leave(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto argc    = lua->argc();
    auto clan    = lua->touserdata<fb::game::clan>(1);
    if (clan == nullptr)
        return 0;

    auto name = lua->tostring(2);

    static auto fn = [](fb::game::context* context,
                        fb::lua::context*  lua,
                        fb::game::clan*    clan,
                        const std::string& name) -> async::task<void> {
        try
        {
            co_await context->leave_clan_member(*clan, name);
            lua->pushnil();
        }
        catch (std::exception& e)
        {
            lua->pushstring(e.what());
        }

        lua->resume(1);
    };

    std::ignore = context->threads.current()->dispatch([=](auto&) -> async::task<void> {
        co_await fn(context, lua, clan, name);
    });

    return lua->yield(1);
}

int clan::builtin::builtin_kick(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto argc    = lua->argc();
    auto clan    = lua->touserdata<fb::game::clan>(1);
    if (clan == nullptr)
        return 0;

    auto kicker = lua->tostring(2);
    auto target = lua->tostring(3);

    static auto fn = [](fb::game::context* context,
                        fb::lua::context*  lua,
                        fb::game::clan*    clan,
                        const std::string& kicker,
                        const std::string& target) -> async::task<void> {
        try
        {
            co_await context->kick_clan_member(*clan, kicker, target);
            lua->pushnil();
        }
        catch (std::exception& e)
        {
            lua->pushstring(e.what());
        }

        lua->resume(1);
    };

    std::ignore = context->threads.current()->dispatch([=](auto&) -> async::task<void> {
        co_await fn(context, lua, clan, kicker, target);
    });

    return lua->yield(1);
}

int clan::builtin::builtin_change_role(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto argc    = lua->argc();
    auto clan    = lua->touserdata<fb::game::clan>(1);
    if (clan == nullptr)
        return 0;

    auto changer = lua->tostring(2);
    auto target  = lua->tostring(3);
    auto role    = lua->toenum(4, CLAN_ROLE::MATE);

    static auto fn = [](fb::game::context* context,
                        fb::lua::context*  lua,
                        fb::game::clan*    clan,
                        const std::string& changer,
                        const std::string& target,
                        CLAN_ROLE          role) -> async::task<void> {
        try
        {
            co_await context->change_clan_member_role(*clan, changer, target, role);
            lua->pushnil();
        }
        catch (std::exception& e)
        {
            lua->pushstring(e.what());
        }

        lua->resume(1);
    };

    std::ignore = context->threads.current()->dispatch([=](auto&) -> async::task<void> {
        co_await fn(context, lua, clan, changer, target, role);
    });

    return lua->yield(1);
}

int clan::builtin::builtin_message(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto argc    = lua->argc();
    auto clan    = lua->touserdata<fb::game::clan>(1);
    if (clan == nullptr)
        return 0;

    auto message = lua->tostring(2);
    auto type    = lua->toenum(3, MESSAGE_TYPE::STATE);

    static auto fn = [](fb::game::context* context,
                        fb::lua::context*  lua,
                        fb::game::clan*    clan,
                        const std::string& message,
                        MESSAGE_TYPE       type) -> async::task<void> {
        try
        {
            co_await context->broadcast(*clan, message, type);
            lua->pushnil();
        }
        catch (std::exception& e)
        {
            lua->pushstring(e.what());
        }

        lua->resume(1);
    };

    std::ignore = context->threads.current()->dispatch([=](auto&) -> async::task<void> {
        co_await fn(context, lua, clan, message, type);
    });

    return lua->yield(1);
}