#include <clan.h>
#include <context.h>

using namespace fb::game;

// clang-format off
IMPLEMENT_LUA_EXTENSION(fb::game::clan, "fb.game.clan")
{"name",                fb::game::clan::builtin_name},
{"members",             fb::game::clan::builtin_members},
{"nears",               fb::game::clan::builtin_nears},
{"title",               fb::game::clan::builtin_title},
{"join",                fb::game::clan::builtin_join},
{"leave",               fb::game::clan::builtin_leave},
{"message",             fb::game::clan::builtin_message},
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
    auto i = 0;
    for (auto& [name, member] : clan->_members)
    {
        thread->pushobject(member);
        lua_rawseti(lua, -2, i + 1);
        i++;
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

int clan::builtin_title(lua_State* lua)
{
    auto thread = lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto argc    = thread->argc();
    auto clan    = thread->touserdata<fb::game::clan>(1);
    if (clan == nullptr)
        return 0;

    if (argc == 1)
    {
        auto& title = clan->title();
        if (title.has_value())
            thread->pushstring(title.value());
        else
            thread->pushnil();

        return 1;
    }
    else if (argc == 2)
    {
        static auto fn = [](fb::game::context* context,
                            fb::lua::context*  thread,
                            fb::game::clan*    clan,
                            std::string        name) -> async::task<void> {
            try
            {
                co_await context->set_clan_title(*clan, name);
                thread->pushnil();
            }
            catch (std::exception& e)
            {
                thread->pushstring(e.what());
            }

            thread->resume(1);
        };

        auto name   = thread->tostring(2);
        std::ignore = context->threads.current()->dispatch([=](auto&) -> async::task<void> {
            co_await fn(context, thread, clan, name);
        });

        return thread->yield(1);
    }
    else
    {
        return 0;
    }
}

int clan::builtin_join(lua_State* lua)
{
    auto thread = lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto argc    = thread->argc();
    auto clan    = thread->touserdata<fb::game::clan>(1);
    if (clan == nullptr)
        return 0;

    auto ch = thread->touserdata<fb::game::character>(2);
    if (ch == nullptr)
        return 0;

    static auto fn = [](fb::game::context*   context,
                        fb::lua::context*    thread,
                        fb::game::clan*      clan,
                        fb::game::character* ch) -> async::task<void> {
        try
        {
            co_await context->join_clan_member(*clan, *ch);
            thread->pushnil();
        }
        catch (std::exception& e)
        {
            thread->pushstring(e.what());
        }

        thread->resume(1);
    };

    std::ignore = context->threads.current()->dispatch([=](auto&) -> async::task<void> {
        co_await fn(context, thread, clan, ch);
    });

    return thread->yield(1);
}

int clan::builtin_leave(lua_State* lua)
{
    auto thread = lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto argc    = thread->argc();
    auto clan    = thread->touserdata<fb::game::clan>(1);
    if (clan == nullptr)
        return 0;

    auto name = thread->tostring(2);
    auto kick = argc >= 3 ? thread->toboolean(3) : false;

    static auto fn = [](fb::game::context* context,
                        fb::lua::context*  thread,
                        fb::game::clan*    clan,
                        const std::string& name,
                        bool               kick) -> async::task<void> {
        try
        {
            co_await context->leave_clan_member(*clan, name, kick);
            thread->pushnil();
        }
        catch (std::exception& e)
        {
            thread->pushstring(e.what());
        }

        thread->resume(1);
    };

    std::ignore = context->threads.current()->dispatch([=](auto&) -> async::task<void> {
        co_await fn(context, thread, clan, name, kick);
    });

    return thread->yield(1);
}

int clan::builtin_message(lua_State* lua)
{
    auto thread = lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto argc    = thread->argc();
    auto clan    = thread->touserdata<fb::game::clan>(1);
    if (clan == nullptr)
        return 0;

    auto message = thread->tostring(2);
    auto type    = argc < 3 ? MESSAGE_TYPE::STATE : static_cast<MESSAGE_TYPE>(thread->tointeger(3));

    static auto fn = [](fb::game::context* context,
                        fb::lua::context*  thread,
                        fb::game::clan*    clan,
                        const std::string& message,
                        MESSAGE_TYPE       type) -> async::task<void> {
        try
        {
            co_await context->broadcast_clan(*clan, message, type);
            thread->pushnil();
        }
        catch (std::exception& e)
        {
            thread->pushstring(e.what());
        }

        thread->resume(1);
    };

    std::ignore = context->threads.current()->dispatch([=](auto&) -> async::task<void> {
        co_await fn(context, thread, clan, message, type);
    });

    return thread->yield(1);
}