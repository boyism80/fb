#include <fb/game/builtin/server.h>
#include <boost/xpressive/xpressive.hpp>
#include <unordered_map>
#include <regex>

using namespace fb::game;
using table = fb::model::table;

// clang-format off
IMPLEMENT_LUA_EXTENSION(fb::game::server, "")
END_LUA_EXTENSION; // clang-format on

int builtin::server::builtin_log(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto message = lua->tostring(1);
    fb::logger::info(message);
    return 0;
}

int builtin::server::builtin_seed(lua_State* L)
{
    static std::random_device random;
    static std::mt19937       gen(random());
    static auto               distribution = std::uniform_int_distribution<uint32_t>(0, 0xFFFFFFFF);

    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto seed = distribution(gen);
    lua->pushinteger(seed);
    return 1;
}

int builtin::server::builtin_sleep(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ms = (uint32_t)lua->tointeger(1);
    lua->pending(true);

    auto server = lua->env<fb::game::server>("server");
    async::awaitable_then(server->sleep(std::chrono::milliseconds(ms)), [lua](auto result) {
        lua->pending(false);
        lua->resume(0);
    });
    return lua->yield(0);
}

int builtin::server::builtin_baram_time(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    lua->pushstring(baram_time());
    return 1;
}

int builtin::server::builtin_name2mob(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto name   = lua->tostring(1);
    auto mob    = table::mob.name2mob(name);

    if (mob == nullptr)
    {
        lua->pushnil();
    }
    else
    {
        lua->pushobject(mob);
    }
    return 1;
}

int builtin::server::builtin_name2spell(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto name   = lua->tostring(1);
    auto spell  = table::spell.name2spell(name);

    if (spell == nullptr)
    {
        lua->pushnil();
    }
    else
    {
        lua->pushobject(spell);
    }
    return 1;
}

int builtin::server::builtin_name2npc(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto name   = lua->tostring(1);
    auto npc    = table::npc.name2npc(name);

    if (npc == nullptr)
    {
        lua->pushnil();
    }
    else
    {
        lua->pushobject(npc);
    }
    return 1;
}

int builtin::server::builtin_name2map(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto name   = lua->tostring(1);
    auto map    = table::map.name2map(name);

    if (map == nullptr)
    {
        lua->pushnil();
    }
    else
    {
        lua->pushobject(map);
    }
    return 1;
}

int builtin::server::builtin_name2ch(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto name   = lua->tostring(1);

    return server->characters.read([lua, server, name](auto& container) {
        auto ch = container.find(name);
        if (ch == nullptr)
        {
            lua->pushnil();
            return 1;
        }

        auto weak = ch->template weak_from_this_as<character>();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            return lua->ensure_resume(*server, weak, [=]() {
                lua->pushobject(ch);
                return 1;
            });
        });
    });
}

int builtin::server::builtin_name2item(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto name   = lua->tostring(1);
    auto item   = table::item.name2item(name);

    if (item == nullptr)
    {
        lua->pushnil();
    }
    else
    {
        lua->pushobject(item);
    }
    return 1;
}

int builtin::server::builtin_id2mob(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto id     = static_cast<uint32_t>(lua->tointeger(1));
    auto mob    = const_cast<fb::model::mob*>(table::mob.find(id));

    if (mob == nullptr)
        lua->pushnil();
    else
        lua->pushobject(mob);
    return 1;
}

int builtin::server::builtin_id2spell(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto id     = static_cast<uint32_t>(lua->tointeger(1));
    auto spell  = const_cast<fb::model::spell*>(table::spell.find(id));

    if (spell == nullptr)
        lua->pushnil();
    else
        lua->pushobject(spell);
    return 1;
}

int builtin::server::builtin_id2npc(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto id     = static_cast<uint32_t>(lua->tointeger(1));
    auto npc    = const_cast<fb::model::npc*>(table::npc.find(id));

    if (npc == nullptr)
        lua->pushnil();
    else
        lua->pushobject(npc);
    return 1;
}

