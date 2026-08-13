#include <fb/game/builtin/server.h>
#include <fb/lua.h>
#include <fb/encoding.h>
#include <json/json.h>
#include <boost/xpressive/xpressive.hpp>
#include <chrono>
#include <unordered_map>
#include <regex>
#include <string_view>
#include <fb/model/datetime.h>
#include <tuple>
#include <format>
#include <stdexcept>

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

    auto ms       = (uint32_t)lua->tointeger(1);
    auto builder  = lua->new_co_builder();
    builder.yield = [=]() -> async::task<void> {
        auto& server = static_cast<fb::game::server&>(lua->executor);
        co_await server.sleep(std::chrono::milliseconds(ms));
    };
    builder.resume = []() -> async::task<int> {
        co_return 0;
    };
    return builder.run();
}

/**
 * @brief      Returns current time as seconds since epoch (for time comparisons and cooldowns).
 *
 *             Takes no arguments. Pushes a single integer. Use with 용왕, 상어장군, 사천족제사장
 *             etc. for logic like now() + delay, now() < end_time.
 *
 * @param[in]  L     Lua state.
 * @return     Number of return values (1 integer).
 */
int builtin::server::builtin_now(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto& srv  = static_cast<fb::game::server&>(lua->executor);
    auto  argc = lua->argc();

    if (argc == 0)
    {
        auto offset = srv.now_offset();
        auto now_c  = std::chrono::system_clock::now() + std::chrono::milliseconds(offset.total_milliseconds());
        auto sec_since_epoch = std::chrono::duration_cast<std::chrono::seconds>(now_c.time_since_epoch()).count();
        lua->pushinteger(static_cast<lua_Integer>(sec_since_epoch));
        return 1;
    }
    else
    {
        auto value = lua->tostring(1);
        auto reset = false;
        if (value == "reset")
        {
            reset = true;
            value = "";
        }

        if (value.empty() && !reset)
        {
            lua->pushboolean(false);
            lua->pushstring("datetime is required");
            return 2;
        }

        auto success  = std::make_shared<bool>(false);
        auto error    = std::make_shared<std::string>();
        auto builder  = lua->new_co_builder();
        builder.yield = [=]() -> async::task<void> {
            auto&  server = static_cast<fb::game::server&>(lua->executor);
            auto   world  = fb::config<uint32_t>("world");
            auto&& resp   = co_await server.http.post("internal",
                                                    "/in-game/set-datetime",
                                                    internal_reqs::SetDateTime{world, value, reset});
            if (resp.error == 0)
                *success = true;
            else
                *error = std::format("Set datetime failed with error code: {}", resp.error);
        };
        builder.resume = [=]() -> async::task<int> {
            lua->pushboolean(*success);
            if (*success == false)
                lua->pushstring(*error);
            co_return *success ? 1 : 2;
        };
        return builder.run();
    }
}

int builtin::server::builtin_time_forward(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto& srv   = static_cast<fb::game::server&>(lua->executor);
    auto  value = lua->tostring(1);
    if (value.empty())
    {
        lua->pushboolean(false);
        lua->pushstring("timespan is required");
        return 2;
    }

    fb::model::timespan delta;
    try
    {
        delta = fb::model::timespan(value);
    }
    catch (const std::exception& e)
    {
        lua->pushboolean(false);
        lua->pushstring(e.what());
        return 2;
    }

    auto target   = srv.now() + delta;
    auto datetime = target.to_string();
    auto success  = std::make_shared<bool>(false);
    auto error    = std::make_shared<std::string>();
    auto builder  = lua->new_co_builder();
    builder.yield = [=]() -> async::task<void> {
        auto& server = static_cast<fb::game::server&>(lua->executor);
        auto  world  = fb::config<uint32_t>("world");
        auto  resp   = co_await server.http.post("internal",
                                              "/in-game/set-datetime",
                                              internal_reqs::SetDateTime{world, datetime, false});
        if (resp.error == 0)
            *success = true;
        else
            *error = std::format("Set datetime failed with error code: {}", resp.error);
    };
    builder.resume = [=]() -> async::task<int> {
        lua->pushboolean(*success);
        if (*success == false)
            lua->pushstring(*error);
        co_return *success ? 1 : 2;
    };
    return builder.run();
}

