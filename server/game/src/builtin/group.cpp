#include <fb/game/server.h>
#include <fb/game/builtin/group.h>
#include <string_view>

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
    auto& srv   = static_cast<fb::game::server&>(lua->executor);
    auto  argc  = lua->argc();
    auto  group = lua->touserdata<fb::game::group>(1);
    if (group == nullptr)
        return 0;

    auto message  = lua->tostring(2);
    auto type     = lua->toenum(3, MESSAGE_TYPE::STATE);
    auto group_id = group->id();

    auto                                  master_name = group->master();
    character::container::character_ptr_t master_ch;
    {
        auto guard = srv.characters.enter_read();
        master_ch  = guard.value().find(master_name);
    }

    auto error   = std::make_shared<std::optional<std::string>>();
    auto builder = lua->new_co_builder();
    if (master_ch != nullptr)
        builder.weak = master_ch->template weak_from_this_as<fb::game::character>();
    builder.yield = [=]() -> async::task<void> {
        auto& server = static_cast<fb::game::server&>(lua->executor);
        try
        {
            co_await server.groups.broadcast(group_id, message, type);
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

int builtin::group::builtin_toggle(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;
    auto argc  = lua->argc();
    auto group = lua->touserdata<fb::game::group>(1);
    if (group == nullptr)
        return 0;

    auto actor = lua->touserdata<fb::game::character>(2);
    if (actor == nullptr)
        return 0;

    auto target_name = lua->tostring(3);
    if (target_name.empty())
        return 0;

    auto actor_weak = actor->weak_from_this_as<fb::game::character>();
    auto error      = std::make_shared<std::optional<std::string>>();
    auto builder    = lua->new_co_builder();
    builder.weak    = actor_weak;
    builder.yield   = [=]() -> async::task<void> {
        auto& server = static_cast<fb::game::server&>(lua->executor);
        try
        {
            auto actor_shared = actor_weak.lock();
            if (actor_shared == nullptr)
                throw std::runtime_error("actor character is not alive");

            co_await server.groups.toggle_member(*actor_shared, target_name);
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
