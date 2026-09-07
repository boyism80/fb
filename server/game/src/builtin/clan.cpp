#include <fb/game/clan.h>
#include <fb/game/server.h>
#include <fb/game/builtin/clan.h>
#include <macro.h>
#include <string_view>

using namespace fb::game;

// clang-format off
IMPLEMENT_LUA_EXTENSION(clan, "fb.game.clan")
{"id",                  builtin::clan::builtin_id},
{"name",                builtin::clan::builtin_name},
{"members",             builtin::clan::builtin_members},
{"nears",               builtin::clan::builtin_nears},
{"title",               builtin::clan::builtin_title},
{"join",                builtin::clan::builtin_join},
{"leave",               builtin::clan::builtin_leave},
{"kick",                builtin::clan::builtin_kick},
{"change_role",         builtin::clan::builtin_change_role},
{"message",             builtin::clan::builtin_message},
{"allied_clan_id",      builtin::clan::builtin_allied_clan_id},
{"enemy_clan_ids",      builtin::clan::builtin_enemy_clan_ids},
{"is_allied",           builtin::clan::builtin_is_allied},
{"is_hostile",          builtin::clan::builtin_is_hostile},
{"request_ally",        builtin::clan::builtin_request_ally},
{"break_ally",          builtin::clan::builtin_break_ally},
{"declare_enemy",       builtin::clan::builtin_declare_enemy},
{"end_enemy",           builtin::clan::builtin_end_enemy},
{"money",               builtin::clan::builtin_money},
END_LUA_EXTENSION; // clang-format on

int builtin::clan::builtin_id(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto clan = lua->touserdata<fb::game::clan>(1);
    if (clan == nullptr)
        return 0;

    lua->pushinteger(clan->id());
    return 1;
}

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
    auto argc = lua->argc();
    auto clan = lua->touserdata<fb::game::clan>(1);
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
        auto changer = lua->touserdata<fb::game::character>(2);
        if (changer == nullptr)
            return 0;

        auto weak_ptr = changer->weak_from_this_as<fb::game::character>();
        auto title    = lua->tostring(3);
        auto error    = std::make_shared<std::optional<std::string>>();
        auto builder  = lua->new_co_builder();
        builder.weak  = weak_ptr;
        builder.yield = [=]() -> async::task<void> {
            auto& server = static_cast<fb::game::server&>(lua->executor);
            try
            {
                auto shared_ptr = weak_ptr.lock();
                if (shared_ptr == nullptr)
                    throw std::runtime_error("character is not alive");

                co_await server.clans.set_title(*shared_ptr, title);
            }
            catch (std::exception& e)
            {
                *error = e.what();
            }
        };
        builder.resume = [=]() -> async::task<int> {
            if (error->has_value())
                lua->pushstring(error->value().c_str());
            else
                lua->pushnil();
            co_return 1;
        };
        return builder.run();
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
    auto argc = lua->argc();
    auto clan = lua->touserdata<fb::game::clan>(1);
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

    auto weak_ptr = inviter->weak_from_this_as<fb::game::character>();
    auto error    = std::make_shared<std::optional<std::string>>();
    auto builder  = lua->new_co_builder();
    builder.weak  = weak_ptr;
    builder.yield = [=]() -> async::task<void> {
        auto& server = static_cast<fb::game::server&>(lua->executor);
        try
        {
            auto shared_ptr = weak_ptr.lock();
            if (shared_ptr == nullptr)
                throw std::runtime_error("inviter character is not alive");

            co_await server.clans.join_member(*shared_ptr, target_name);
        }
        catch (std::exception& e)
        {
            *error = e.what();
        }
    };
    builder.resume = [=]() -> async::task<int> {
        if (error->has_value())
            lua->pushstring(error->value().c_str());
        else
            lua->pushnil();
        co_return 1;
    };
    return builder.run();
}

int builtin::clan::builtin_leave(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;
    auto argc = lua->argc();
    auto clan = lua->touserdata<fb::game::clan>(1);
    if (clan == nullptr)
        return 0;

    auto leaver = lua->touserdata<fb::game::character>(2);
    if (leaver == nullptr)
        return 0;

    auto weak_ptr = leaver->weak_from_this_as<fb::game::character>();
    auto error    = std::make_shared<std::optional<std::string>>();
    auto builder  = lua->new_co_builder();
    builder.weak  = weak_ptr;
    builder.yield = [=]() -> async::task<void> {
        auto& server = static_cast<fb::game::server&>(lua->executor);
        try
        {
            auto shared_ptr = weak_ptr.lock();
            if (shared_ptr == nullptr)
                throw std::runtime_error("leaver character is not alive");

            co_await server.clans.leave_member(*shared_ptr);
        }
        catch (std::exception& e)
        {
            *error = e.what();
        }
    };
    builder.resume = [=]() -> async::task<int> {
        if (error->has_value())
            lua->pushstring(error->value().c_str());
        else
            lua->pushnil();
        co_return 1;
    };
    return builder.run();
}