int builtin::server::builtin_time_backward(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto& srv   = static_cast<fb::game::server&>(lua->executor);
    auto  value = lua->tostring(1);
    if (value.empty())
    {
        lua->pushboolean(false);
        lua->pushstring("timespan is required");
        return 2;
    }

    fb::model::timespan delta;
    try
    {
        delta = fb::model::timespan(value);
    }
    catch (const std::exception& e)
    {
        lua->pushboolean(false);
        lua->pushstring(e.what());
        return 2;
    }

    auto target   = srv.now() - delta;
    auto datetime = target.to_string();
    auto success  = std::make_shared<bool>(false);
    auto error    = std::make_shared<std::string>();
    auto builder  = lua->new_co_builder();
    builder.yield = [=]() -> async::task<void> {
        auto& server = static_cast<fb::game::server&>(lua->executor);
        auto  world  = fb::config<uint32_t>("world");
        auto  resp   = co_await server.http.post("internal",
                                              "/in-game/set-datetime",
                                              internal_reqs::SetDateTime{world, datetime, false});
        if (resp.error == 0)
            *success = true;
        else
            *error = std::format("Set datetime failed with error code: {}", resp.error);
    };
    builder.resume = [=]() -> async::task<int> {
        lua->pushboolean(*success);
        if (*success == false)
            lua->pushstring(*error);
        co_return *success ? 1 : 2;
    };
    return builder.run();
}

namespace {

void push_datetime_table(lua_State* L, const fb::model::datetime& dt)
{
    lua_createtable(L, 0, 10);
    lua_pushinteger(L, static_cast<lua_Integer>(dt.year()));
    lua_setfield(L, -2, "year");
    lua_pushinteger(L, static_cast<lua_Integer>(dt.month()));
    lua_setfield(L, -2, "month");
    lua_pushinteger(L, static_cast<lua_Integer>(dt.day()));
    lua_setfield(L, -2, "day");
    lua_pushinteger(L, static_cast<lua_Integer>(dt.hours()));
    lua_setfield(L, -2, "hour");
    lua_pushinteger(L, static_cast<lua_Integer>(dt.minutes()));
    lua_setfield(L, -2, "minute");
    lua_pushinteger(L, static_cast<lua_Integer>(dt.seconds()));
    lua_setfield(L, -2, "second");
    try
    {
        auto lunar = dt.to_lunar();
        lua_pushinteger(L, static_cast<lua_Integer>(lunar.year));
        lua_setfield(L, -2, "lunar_year");
        lua_pushinteger(L, static_cast<lua_Integer>(lunar.month));
        lua_setfield(L, -2, "lunar_month");
        lua_pushinteger(L, static_cast<lua_Integer>(lunar.day));
        lua_setfield(L, -2, "lunar_day");
        lua_pushboolean(L, lunar.leap ? 1 : 0);
        lua_setfield(L, -2, "lunar_leap");
    }
    catch (const std::exception&)
    {
        lua_pushnil(L);
        lua_setfield(L, -2, "lunar_year");
        lua_pushnil(L);
        lua_setfield(L, -2, "lunar_month");
        lua_pushnil(L);
        lua_setfield(L, -2, "lunar_day");
        lua_pushnil(L);
        lua_setfield(L, -2, "lunar_leap");
    }
}

void push_lunar_table(lua_State* L, const fb::model::lunar_date& lunar)
{
    lua_createtable(L, 0, 4);
    lua_pushinteger(L, static_cast<lua_Integer>(lunar.year));
    lua_setfield(L, -2, "year");
    lua_pushinteger(L, static_cast<lua_Integer>(lunar.month));
    lua_setfield(L, -2, "month");
    lua_pushinteger(L, static_cast<lua_Integer>(lunar.day));
    lua_setfield(L, -2, "day");
    lua_pushboolean(L, lunar.leap ? 1 : 0);
    lua_setfield(L, -2, "leap");
}

fb::model::datetime datetime_from_lua_table(fb::lua::context& lua, int index)
{
    lua_getfield(lua, index, "year");
    auto year = static_cast<uint16_t>(lua_tointeger(lua, -1));
    lua_pop(lua, 1);
    lua_getfield(lua, index, "month");
    auto month = static_cast<uint16_t>(lua_tointeger(lua, -1));
    lua_pop(lua, 1);
    lua_getfield(lua, index, "day");
    auto day = static_cast<uint16_t>(lua_tointeger(lua, -1));
    lua_pop(lua, 1);

    auto hour   = uint16_t{0};
    auto minute = uint16_t{0};
    auto second = uint16_t{0};
    lua_getfield(lua, index, "hour");
    if (!lua_isnil(lua, -1))
        hour = static_cast<uint16_t>(lua_tointeger(lua, -1));
    lua_pop(lua, 1);
    lua_getfield(lua, index, "minute");
    if (!lua_isnil(lua, -1))
        minute = static_cast<uint16_t>(lua_tointeger(lua, -1));
    lua_pop(lua, 1);
    lua_getfield(lua, index, "second");
    if (!lua_isnil(lua, -1))
        second = static_cast<uint16_t>(lua_tointeger(lua, -1));
    lua_pop(lua, 1);

    return fb::model::datetime(
        std::format("{:04}-{:02}-{:02} {:02}:{:02}:{:02}", year, month, day, hour, minute, second));
}

fb::model::lunar_date lunar_from_lua_table(fb::lua::context& lua, int index)
{
    auto lunar = fb::model::lunar_date{};
    lua_getfield(lua, index, "year");
    lunar.year = static_cast<uint16_t>(lua_tointeger(lua, -1));
    lua_pop(lua, 1);
    lua_getfield(lua, index, "month");
    lunar.month = static_cast<uint16_t>(lua_tointeger(lua, -1));
    lua_pop(lua, 1);
    lua_getfield(lua, index, "day");
    lunar.day = static_cast<uint16_t>(lua_tointeger(lua, -1));
    lua_pop(lua, 1);
    lua_getfield(lua, index, "leap");
    lunar.leap = lua_toboolean(lua, -1) != 0;
    lua_pop(lua, 1);
    return lunar;
}

} // namespace