int builtin::server::builtin_id2map(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto id     = static_cast<uint32_t>(lua->tointeger(1));
    auto map    = const_cast<fb::model::map*>(table::map.find(id));

    if (map == nullptr)
        lua->pushnil();
    else
        lua->pushobject(map);
    return 1;
}

int builtin::server::builtin_id2item(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto id     = static_cast<uint32_t>(lua->tointeger(1));
    auto item   = const_cast<fb::model::item*>(table::item.find(id));

    if (item == nullptr)
        lua->pushnil();
    else
        lua->pushobject(item);
    return 1;
}

int builtin::server::builtin_id2ch(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto id     = static_cast<uint32_t>(lua->tointeger(1));

    return server->characters.read([lua, server, id](auto& container) {
        auto ch = container.find(id);
        if (ch == nullptr)
        {
            lua->pushnil();
            return 0;
        }

        auto weak = ch->template weak_from_this_as<character>();
        return lua->ensure_yield(*server, weak, [=](auto /*is_yield*/) {
            return lua->ensure_resume(*server, weak, [=]() {
                lua->pushobject(ch);
                return 1;
            });
        });
    });
}

int builtin::server::builtin_pursuit_sell(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");

    lua->new_table();
    if (lua->is_nil(1))
        return 1;

    auto pursuit = lua->tointeger(1);
    if (table::sell.contains(pursuit))
    {
        auto& sell = table::sell[pursuit];
        auto  i    = 0;
        for (auto& v : sell)
        {
            auto& item  = table::item[v.item];
            auto  price = v.price.value_or(item.price);

            lua->pushinteger(++i);
            lua->new_table();
            {
                lua->pushinteger(1);
                lua->pushobject(item);
                lua_settable(L, -3);

                lua->pushinteger(2);
                lua->pushinteger(price);
                lua_settable(L, -3);
            }
            lua_settable(L, -3);
        }
    }

    return 1;
}

int builtin::server::builtin_pursuit_sell_price(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server  = lua->env<fb::game::server>("server");
    auto pursuit = lua->tointeger(1);
    auto name    = lua->tostring(2);

    if (table::sell.contains(pursuit) == false)
        return 0;

    for (auto& x : table::sell[pursuit])
    {
        auto& model = table::item[x.item];
        if (model.name != name)
            continue;

        lua->pushinteger(x.price.value_or(model.price));
        return 1;
    }

    return 0;
}

int builtin::server::builtin_pursuit_sell_name(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server  = lua->env<fb::game::server>("server");
    auto pursuit = lua->tointeger(1);

    if (table::sell_attribute.contains(pursuit) == false)
        return 0;

    lua->pushstring(table::sell_attribute[pursuit].group);
    return 1;
}

int builtin::server::builtin_pursuit_buy(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server  = lua->env<fb::game::server>("server");
    auto pursuit = lua->tointeger(1);
    lua->new_table();
    if (table::buy.contains(pursuit))
    {
        auto& buy = table::buy[pursuit];
        auto  i   = 0;
        for (auto& [k, v] : buy)
        {
            auto& item  = table::item[k];
            auto  price = v.price.value_or(item.price / 2);

            lua->pushinteger(++i);
            lua->new_table();
            {
                lua->pushinteger(1);
                lua->pushobject(item);
                lua_settable(L, -3);

                lua->pushinteger(2);
                lua->pushinteger(price);
                lua_settable(L, -3);
            }
            lua_settable(L, -3);
        }
    }

    return 1;
}

int builtin::server::builtin_timer(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server   = lua->env<fb::game::server>("server");
    auto value    = (uint32_t)lua->tointeger(1);
    auto decrease = lua->toboolean(2);

    auto type = decrease ? TIMER_TYPE::DECREASE : TIMER_TYPE::INCREASE;
    server->characters.write([value, type](auto& container) {
        for (auto& [uid, ch] : container)
        {
            ch->timer(value, type);
        }
    });
    return 0;
}