int builtin::clan::builtin_kick(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;
    auto argc = lua->argc();
    auto clan = lua->touserdata<fb::game::clan>(1);
    if (clan == nullptr)
        return 0;

    auto kicker = lua->touserdata<fb::game::character>(2);
    if (kicker == nullptr)
        return 0;

    auto target = lua->tostring(3);

    auto weak_ptr = kicker->weak_from_this_as<fb::game::character>();
    auto error    = std::make_shared<std::optional<std::string>>();
    auto builder  = lua->new_co_builder();
    builder.weak  = weak_ptr;
    builder.yield = [=]() -> async::task<void> {
        auto& server = static_cast<fb::game::server&>(lua->executor);
        try
        {
            auto shared_ptr = weak_ptr.lock();
            if (shared_ptr == nullptr)
                throw std::runtime_error("kicker character is not alive");

            co_await server.clans.kick_member(*shared_ptr, target);
        }
        catch (std::exception& e)
        {
            *error = e.what();
        }
    };
    builder.resume = [=]() -> async::task<int> {
        if (error->has_value())
            lua->pushstring(error->value().c_str());
        else
            lua->pushnil();
        co_return 1;
    };
    return builder.run();
}

int builtin::clan::builtin_change_role(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;
    auto argc = lua->argc();
    auto clan = lua->touserdata<fb::game::clan>(1);
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

    auto weak_ptr = changer->weak_from_this_as<fb::game::character>();
    auto error    = std::make_shared<std::optional<std::string>>();
    auto builder  = lua->new_co_builder();
    builder.weak  = weak_ptr;
    builder.yield = [=]() -> async::task<void> {
        auto& server = static_cast<fb::game::server&>(lua->executor);
        try
        {
            auto shared_ptr = weak_ptr.lock();
            if (shared_ptr == nullptr)
                throw std::runtime_error("character is not alive");

            co_await server.clans.change_role(*shared_ptr, target, role);
        }
        catch (std::exception& e)
        {
            *error = e.what();
        }
    };
    builder.resume = [=]() -> async::task<int> {
        if (error->has_value())
            lua->pushstring(error->value().c_str());
        else
            lua->pushnil();
        co_return 1;
    };
    return builder.run();
}

int builtin::clan::builtin_message(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;
    auto argc = lua->argc();
    auto clan = lua->touserdata<fb::game::clan>(1);
    if (clan == nullptr)
        return 0;

    auto message = lua->tostring(2);
    auto type    = lua->toenum(3, MESSAGE_TYPE::NOTIFY);
    auto clan_id = clan->id();
    auto world   = uint32_t{0};
    for (auto& [_, weak] : clan->characters())
    {
        auto member = weak.lock();
        if (member == nullptr)
            continue;

        world = member->world();
        break;
    }

    auto error    = std::make_shared<std::optional<std::string>>();
    auto builder  = lua->new_co_builder();
    builder.yield = [=]() -> async::task<void> {
        auto& server = static_cast<fb::game::server&>(lua->executor);
        try
        {
            if (world == 0)
                throw std::runtime_error("no online clan member");

            co_await server.clans.broadcast(world, clan_id, message, type);
        }
        catch (std::exception& e)
        {
            *error = e.what();
        }
    };
    builder.resume = [=]() -> async::task<int> {
        if (error->has_value())
            lua->pushstring(error->value().c_str());
        else
            lua->pushnil();
        co_return 1;
    };
    return builder.run();
}

int builtin::clan::builtin_allied_clan_id(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto clan = lua->touserdata<fb::game::clan>(1);
    if (clan == nullptr)
        return 0;

    auto& allied = clan->allied_clan_id();
    if (allied.has_value())
        lua->pushinteger(allied.value());
    else
        lua->pushnil();
    return 1;
}