/**
 * @brief      Returns current date/time as a table (server local time) for calendar-style logic.
 *
 *             Takes no arguments. Pushes a single table with fields: year, month, day,
 *             hour, minute, second. Use for 선원/뱃사공 time windows (e.g. datetime().hour, datetime().minute).
 *
 * @param[in]  L     Lua state.
 * @return     Number of return values (1 table).
 */
int builtin::server::builtin_datetime(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto& srv = static_cast<fb::game::server&>(lua->executor);
    push_datetime_table(L, srv.now());
    return 1;
}

int builtin::server::builtin_to_lunar(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    if (lua->argc() < 1 || !lua->is_table(1))
    {
        lua->pushnil();
        lua->pushstring("datetime table required (e.g. to_lunar(datetime()))");
        return 2;
    }

    try
    {
        auto solar = datetime_from_lua_table(*lua, 1);
        push_lunar_table(L, solar.to_lunar());
        return 1;
    }
    catch (const std::exception& e)
    {
        lua->pushnil();
        lua->pushstring(e.what());
        return 2;
    }
}

int builtin::server::builtin_from_lunar(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    if (lua->argc() < 1 || !lua->is_table(1))
    {
        lua->pushnil();
        lua->pushstring("lunar table {year,month,day[,leap]} required");
        return 2;
    }

    try
    {
        auto lunar   = lunar_from_lua_table(*lua, 1);
        auto hours   = static_cast<uint16_t>(lua->tointeger(2, 0));
        auto minutes = static_cast<uint16_t>(lua->tointeger(3, 0));
        auto seconds = static_cast<uint16_t>(lua->tointeger(4, 0));
        auto solar   = fb::model::datetime::from_lunar(lunar, hours, minutes, seconds);
        push_datetime_table(L, solar);
        return 1;
    }
    catch (const std::exception& e)
    {
        lua->pushnil();
        lua->pushstring(e.what());
        return 2;
    }
}