int builtin::server::builtin_weather(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto value  = (uint32_t)lua->tointeger(1);

    server->characters.write([value](auto& container) {
        for (auto& [uid, ch] : container)
        {
            ch->weather(WEATHER_TYPE(value));
        }
    });
    return 0;
}

int builtin::server::builtin_bright(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto value  = (uint32_t)lua->tointeger(1);

    server->characters.write([value](auto& container) {
        for (auto& [uid, ch] : container)
        {
            ch->bright(value);
        }
    });
    return 0;
}

int builtin::server::builtin_name_with(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto argc = lua->argc();
    auto name = lua->tostring(1);

    if (argc < 3)
    {
        lua->pushstring(name_with(name));
    }
    else
    {
        auto first  = lua->tostring(2);
        auto second = lua->tostring(3);
        lua->pushstring(name_with(name, {first, second}));
    }
    return 1;
}

int builtin::server::builtin_assert_korean(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto argc = lua->argc();
    auto text = lua->tostring(1);
    lua->pushboolean(assert_korean(text));
    return 1;
}

int builtin::server::builtin_cp949(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto argc = lua->argc();
    auto text = lua->tostring(1);
    lua->pushstring(CP949(text));
    return 1;
}

int builtin::server::builtin_debug(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

#if defined DEBUG | defined _DEBUG
    lua->pushboolean(true);
#else
    lua->pushboolean(false);
#endif
    return 1;
}

int builtin::server::builtin_name2class(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto name   = lua->tostring(1);

    auto cls       = CLASS::NONE;
    auto promotion = uint8_t{0};
    if (table::promotion.name2class(name, cls, promotion) == false)
        return 0;

    lua->pushinteger(cls);
    lua->pushinteger(promotion);
    return 2;
}

int builtin::server::builtin_class2name(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server    = lua->env<fb::game::server>("server");
    auto cls       = (uint8_t)lua->tointeger(1);
    auto promotion = (uint8_t)lua->tointeger(2);
    auto name      = std::string{};
    if (table::promotion.class2name(static_cast<CLASS>(cls), promotion, name) == false)
        lua->pushnil();
    else
        lua->pushstring(name);

    return 1;
}

int builtin::server::builtin_save(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    server->save();
    return 0;
}

int builtin::server::builtin_mknpc(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto name   = lua->tostring(1);
    auto model  = table::npc.name2npc(name);
    if (model == nullptr)
        return 0;

    std::shared_ptr<fb::game::map> map = nullptr;
    if (lua->is_string(2))
    {
        auto name      = lua->tostring(2);
        auto map_model = table::map.name2map(name);
        if (map_model == nullptr)
            return 0;

        if (server->maps.contains(map_model->id) == false)
            return 0;

        map = server->maps[map_model->id];
    }
    else if (lua->is_userdata<fb::game::map>(2))
    {
        map = lua->touserdata<fb::game::map>(2);
        if (map == nullptr)
            return 0;
    }
    else if (lua->is_userdata<fb::model::map>(2))
    {
        auto map_model = lua->touserdata<fb::model::map>(2);
        if (map_model == nullptr)
            return 0;

        if (server->maps.contains(map_model->id) == false)
            return 0;

        map = server->maps[map_model->id];
    }
    else
    {
        return 0;
    }

    uint16_t x = 0, y = 0;
    auto     direction = DIRECTION::BOTTOM;
    if (argc < 3)
    {
        x = y = 0;
    }
    else if (lua->is_table(3))
    {
        if (argc >= 4)
            direction = static_cast<DIRECTION>(lua->tointeger(4));

        lua->rawgeti(3, 1);
        x = (uint16_t)lua->tointeger(-1);
        lua->remove(-1);

        lua->rawgeti(3, 2);
        y = (uint16_t)lua->tointeger(-1);
        lua->remove(-1);
    }
    else
    {
        if (argc >= 5)
            direction = static_cast<DIRECTION>(lua->tointeger(5));

        x = (uint16_t)lua->tointeger(3);
        y = (uint16_t)lua->tointeger(4);
    }

    if (map->thread()->id() == std::this_thread::get_id())
    {
        auto weak = map->weak_from_this();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            // Use smart pointer for NPC creation
            auto npc = server->make<fb::game::npc>(*model);
            npc->direction(direction);
            npc->map(map, fb::model::point16_t{x, y});

            auto weak = npc->weak_from_this();
            return lua->ensure_resume(*server, weak, [=]() {
                lua->pushobject(npc);
                return 1;
            });
        });
    }
    else
    {
        auto weak = map->weak_from_this();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            // Use smart pointer for NPC creation
            auto npc = server->make<fb::game::npc>(*model);
            npc->direction(direction);
            npc->map(map, fb::model::point16_t{x, y});

            auto weak = npc->weak_from_this();
            return lua->ensure_resume(*server, weak, [=]() {
                lua->pushobject(npc);
                return 1;
            });
        });
    }
}