int builtin::clan::builtin_enemy_clan_ids(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto clan = lua->touserdata<fb::game::clan>(1);
    if (clan == nullptr)
        return 0;

    lua->new_table();
    auto i = 0;
    for (auto enemy_id : clan->enemy_clan_ids())
    {
        lua->pushinteger(enemy_id);
        lua_rawseti(L, -2, ++i);
    }
    return 1;
}

int builtin::clan::builtin_is_allied(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto clan = lua->touserdata<fb::game::clan>(1);
    if (clan == nullptr)
        return 0;

    std::optional<uint32_t> other_id;
    if (lua->is_number(2))
        other_id = static_cast<uint32_t>(lua->tointeger(2));
    else if (auto other = lua->touserdata<fb::game::clan>(2); other != nullptr)
        other_id = other->id();

    if (other_id.has_value() == false)
        return 0;

    lua->pushboolean(clan->is_allied(other_id.value()));
    return 1;
}

int builtin::clan::builtin_is_hostile(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto clan = lua->touserdata<fb::game::clan>(1);
    if (clan == nullptr)
        return 0;

    std::optional<uint32_t> other_id;
    if (lua->is_number(2))
        other_id = static_cast<uint32_t>(lua->tointeger(2));
    else if (auto other = lua->touserdata<fb::game::clan>(2); other != nullptr)
        other_id = other->id();

    if (other_id.has_value() == false)
        return 0;

    lua->pushboolean(clan->is_hostile(other_id.value()));
    return 1;
}

int builtin::clan::builtin_request_ally(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto clan = lua->touserdata<fb::game::clan>(1);
    if (clan == nullptr)
        return 0;

    auto requester = lua->touserdata<fb::game::character>(2);
    if (requester == nullptr)
        return 0;

    std::optional<uint32_t> target_clan_id;
    if (lua->is_number(3))
        target_clan_id = static_cast<uint32_t>(lua->tointeger(3));
    else if (auto other = lua->touserdata<fb::game::clan>(3); other != nullptr)
        target_clan_id = other->id();

    if (target_clan_id.has_value() == false)
    {
        lua->pushstring(_TEXT(MESSAGE_CLAN_TARGET_NOT_FOUND));
        return 1;
    }

    auto weak_ptr = requester->weak_from_this_as<fb::game::character>();
    auto error    = std::make_shared<std::optional<std::string>>();
    auto builder  = lua->new_co_builder();
    builder.weak  = weak_ptr;
    builder.yield = [=]() -> async::task<void> {
        auto& server = static_cast<fb::game::server&>(lua->executor);
        try
        {
            auto shared_ptr = weak_ptr.lock();
            if (shared_ptr == nullptr)
                throw std::runtime_error("character is not alive");

            co_await server.clans.request_ally(*shared_ptr, target_clan_id.value());
        }
        catch (std::exception& e)
        {
            *error = e.what();
        }
    };
    builder.resume = [=]() -> async::task<int> {
        if (error->has_value())
            lua->pushstring(error->value().c_str());
        else
            lua->pushnil();
        co_return 1;
    };
    return builder.run();
}

int builtin::clan::builtin_break_ally(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto clan = lua->touserdata<fb::game::clan>(1);
    if (clan == nullptr)
        return 0;

    auto requester = lua->touserdata<fb::game::character>(2);
    if (requester == nullptr)
        return 0;

    auto weak_ptr = requester->weak_from_this_as<fb::game::character>();
    auto error    = std::make_shared<std::optional<std::string>>();
    auto builder  = lua->new_co_builder();
    builder.weak  = weak_ptr;
    builder.yield = [=]() -> async::task<void> {
        auto& server = static_cast<fb::game::server&>(lua->executor);
        try
        {
            auto shared_ptr = weak_ptr.lock();
            if (shared_ptr == nullptr)
                throw std::runtime_error("character is not alive");

            co_await server.clans.break_ally(*shared_ptr);
        }
        catch (std::exception& e)
        {
            *error = e.what();
        }
    };
    builder.resume = [=]() -> async::task<int> {
        if (error->has_value())
            lua->pushstring(error->value().c_str());
        else
            lua->pushnil();
        co_return 1;
    };
    return builder.run();
}