int builtin::server::builtin_name2mob(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto name = lua->tostring(1);
    auto mob  = table::mob->name2mob(name);

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

    auto name  = lua->tostring(1);
    auto spell = table::spell->name2spell(name);

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

    auto name = lua->tostring(1);
    auto npc  = table::npc->name2npc(name);

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

    auto& srv   = static_cast<fb::game::server&>(lua->executor);
    auto  name  = lua->tostring(1);
    auto  model = table::map->name2map(name);
    if (model == nullptr)
    {
        lua->pushnil();
        return 1;
    }

    auto map = srv.maps.find(model->id);
    if (map == nullptr)
        lua->pushnil();
    else
        lua->pushobject(map);
    return 1;
}

int builtin::server::builtin_name2ch(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto& srv  = static_cast<fb::game::server&>(lua->executor);
    auto  argc = lua->argc();
    auto  name = lua->tostring(1);

    character::container::character_ptr_t ch = srv.characters.find(name);
    if (ch == nullptr)
    {
        lua->pushnil();
        return 1;
    }

    auto weak      = ch->template weak_from_this_as<character>();
    auto ch_holder = std::make_shared<std::shared_ptr<character>>();
    auto builder   = lua->new_co_builder();
    builder.weak   = weak;
    builder.yield  = [=]() -> async::task<void> {
        *ch_holder = weak.lock();
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        if (*ch_holder == nullptr)
            lua->pushnil();
        else
            lua->pushobject(*ch_holder);
        co_return 1;
    };
    return builder.run();
}

int builtin::server::builtin_name2item(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto name = lua->tostring(1);
    auto item = table::item->name2item(name);

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

    auto id  = static_cast<uint32_t>(lua->tointeger(1));
    auto mob = const_cast<fb::model::mob*>(table::mob->find(id));

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

    auto id    = static_cast<uint32_t>(lua->tointeger(1));
    auto spell = const_cast<fb::model::spell*>(table::spell->find(id));

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

    auto id  = static_cast<uint32_t>(lua->tointeger(1));
    auto npc = const_cast<fb::model::npc*>(table::npc->find(id));

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

    auto& srv = static_cast<fb::game::server&>(lua->executor);
    auto  id  = static_cast<uint32_t>(lua->tointeger(1));
    auto  map = srv.maps.find(id);

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

    auto id   = static_cast<uint32_t>(lua->tointeger(1));
    auto item = const_cast<fb::model::item*>(table::item->find(id));

    if (item == nullptr)
        lua->pushnil();
    else
        lua->pushobject(item);
    return 1;
}

int builtin::server::builtin_id2clan(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto& srv = static_cast<fb::game::server&>(lua->executor);
    auto  id  = static_cast<uint32_t>(lua->tointeger(1));

    auto guard = srv.clans.try_enter_read(id);
    if (guard.has_value() == false || guard->value() == nullptr)
        lua->pushnil();
    else
        lua->pushobject(guard->value());

    return 1;
}

int builtin::server::builtin_castle(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto divine_beast = static_cast<uint32_t>(lua->tointeger(1));
    if (divine_beast > static_cast<uint32_t>(fb::model::enum_value::DIVINE_BEAST::AZURE_DRAGON))
    {
        lua->pushnil();
        return 1;
    }

    auto& srv = static_cast<fb::game::server&>(lua->executor);

    // Fast path: already cached in this process.
    {
        auto guard = srv.castles.try_enter_read(divine_beast);
        if (guard.has_value() && guard->value() != nullptr)
        {
            lua->pushobject(guard->value());
            return 1;
        }
    }

    // Slow path: lazy-load from internal/DB via ensure().
    auto holder   = std::make_shared<std::shared_ptr<fb::game::castle>>();
    auto builder  = lua->new_co_builder();
    builder.yield = [=]() -> async::task<void> {
        auto& server = static_cast<fb::game::server&>(lua->executor);
        try
        {
            auto guard = co_await server.castles.ensure(divine_beast);
            *holder    = guard.value();
        }
        catch (std::exception& e)
        {
            fb::logger::warn("castle ensure failed (id: {}): {}", divine_beast, e.what());
        }
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        if (*holder == nullptr)
            lua->pushnil();
        else
            lua->pushobject(*holder);
        co_return 1;
    };
    return builder.run();
}