int builtin::server::builtin_maps(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    lua->new_table();
    auto i = 1;
    for (auto& [id, map] : server->maps)
    {
        lua->pushobject(map);
        lua_rawseti(L, -2, i++);
    }
    return 1;
}

int builtin::server::builtin_shutdown(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    std::ignore = server->http.post("internal", "/system/shutdown", internal_reqs::Shutdown{});
    return 0;
}

int builtin::server::builtin_broadcast(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server     = lua->env<fb::game::server>("server");
    auto argc       = lua->argc();
    auto text       = lua->tostring(1);
    auto type       = lua->toenum(2, MESSAGE_TYPE::STATE);
    auto broad_type = lua->toenum(3, BROADCAST_TYPE::GLOBAL);

    if (broad_type == BROADCAST_TYPE::WORLD)
    {
        std::ignore = server->broadcast(text, type, broad_type);
        return 0;
    }
    else
    {
        async::awaitable_then(server->broadcast(text, type, broad_type), [lua](auto result) {
            lua->resume(0);
        });
        return lua->yield(0);
    }
}

int builtin::server::builtin_assert_alive(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto obj    = lua->touserdata<fb::game::object>(1);

    lua->pushboolean(obj != nullptr);
    return 1;
}

int builtin::server::builtin_ban(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();

    auto ch = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
    {
        lua->pushboolean(false);
        lua->pushstring("Invalid character context");
        return 2;
    }

    auto name   = lua->tostring(2);
    auto reason = lua->tostring(3);
    auto days   = std::optional<uint32_t>{std::nullopt};
    if (argc >= 4 && lua->is_nil(4) == false && lua->is_number(4))
        days = static_cast<uint32_t>(lua->tointeger(4));

    static auto fn = [](fb::game::server*              server,
                        fb::lua::context*              lua,
                        const std::string&             name,
                        const std::string&             reason,
                        const std::optional<uint32_t>& days) -> async::task<void> {
        auto   success = false;
        auto   error   = std::string{};
        auto&& resp    = co_await server->ban(name, reason, days);
        if (resp.error == 0)
        {
            success = true;
        }
        else
        {
            error = std::format("Ban failed with error code: {}", resp.error);
        }

        co_await lua->switching();
        lua->pushboolean(success);
        if (!success)
            lua->pushstring(error);
        lua->resume(success ? 1 : 2);
    };

    async::awaitable_then(fn(server, lua, name, reason, days), [lua](auto result) {
        result();
    });

    return lua->yield(1);
}

int builtin::server::builtin_unban(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();

    auto ch = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
    {
        lua->pushboolean(false);
        lua->pushstring("Invalid character context");
        return 2;
    }

    auto name = lua->tostring(2);

    static auto fn = [](fb::game::server* server, fb::lua::context* lua, const std::string& name) -> async::task<void> {
        auto   success = false;
        auto   error   = std::string{};
        auto&& resp    = co_await server->unban(name);
        if (resp.error == 0)
        {
            success = true;
        }
        else
        {
            error = std::format("Unban failed with error code: {}", resp.error);
        }

        co_await lua->switching();
        lua->pushboolean(success);
        if (!success)
            lua->pushstring(error);
        lua->resume(success ? 1 : 2);
    };

    async::awaitable_then(fn(server, lua, name), [lua](auto result) {
        result();
    });

    return lua->yield(1);
}