int builtin::clan::builtin_declare_enemy(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto clan = lua->touserdata<fb::game::clan>(1);
    if (clan == nullptr)
        return 0;

    auto requester = lua->touserdata<fb::game::character>(2);
    if (requester == nullptr)
        return 0;

    std::optional<uint32_t> target_clan_id;
    if (lua->is_number(3))
        target_clan_id = static_cast<uint32_t>(lua->tointeger(3));
    else if (auto other = lua->touserdata<fb::game::clan>(3); other != nullptr)
        target_clan_id = other->id();

    if (target_clan_id.has_value() == false)
    {
        lua->pushstring(_TEXT(MESSAGE_CLAN_TARGET_NOT_FOUND));
        return 1;
    }

    auto weak_ptr = requester->weak_from_this_as<fb::game::character>();
    auto error    = std::make_shared<std::optional<std::string>>();
    auto builder  = lua->new_co_builder();
    builder.weak  = weak_ptr;
    builder.yield = [=]() -> async::task<void> {
        auto& server = static_cast<fb::game::server&>(lua->executor);
        try
        {
            auto shared_ptr = weak_ptr.lock();
            if (shared_ptr == nullptr)
                throw std::runtime_error("character is not alive");

            co_await server.clans.declare_enemy(*shared_ptr, target_clan_id.value());
        }
        catch (std::exception& e)
        {
            *error = e.what();
        }
    };
    builder.resume = [=]() -> async::task<int> {
        if (error->has_value())
            lua->pushstring(error->value().c_str());
        else
            lua->pushnil();
        co_return 1;
    };
    return builder.run();
}

int builtin::clan::builtin_end_enemy(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto clan = lua->touserdata<fb::game::clan>(1);
    if (clan == nullptr)
        return 0;

    auto requester = lua->touserdata<fb::game::character>(2);
    if (requester == nullptr)
        return 0;

    std::optional<uint32_t> target_clan_id;
    if (lua->is_number(3))
        target_clan_id = static_cast<uint32_t>(lua->tointeger(3));
    else if (auto other = lua->touserdata<fb::game::clan>(3); other != nullptr)
        target_clan_id = other->id();

    if (target_clan_id.has_value() == false)
    {
        lua->pushstring(_TEXT(MESSAGE_CLAN_TARGET_NOT_FOUND));
        return 1;
    }

    auto weak_ptr = requester->weak_from_this_as<fb::game::character>();
    auto error    = std::make_shared<std::optional<std::string>>();
    auto builder  = lua->new_co_builder();
    builder.weak  = weak_ptr;
    builder.yield = [=]() -> async::task<void> {
        auto& server = static_cast<fb::game::server&>(lua->executor);
        try
        {
            auto shared_ptr = weak_ptr.lock();
            if (shared_ptr == nullptr)
                throw std::runtime_error("character is not alive");

            co_await server.clans.end_enemy(*shared_ptr, target_clan_id.value());
        }
        catch (std::exception& e)
        {
            *error = e.what();
        }
    };
    builder.resume = [=]() -> async::task<int> {
        if (error->has_value())
            lua->pushstring(error->value().c_str());
        else
            lua->pushnil();
        co_return 1;
    };
    return builder.run();
}

int builtin::clan::builtin_money(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto clan = lua->touserdata<fb::game::clan>(1);
    if (clan == nullptr)
        return 0;

    auto argc = lua->argc();
    if (argc < 2)
    {
        lua->pushinteger(clan->money());
        return 1;
    }

    auto delta   = static_cast<int64_t>(lua->tointeger(2));
    auto clan_id = clan->id();
    auto world   = uint32_t{0};
    for (auto& [_, weak] : clan->characters())
    {
        auto member = weak.lock();
        if (member == nullptr)
            continue;

        world = member->world();
        break;
    }

    auto error    = std::make_shared<std::optional<std::string>>();
    auto money    = std::make_shared<uint64_t>(0);
    auto builder  = lua->new_co_builder();
    builder.yield = [=]() -> async::task<void> {
        auto& server = static_cast<fb::game::server&>(lua->executor);
        try
        {
            if (world == 0)
                throw std::runtime_error("no online clan member");

            co_await server.clans.add_money(world, clan_id, delta);
            auto guard = co_await server.clans.ensure(clan_id);
            if (guard.value() != nullptr)
                *money = guard.value()->money();
        }
        catch (std::exception& e)
        {
            *error = e.what();
        }
    };
    builder.resume = [=]() -> async::task<int> {
        if (error->has_value())
        {
            lua->pushnil();
            lua->pushstring(error->value().c_str());
            co_return 2;
        }

        lua->pushinteger(*money);
        lua->pushnil();
        co_return 2;
    };
    return builder.run();
}