int builtin::server::builtin_siege_active(lua_State* L)
{
    // Sync-only: never yields. Used from attack/PK hot paths.
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto divine_beast = static_cast<uint32_t>(lua->tointeger(1));
    if (divine_beast > static_cast<uint32_t>(fb::model::enum_value::DIVINE_BEAST::AZURE_DRAGON))
    {
        lua->pushboolean(false);
        return 1;
    }

    auto& srv   = static_cast<fb::game::server&>(lua->executor);
    auto  guard = srv.castles.try_enter_read(divine_beast);
    if (guard.has_value() == false || guard->value() == nullptr)
    {
        lua->pushboolean(false);
        return 1;
    }

    lua->pushboolean(guard->value()->siege_active());
    return 1;
}

int builtin::server::builtin_id2ch(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto& srv = static_cast<fb::game::server&>(lua->executor);
    auto  id  = static_cast<uint32_t>(lua->tointeger(1));

    character::container::character_ptr_t ch = srv.characters.find(id);
    if (ch == nullptr)
    {
        lua->pushnil();
        return 0;
    }

    auto weak      = ch->template weak_from_this_as<character>();
    auto ch_holder = std::make_shared<std::shared_ptr<character>>();
    auto builder   = lua->new_co_builder();
    builder.weak   = weak;
    builder.yield  = [=]() -> async::task<void> {
        *ch_holder = weak.lock();
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        if (*ch_holder == nullptr)
            lua->pushnil();
        else
            lua->pushobject(*ch_holder);
        co_return 1;
    };
    return builder.run();
}