int builtin::server::builtin_regex(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto argc = lua->argc();
    if (argc < 2)
    {
        lua->pushnil();
        return 1;
    }

    auto pattern = lua->tostring(1);
    auto message = lua->tostring(2);

    if (pattern.empty() || message.empty())
    {
        lua->pushnil();
        return 1;
    }

    static thread_local auto regex_cache = std::unordered_map<std::string, boost::xpressive::sregex>{};
    auto                     it          = regex_cache.find(pattern);
    if (it == regex_cache.end())
    {
        regex_cache[pattern] = boost::xpressive::sregex::compile(pattern);
        it                   = regex_cache.find(pattern);
    }
    auto& regex = it->second;

    auto what = boost::xpressive::smatch();
    if (boost::xpressive::regex_search(message, what, regex) == false)
    {
        lua->pushnil();
        return 1;
    }

    lua->new_table();

    std::regex           named_group_regex(R"((\?P<(\w+)>))");
    std::sregex_iterator iter(pattern.begin(), pattern.end(), named_group_regex);
    std::sregex_iterator end;

    while (iter != end)
    {
        auto  match      = *iter;
        auto  group_name = match[2].str();
        auto& sub        = what[group_name];
        if (sub.matched)
        {
            lua->pushstring(group_name.c_str());
            lua->pushstring(sub.str().c_str());
            lua_settable(L, -3);
        }
        ++iter;
    }

    return 1;
}

int builtin::server::builtin_exp_multiplier(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();

    if (argc == 0)
    {
        // Get multiplier
        auto multiplier = server->exp_multiplier();
        lua->pushnumber(multiplier);
        return 1;
    }
    else
    {
        // Set multiplier - send request to internal server
        auto value = lua->tonumber(1);

        static auto fn = [](fb::game::server* server, fb::lua::context* lua, double value) -> async::task<void> {
            auto   success = false;
            auto   error   = std::string{};
            auto world = fb::config<uint32_t>("world");
            auto&& resp    = co_await server->http.post("internal",
                                                     "/in-game/set-exp-multiplier",
                                                     internal_reqs::SetExpMultiplier{world, value});
            if (resp.error == 0)
            {
                success = true;
            }
            else
            {
                error = std::format("Set exp multiplier failed with error code: {}", resp.error);
            }

            co_await lua->switching();
            lua->pushboolean(success);
            if (!success)
                lua->pushstring(error);
            lua->resume(success ? 1 : 2);
        };

        async::awaitable_then(fn(server, lua, value), [lua](auto result) {
            result();
        });

        return lua->yield(1);
    }
}

int builtin::server::builtin_drop_rate_multiplier(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();

    if (argc == 0)
    {
        // Get multiplier
        auto multiplier = server->drop_rate_multiplier();
        lua->pushnumber(multiplier);
        return 1;
    }
    else
    {
        // Set multiplier - send request to internal server
        auto value = lua->tonumber(1);

        static auto fn = [](fb::game::server* server, fb::lua::context* lua, double value) -> async::task<void> {
            auto   success = false;
            auto   error   = std::string{};
            auto world = fb::config<uint32_t>("world");
            auto&& resp    = co_await server->http.post("internal",
                                                     "/in-game/set-drop-rate-multiplier",
                                                     internal_reqs::SetDropRateMultiplier{world, value});

            lua->pushboolean(resp.error == 0);
            if (resp.error == 0)
            {
                success = true;
            }
            else
            {
                error = std::format("Set drop rate multiplier failed with error code: {}", resp.error);
            }

            co_await lua->switching();
            lua->pushboolean(success);
            if (!success)
                lua->pushstring(error);
            lua->resume(success ? 1 : 2);
        };

        async::awaitable_then(fn(server, lua, value), [lua](auto result) {
            result();
        });

        return lua->yield(1);
    }
}