int builtin::server::builtin_pursuit_sell(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;
    lua->new_table();
    if (lua->is_nil(1))
        return 1;

    auto pursuit = lua->tointeger(1);
    if (table::sell->contains(pursuit))
    {
        auto  sell_table = table::sell;
        auto& sell       = sell_table[pursuit];
        auto  i          = 0;
        for (auto& v : sell)
        {
            auto  item_table = table::item;
            auto& item       = item_table[v.item];
            auto  price      = v.price.value_or(item.price);

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

    auto pursuit = lua->tointeger(1);
    auto name    = lua->tostring(2);

    if (table::sell->contains(pursuit) == false)
        return 0;

    for (auto& x : table::sell[pursuit])
    {
        auto  item_table2 = table::item;
        auto& model       = item_table2[x.item];
        if (model.name != name)
            continue;

        lua->pushinteger(x.price.value_or(model.price));
        return 1;
    }

    return 0;
}

int builtin::server::builtin_pursuit_buy(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto pursuit = lua->tointeger(1);
    lua->new_table();
    if (table::buy->contains(pursuit))
    {
        auto  buy_table = table::buy;
        auto& buy       = buy_table[pursuit];
        auto  i         = 0;
        for (auto& [k, v] : buy)
        {
            auto  item_table3 = table::item;
            auto& item        = item_table3[k];
            auto  price       = v.price.value_or(item.price / 2);

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

    auto& srv      = static_cast<fb::game::server&>(lua->executor);
    auto  value    = (uint32_t)lua->tointeger(1);
    auto  decrease = lua->toboolean(2);

    auto type = decrease ? TIMER_TYPE::DECREASE : TIMER_TYPE::INCREASE;
    srv.characters.foreach_enqueue([value, type](auto& ch) -> async::task<void> {
        ch->timer(value, type);
        co_return;
    });
    return 0;
}

int builtin::server::builtin_weather(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto& srv   = static_cast<fb::game::server&>(lua->executor);
    auto  value = (uint32_t)lua->tointeger(1);

    srv.characters.foreach_enqueue([value](auto& ch) -> async::task<void> {
        ch->weather(WEATHER_TYPE(value));
        co_return;
    });
    return 0;
}

int builtin::server::builtin_bright(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto& srv   = static_cast<fb::game::server&>(lua->executor);
    auto  value = (uint32_t)lua->tointeger(1);

    srv.characters.foreach_enqueue([value](auto& ch) -> async::task<void> {
        ch->bright(value);
        co_return;
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

    auto text      = lua->tostring(1);
    auto completed = lua->toboolean(2, true);
    lua->pushboolean(assert_korean(text, completed));
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

#if defined DEBUG || defined _DEBUG
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

    auto name = lua->tostring(1);

    auto cls       = CLASS::NONE;
    auto promotion = uint8_t{0};
    if (table::promotion->name2class(name, cls, promotion) == false)
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

    auto cls       = (uint8_t)lua->tointeger(1);
    auto promotion = (uint8_t)lua->tointeger(2);
    auto name      = std::string{};
    if (table::promotion->class2name(static_cast<CLASS>(cls), promotion, name) == false)
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

    auto builder  = lua->new_co_builder();
    builder.yield = [=]() -> async::task<void> {
        auto& server = static_cast<fb::game::server&>(lua->executor);
        co_await server.save();
        co_return;
    };
    builder.resume = []() -> async::task<int> {
        co_return 0;
    };
    return builder.run();
}

int builtin::server::builtin_reload_table(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    // Do not reload on the logic thread. Broadcast via internal HTTP → AMQP,
    // same pattern as exp_multiplier; each host offloads build to a background thread.
    auto success  = std::make_shared<bool>(false);
    auto error    = std::make_shared<std::string>();
    auto builder  = lua->new_co_builder();
    builder.yield = [=]() -> async::task<void> {
        auto& server = static_cast<fb::game::server&>(lua->executor);
        auto  world  = fb::config<uint32_t>("world");
        auto  resp =
            co_await server.http.post("internal", "/in-game/reload-tables", internal_reqs::ReloadTables(world, "", {}));
        if (resp.error == 0)
            *success = true;
        else
            *error = std::format("Reload tables failed with error code: {}", resp.error);
    };
    builder.resume = [=]() -> async::task<int> {
        lua->pushboolean(*success);
        if (*success == false)
            lua->pushstring(*error);
        co_return *success ? 1 : 2;
    };
    return builder.run();
}

int builtin::server::builtin_mknpc(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto& srv   = static_cast<fb::game::server&>(lua->executor);
    auto  argc  = lua->argc();
    auto  name  = lua->tostring(1);
    auto  model = table::npc->name2npc(name);
    if (model == nullptr)
        return 0;

    std::shared_ptr<fb::game::map> map = nullptr;
    if (lua->is_string(2))
    {
        auto name      = lua->tostring(2);
        auto map_model = table::map->name2map(name);
        if (map_model == nullptr)
            return 0;

        if (srv.maps.contains(map_model->id) == false)
            return 0;

        map = srv.maps[map_model->id];
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

        if (srv.maps.contains(map_model->id) == false)
            return 0;

        map = srv.maps[map_model->id];
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

    auto weak       = map->weak_from_this();
    auto npc_holder = std::make_shared<std::shared_ptr<fb::game::npc>>();
    auto builder    = lua->new_co_builder();
    builder.weak    = weak;
    builder.yield   = [=]() -> async::task<void> {
        auto& server = static_cast<fb::game::server&>(lua->executor);
        auto  npc    = server.make<fb::game::npc>(*model);
        std::ignore  = npc->direction(direction);
        std::ignore  = co_await npc->map(map, fb::model::point16_t{x, y});
        *npc_holder  = npc;
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        lua->pushobject(*npc_holder);
        co_return 1;
    };
    return builder.run();
}

int builtin::server::builtin_maps(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto& srv = static_cast<fb::game::server&>(lua->executor);
    lua->new_table();
    auto i = 1;
    for (auto& map : *srv.maps.snapshot())
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

    auto& srv   = static_cast<fb::game::server&>(lua->executor);
    std::ignore = srv.http.post("internal", "/system/shutdown", internal_reqs::Shutdown{});
    return 0;
}

int builtin::server::builtin_broadcast(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto& srv          = static_cast<fb::game::server&>(lua->executor);
    auto  text         = lua->tostring(1);
    auto  type         = lua->toenum(2, MESSAGE_TYPE::STATE);
    auto  broad_type   = lua->toenum(3, BROADCAST_TYPE::GLOBAL);
    auto  respect_roar = false;

    if (lua->argc() >= 4 && lua->is_table(4))
    {
        lua->pushstring("respect_roar");
        if (lua_rawget(L, 4) == LUA_TBOOLEAN)
            respect_roar = lua_toboolean(L, -1);
        lua_pop(L, 1);
    }

    if (broad_type == BROADCAST_TYPE::WORLD)
    {
        srv.characters.broadcast(text, type, respect_roar);
        return 0;
    }
    else
    {
        auto builder  = lua->new_co_builder();
        builder.yield = [=]() -> async::task<void> {
            auto& server = static_cast<fb::game::server&>(lua->executor);
            co_await server.characters.broadcast(text, type, broad_type, respect_roar);
            co_return;
        };
        builder.resume = []() -> async::task<int> {
            co_return 0;
        };
        return builder.run();
    }
}

int builtin::server::builtin_assert_alive(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto argc = lua->argc();
    auto obj  = lua->touserdata<fb::game::object>(1);

    lua->pushboolean(obj != nullptr);
    return 1;
}

int builtin::server::builtin_ban(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto argc = lua->argc();

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

    auto success  = std::make_shared<bool>(false);
    auto error    = std::make_shared<std::string>();
    auto builder  = lua->new_co_builder();
    builder.yield = [=]() -> async::task<void> {
        auto&  server = static_cast<fb::game::server&>(lua->executor);
        auto&& resp   = co_await server.ban(name, reason, days);
        if (resp.error == 0)
            *success = true;
        else
            *error = std::format("Ban failed with error code: {}", resp.error);
    };
    builder.resume = [=]() -> async::task<int> {
        lua->pushboolean(*success);
        if (*success == false)
            lua->pushstring(*error);
        co_return *success ? 1 : 2;
    };
    return builder.run();
}

int builtin::server::builtin_unban(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto argc = lua->argc();

    auto ch = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
    {
        lua->pushboolean(false);
        lua->pushstring("Invalid character context");
        return 2;
    }

    auto name = lua->tostring(2);

    auto success  = std::make_shared<bool>(false);
    auto error    = std::make_shared<std::string>();
    auto builder  = lua->new_co_builder();
    builder.yield = [=]() -> async::task<void> {
        auto&  server = static_cast<fb::game::server&>(lua->executor);
        auto&& resp   = co_await server.unban(name);
        if (resp.error == 0)
            *success = true;
        else
            *error = std::format("Unban failed with error code: {}", resp.error);
    };
    builder.resume = [=]() -> async::task<int> {
        lua->pushboolean(*success);
        if (*success == false)
            lua->pushstring(*error);
        co_return *success ? 1 : 2;
    };
    return builder.run();
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

    auto& srv  = static_cast<fb::game::server&>(lua->executor);
    auto  argc = lua->argc();

    if (argc == 0)
    {
        // Get multiplier
        auto multiplier = srv.exp_multiplier();
        lua->pushnumber(multiplier);
        return 1;
    }
    else
    {
        // Set multiplier - send request to internal server
        const double multiplier = static_cast<double>(lua->tonumber(1));

        auto success  = std::make_shared<bool>(false);
        auto error    = std::make_shared<std::string>();
        auto builder  = lua->new_co_builder();
        builder.yield = [=]() -> async::task<void> {
            auto& server = static_cast<fb::game::server&>(lua->executor);
            auto  world  = fb::config<uint32_t>("world");
            auto  resp   = co_await server.http.post("internal",
                                                  "/in-game/set-exp-multiplier",
                                                  internal_reqs::SetExpMultiplier(world, multiplier));
            if (resp.error == 0)
                *success = true;
            else
                *error = std::format("Set exp multiplier failed with error code: {}", resp.error);
        };
        builder.resume = [=]() -> async::task<int> {
            lua->pushboolean(*success);
            if (*success == false)
                lua->pushstring(*error);
            co_return *success ? 1 : 2;
        };
        return builder.run();
    }
}

int builtin::server::builtin_drop_rate_multiplier(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto& srv  = static_cast<fb::game::server&>(lua->executor);
    auto  argc = lua->argc();

    if (argc == 0)
    {
        // Get multiplier
        auto multiplier = srv.drop_rate_multiplier();
        lua->pushnumber(multiplier);
        return 1;
    }
    else
    {
        // Set multiplier - send request to internal server
        const double multiplier = static_cast<double>(lua->tonumber(1));

        auto success  = std::make_shared<bool>(false);
        auto error    = std::make_shared<std::string>();
        auto builder  = lua->new_co_builder();
        builder.yield = [=]() -> async::task<void> {
            auto& server = static_cast<fb::game::server&>(lua->executor);
            auto  world  = fb::config<uint32_t>("world");
            auto  resp   = co_await server.http.post("internal",
                                                  "/in-game/set-drop-rate-multiplier",
                                                  internal_reqs::SetDropRateMultiplier(world, multiplier));
            if (resp.error == 0)
                *success = true;
            else
                *error = std::format("Set drop rate multiplier failed with error code: {}", resp.error);
        };
        builder.resume = [=]() -> async::task<int> {
            lua->pushboolean(*success);
            if (*success == false)
                lua->pushstring(*error);
            co_return *success ? 1 : 2;
        };
        return builder.run();
    }
}

int builtin::server::builtin_http_response_delay(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto& srv  = static_cast<fb::game::server&>(lua->executor);
    auto  argc = lua->argc();

    if (argc == 0)
    {
        lua->pushinteger(srv.http.response_delay().total_milliseconds());
        return 1;
    }

    auto ms = static_cast<int64_t>(lua->tointeger(1));
    if (ms < 0)
        ms = 0;

    srv.http.response_delay(fb::model::timespan(std::chrono::milliseconds(ms)));
    lua->pushinteger(srv.http.response_delay().total_milliseconds());
    return 1;
}

int builtin::server::builtin_property(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto& srv = static_cast<fb::game::server&>(lua->executor);

    auto argc = lua->argc();
    if (argc < 1)
        return 0;

    auto key = lua->tostring(1);
    if (key.empty())
        return 0;

    if (argc == 1)
    {
        Json::Value value;
        bool        found = false;
        {
            auto guard = srv.property.enter_read();
            auto it    = guard.value().find(key);
            if (it != guard.value().end())
            {
                value = it->second;
                found = true;
            }
        }

        if (found == false)
        {
            lua->pushnil();
            return 1;
        }

        if (value.isString())
            lua->pushstring(value.asString());
        else if (value.isDouble() || value.isInt())
            lua->pushnumber(value.asDouble());
        else if (value.isBool())
            lua->pushboolean(value.asBool());
        else
            lua->pushnil();
        return 1;
    }

    if (argc >= 2)
    {
        int t = lua_type(L, 2);
        if (t == LUA_TSTRING)
        {
            const char* s = lua_tostring(L, 2);
            Json::Value val(s ? s : "");
            {
                auto guard         = srv.property.enter_write();
                guard.value()[key] = val;
            }
        }
        else if (t == LUA_TNUMBER)
        {
            double n = lua_tonumber(L, 2);
            {
                auto guard         = srv.property.enter_write();
                guard.value()[key] = Json::Value(n);
            }
        }
        else if (t == LUA_TBOOLEAN)
        {
            bool b = lua_toboolean(L, 2) != 0;
            {
                auto guard         = srv.property.enter_write();
                guard.value()[key] = Json::Value(b);
            }
        }
    }
    return 0;
}
