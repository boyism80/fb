#include <fb/game/server.h>
#include <fb/game/builtin/object.h>
#include <fb/game/builtin/life.h>
#include <fb/game/map.h>
#include <tuple>

using namespace fb::game;
using namespace fb::model;

// clang-format off
IMPLEMENT_LUA_EXTENSION(fb::game::life, "fb.game.life")
{"__eq",                 builtin::object::builtin_eq},
{"message",              builtin::life::builtin_message},
{"hp",                   builtin::life::builtin_hp},
{"mp",                   builtin::life::builtin_mp},
{"heal",                 builtin::life::builtin_heal},
{"damage_to",            builtin::life::builtin_damage_to},
{"mp_up",                builtin::life::builtin_mp_up},
{"mp_down",              builtin::life::builtin_mp_down},
{"action",               builtin::life::builtin_action},
{"spell",                builtin::life::builtin_spell},
{"spells",               builtin::life::builtin_spells},
{"cast",                 builtin::life::builtin_cast},
{"cc",                   builtin::life::builtin_cc},
{"add_cc",               builtin::life::builtin_add_cc},
{"remove_cc",            builtin::life::builtin_remove_cc},
{"attack",               builtin::life::builtin_attack},
{"damage_rate",          builtin::life::builtin_damage_rate},
{"damage_derate",        builtin::life::builtin_damage_derate},
{"skill_damage_rate",    builtin::life::builtin_skill_damage_rate},
{"paralysis",            builtin::life::builtin_paralysis},
{"invincible",           builtin::life::builtin_invincible},
{"cover",                builtin::life::builtin_cover},
{"base_hp",              builtin::life::builtin_base_hp},
{"buff_hp",              builtin::life::builtin_buff_hp},
{"maxhp",                builtin::life::builtin_maxhp},
{"base_mp",              builtin::life::builtin_base_mp},
{"buff_mp",              builtin::life::builtin_buff_mp},
{"maxmp",                builtin::life::builtin_maxmp},
{"base_str",             builtin::life::builtin_base_str},
{"buff_str",             builtin::life::builtin_buff_str},
{"str",                  builtin::life::builtin_str},
{"base_dex",             builtin::life::builtin_base_dex},
{"buff_dex",             builtin::life::builtin_buff_dex},
{"dex",                  builtin::life::builtin_dex},
{"base_int",             builtin::life::builtin_base_int},
{"buff_int",             builtin::life::builtin_buff_int},
{"int",                  builtin::life::builtin_intelligence},
{"base_phydef",          builtin::life::builtin_base_phydef},
{"buff_phydef",          builtin::life::builtin_buff_phydef},
{"phydef",               builtin::life::builtin_phydef},
{"base_magdef",          builtin::life::builtin_base_magdef},
{"buff_magdef",          builtin::life::builtin_buff_magdef},
{"magdef",               builtin::life::builtin_magdef},
{"base_resist",          builtin::life::builtin_base_resist},
{"buff_resist",          builtin::life::builtin_buff_resist},
{"resist",               builtin::life::builtin_resist},
{"base_dam",             builtin::life::builtin_base_dam},
{"buff_dam",             builtin::life::builtin_buff_dam},
{"dam",                  builtin::life::builtin_dam},
{"base_hit",             builtin::life::builtin_base_hit},
{"buff_hit",             builtin::life::builtin_buff_hit},
{"hit",                  builtin::life::builtin_hit},
{"base_speed",           builtin::life::builtin_base_speed},
{"buff_speed",           builtin::life::builtin_buff_speed},
{"speed",                builtin::life::builtin_speed},
{"normal_attack_damage", builtin::life::builtin_normal_attack_damage},
{"update",               builtin::life::builtin_update},
END_LUA_EXTENSION; // clang-format on

int builtin::life::builtin_message(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;
    auto argc = lua->argc();
    auto obj  = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    if (obj->is(OBJECT_TYPE::CHARACTER) == false)
        return 0;

    auto ch      = std::static_pointer_cast<fb::game::character>(obj);
    auto message = lua->tostring(2);
    auto type    = lua->toenum(3, MESSAGE_TYPE::STATE);

    auto weak     = ch->weak_from_this();
    auto builder  = lua->new_co_builder();
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        ch->message(message, type);
        co_return;
    };
    builder.resume = []() -> async::task<int> {
        co_return 0;
    };
    return builder.run();
}

int builtin::life::builtin_hp(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;
    auto argc = lua->argc();
    auto obj  = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    if (argc == 1)
    {
        auto hp_value = std::make_shared<uint64_t>();
        auto weak     = obj->weak_from_this();
        auto builder  = lua->new_co_builder();
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            *hp_value = obj->stat.hp();
            co_return;
        };
        builder.resume = [=]() -> async::task<int> {
            lua->pushinteger(*hp_value);
            co_return 1;
        };
        return builder.run();
    }
    else
    {
        auto value    = lua->touint64(2);
        auto notify   = argc < 3 || lua->toboolean(3);
        auto weak     = obj->weak_from_this();
        auto builder  = lua->new_co_builder();
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            obj->stat.hp(value, notify);
            co_return;
        };
        builder.resume = []() -> async::task<int> {
            co_return 0;
        };
        return builder.run();
    }
}

int builtin::life::builtin_mp(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;
    auto argc = lua->argc();
    auto obj  = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    if (argc == 1)
    {
        auto mp_value = std::make_shared<uint64_t>();
        auto weak     = obj->weak_from_this();
        auto builder  = lua->new_co_builder();
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            *mp_value = obj->stat.mp();
            co_return;
        };
        builder.resume = [=]() -> async::task<int> {
            lua->pushinteger(*mp_value);
            co_return 1;
        };
        return builder.run();
    }
    else
    {
        auto value    = lua->touint64(2);
        auto notify   = argc < 3 || lua->toboolean(3);
        auto weak     = obj->weak_from_this();
        auto builder  = lua->new_co_builder();
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            obj->stat.mp(value, notify);
            co_return;
        };
        builder.resume = []() -> async::task<int> {
            co_return 0;
        };
        return builder.run();
    }
}

int builtin::life::builtin_heal(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;
    auto argc = lua->argc();
    auto obj  = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    auto value    = lua->touint64(2);
    auto notify   = argc < 3 || lua->toboolean(3);
    auto weak     = obj->weak_from_this();
    auto builder  = lua->new_co_builder();
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        std::ignore = obj->stat.heal(value, nullptr, notify);
        co_return;
    };
    builder.resume = []() -> async::task<int> {
        co_return 0;
    };
    return builder.run();
}

int builtin::life::builtin_damage_to(lua_State* L)
{
    // me:damage_to(you, damage[, opts])
    // me:damage_to({{you, damage}, ...}[, opts])
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto argc = lua->argc();
    auto me   = lua->touserdata<fb::game::life>(1);
    if (me == nullptr)
        return 0;

    auto opts     = fb::game::life::damage_opts{};
    auto targets  = std::make_shared<fb::game::life::damage_list>();
    auto opts_idx = 0;

    if (argc >= 3 && lua->is_userdata<fb::game::life>(2))
    {
        auto you = lua->touserdata<fb::game::life>(2);
        if (you == nullptr)
            return 0;
        auto value = lua->touint64(3);
        targets->emplace_back(you, value);
        opts_idx = 4;
    }
    else if (argc >= 2 && lua_istable(L, 2))
    {
        auto len = static_cast<int>(lua_rawlen(L, 2));
        for (auto i = 1; i <= len; i++)
        {
            lua_rawgeti(L, 2, i);
            if (!lua_istable(L, -1))
            {
                lua_pop(L, 1);
                continue;
            }

            lua_rawgeti(L, -1, 1);
            auto you = lua->touserdata<fb::game::life>(-1);
            lua_pop(L, 1);

            lua_rawgeti(L, -1, 2);
            auto value = static_cast<uint64_t>(std::max<lua_Integer>(0, lua_tointeger(L, -1)));
            lua_pop(L, 1);

            lua_pop(L, 1);
            if (you == nullptr)
                continue;

            targets->emplace_back(you, value);
        }
        opts_idx = 3;
    }
    else
    {
        return 0;
    }

    if (opts_idx > 0 && opts_idx <= argc && lua_istable(L, opts_idx))
    {
        lua_pushstring(L, "critical");
        lua_rawget(L, opts_idx);
        if (!lua_isnil(L, -1))
            opts.critical = lua_toboolean(L, -1) != 0;
        lua_pop(L, 1);

        lua_pushstring(L, "rate");
        lua_rawget(L, opts_idx);
        if (!lua_isnil(L, -1))
            opts.rate = static_cast<float>(lua_tonumber(L, -1));
        lua_pop(L, 1);

        lua_pushstring(L, "physical");
        lua_rawget(L, opts_idx);
        if (!lua_isnil(L, -1))
            opts.physical = lua_toboolean(L, -1) != 0;
        lua_pop(L, 1);

        lua_pushstring(L, "fixed");
        lua_rawget(L, opts_idx);
        if (!lua_isnil(L, -1))
            opts.fixed = lua_toboolean(L, -1) != 0;
        lua_pop(L, 1);

        lua_pushstring(L, "notify");
        lua_rawget(L, opts_idx);
        if (!lua_isnil(L, -1))
            opts.notify = lua_toboolean(L, -1) != 0;
        lua_pop(L, 1);
    }

    if (targets->empty())
        return 0;

    auto weak     = me->weak_from_this();
    auto builder  = lua->new_co_builder();
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        auto attacker = std::static_pointer_cast<fb::game::life>(weak.lock());
        if (attacker == nullptr)
            co_return;
        co_await attacker->damage_to(*targets, opts);
    };
    builder.resume = []() -> async::task<int> {
        co_return 0;
    };
    return builder.run();
}

int builtin::life::builtin_mp_up(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;
    auto argc = lua->argc();
    auto obj  = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    auto value    = lua->touint64(2);
    auto notify   = argc < 3 || lua->toboolean(3);
    auto weak     = obj->weak_from_this();
    auto builder  = lua->new_co_builder();
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        std::ignore = obj->stat.mp_up(value, nullptr, notify);
        co_return;
    };
    builder.resume = []() -> async::task<int> {
        co_return 0;
    };
    return builder.run();
}

int builtin::life::builtin_mp_down(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;
    auto argc = lua->argc();
    auto obj  = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    auto value    = lua->touint64(2);
    auto notify   = argc < 3 || lua->toboolean(3);
    auto weak     = obj->weak_from_this();
    auto builder  = lua->new_co_builder();
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        std::ignore = obj->stat.mp_down(value, nullptr, notify);
        co_return;
    };
    builder.resume = []() -> async::task<int> {
        co_return 0;
    };
    return builder.run();
}

int builtin::life::builtin_action(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;
    auto argc = lua->argc();
    auto obj  = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    auto action   = lua->tointeger(2);
    auto duration = lua->tointeger(3, static_cast<int>(DURATION::SPELL));
    auto sound    = (uint8_t)lua->tointeger(4, (uint8_t)0x00);
    auto weak     = obj->weak_from_this();
    auto builder  = lua->new_co_builder();
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        obj->action(ACTION(action), DURATION(duration), sound);
        co_return;
    };
    builder.resume = []() -> async::task<int> {
        co_return 0;
    };
    return builder.run();
}

int builtin::life::builtin_spell(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;
    auto argc = lua->argc();
    auto obj  = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    if (lua->is_number(2))
    {
        auto index    = (int)lua->tointeger(2);
        auto spell    = std::make_shared<std::shared_ptr<fb::game::spell>>();
        auto weak     = obj->weak_from_this();
        auto builder  = lua->new_co_builder();
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            *spell = obj->spells[index];
            co_return;
        };
        builder.resume = [=]() -> async::task<int> {
            if (*spell == nullptr)
                lua->pushnil();
            else
                lua->pushobject(*spell);

            co_return 1;
        };
        return builder.run();
    }
    else if (lua->is_string(2))
    {
        auto name     = lua->tostring(2);
        auto spell    = std::make_shared<std::shared_ptr<fb::game::spell>>();
        auto weak     = obj->weak_from_this();
        auto builder  = lua->new_co_builder();
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            *spell = obj->spells.find(name);
            co_return;
        };
        builder.resume = [=]() -> async::task<int> {
            if (*spell == nullptr)
                lua->pushnil();
            else
                lua->pushobject(*spell);

            co_return 1;
        };
        return builder.run();
    }
    else if (lua->is_userdata<fb::model::spell>(2))
    {
        auto model    = lua->touserdata<fb::model::spell>(2);
        auto spell    = std::make_shared<std::shared_ptr<fb::game::spell>>();
        auto weak     = obj->weak_from_this();
        auto builder  = lua->new_co_builder();
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            *spell = obj->spells.find(*model);
            co_return;
        };
        builder.resume = [=]() -> async::task<int> {
            if (*spell == nullptr)
                lua->pushnil();
            else
                lua->pushobject(*spell);

            co_return 1;
        };
        return builder.run();
    }
    else
    {
        return 0;
    }
}

int builtin::life::builtin_spells(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;
    auto argc = lua->argc();
    auto obj  = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    auto buffer   = std::make_shared<std::vector<std::shared_ptr<fb::game::spell>>>();
    auto weak     = obj->weak_from_this();
    auto builder  = lua->new_co_builder();
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        for (int i = 0; i < CONTAINER_CAPACITY; i++)
        {
            auto spell = obj->spells[i];
            if (spell != nullptr)
                buffer->push_back(spell);
        }
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        lua->new_table();
        for (int i = 0; i < buffer->size(); i++)
        {
            lua->pushobject((*buffer)[i]);
            lua_rawseti(L, -2, i + 1);
        }

        co_return 1;
    };
    return builder.run();
}

int builtin::life::builtin_cast(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;
    auto argc   = lua->argc();
    auto offset = 1;
    auto obj    = lua->touserdata<fb::game::life>(offset++);
    if (obj == nullptr)
        return 0;

    std::shared_ptr<fb::game::object> you;
    if (argc == 2)
        you = std::static_pointer_cast<fb::game::object>(obj);
    else if (lua->is_userdata<fb::game::object>(offset))
        you = std::static_pointer_cast<fb::game::object>(lua->touserdata<fb::game::object>(offset++));
    else
        you = nullptr;

    if (you == nullptr)
        return 0;

    auto name  = lua->tostring(offset++);
    auto spell = table::spell.name2spell(name);
    if (spell == nullptr)
        return 0;

    auto count = 2;
    if (spell->type == SPELL_TYPE::TARGET)
        count++;

    auto weak     = obj->weak_from_this();
    auto builder  = lua->new_co_builder();
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        auto map = obj->map();
        if (map == nullptr || map->objects.contains(you) == false)
            co_return;

        auto path = std::format("scripts/spell/{}.lua", spell->id);
        auto func = "on_cast";

        auto x = static_cast<fb::game::server&>(lua->executor).lua.open(path, func);
        if (!x)
            co_return;

        x->pushobject(obj);

        if (spell->type == SPELL_TYPE::TARGET)
        {
            if (you != nullptr)
                x->pushobject(you);
            else
                x->pushnil();
        }

        x->pushobject(spell);
        std::ignore = co_await x->call(count);
    };
    builder.resume = []() -> async::task<int> {
        co_return 0;
    };
    return builder.run();
}

int builtin::life::builtin_cc(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;
    auto argc = lua->argc();
    auto obj  = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    if (argc == 1)
    {
        auto cc_value = std::make_shared<uint32_t>();
        auto weak     = obj->weak_from_this();
        auto builder  = lua->new_co_builder();
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            *cc_value = static_cast<uint32_t>(obj->cc);
            co_return;
        };
        builder.resume = [=]() -> async::task<int> {
            lua->pushinteger(*cc_value);
            co_return 1;
        };
        return builder.run();
    }
    else
    {
        auto cc       = static_cast<CROWD_CONTROL>(lua->tointeger(2));
        auto weak     = obj->weak_from_this();
        auto builder  = lua->new_co_builder();
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            obj->cc.set(cc);
            co_return;
        };
        builder.resume = []() -> async::task<int> {
            co_return 0;
        };
        return builder.run();
    }
}

int builtin::life::builtin_add_cc(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;
    auto argc = lua->argc();
    auto obj  = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    if (argc == 1)
    {
        auto cc_value = std::make_shared<uint32_t>();
        auto weak     = obj->weak_from_this();
        auto builder  = lua->new_co_builder();
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            *cc_value = static_cast<uint32_t>(obj->cc);
            co_return;
        };
        builder.resume = [=]() -> async::task<int> {
            lua->pushinteger(*cc_value);
            co_return 1;
        };
        return builder.run();
    }
    else
    {
        auto cc       = static_cast<CROWD_CONTROL>(lua->tointeger(2));
        auto weak     = obj->weak_from_this();
        auto builder  = lua->new_co_builder();
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            std::ignore = obj->cc.add(cc);
            co_return;
        };
        builder.resume = []() -> async::task<int> {
            co_return 0;
        };
        return builder.run();
    }
}

int builtin::life::builtin_remove_cc(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;
    auto argc = lua->argc();
    auto obj  = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    auto cc       = static_cast<CROWD_CONTROL>(lua->tointeger(2));
    auto weak     = obj->weak_from_this();
    auto builder  = lua->new_co_builder();
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        std::ignore = obj->cc.remove(cc);
        co_return;
    };
    builder.resume = []() -> async::task<int> {
        co_return 0;
    };
    return builder.run();
}

int builtin::life::builtin_attack(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;
    auto argc = lua->argc();
    auto obj  = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    auto duration = lua->toenum(2, DURATION::ATTACK);
    auto weak     = obj->weak_from_this();
    auto builder  = lua->new_co_builder();
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        co_await obj->attack(duration);
        co_return;
    };
    builder.resume = []() -> async::task<int> {
        co_return 0;
    };
    return builder.run();
}

int builtin::life::builtin_damage_rate(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;
    auto argc = lua->argc();
    auto obj  = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    if (argc == 1)
    {
        auto damage_rate_value = std::make_shared<uint32_t>();
        auto weak              = obj->weak_from_this();
        auto builder           = lua->new_co_builder();
        builder.weak           = weak;
        builder.yield          = [=]() -> async::task<void> {
            *damage_rate_value = obj->damage_rate();
            co_return;
        };
        builder.resume = [=]() -> async::task<int> {
            lua->pushinteger(*damage_rate_value);
            co_return 1;
        };
        return builder.run();
    }
    else
    {
        auto value    = lua->tointeger(2);
        auto weak     = obj->weak_from_this();
        auto builder  = lua->new_co_builder();
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            obj->damage_rate(value);
            co_return;
        };
        builder.resume = []() -> async::task<int> {
            co_return 0;
        };
        return builder.run();
    }
}

int builtin::life::builtin_damage_derate(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;
    auto argc = lua->argc();
    auto obj  = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    if (argc == 1)
    {
        auto damage_derate_value = std::make_shared<uint32_t>();
        auto weak                = obj->weak_from_this();
        auto builder             = lua->new_co_builder();
        builder.weak             = weak;
        builder.yield            = [=]() -> async::task<void> {
            *damage_derate_value = obj->damage_derate();
            co_return;
        };
        builder.resume = [=]() -> async::task<int> {
            lua->pushinteger(*damage_derate_value);
            co_return 1;
        };
        return builder.run();
    }
    else
    {
        auto value    = lua->tointeger(2);
        auto weak     = obj->weak_from_this();
        auto builder  = lua->new_co_builder();
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            obj->damage_derate(value);
            co_return;
        };
        builder.resume = []() -> async::task<int> {
            co_return 0;
        };
        return builder.run();
    }
}

int builtin::life::builtin_skill_damage_rate(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;
    auto argc = lua->argc();
    auto obj  = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    if (argc == 1)
    {
        auto skill_damage_rate_value = std::make_shared<uint32_t>();
        auto weak                    = obj->weak_from_this();
        auto builder                 = lua->new_co_builder();
        builder.weak                 = weak;
        builder.yield                = [=]() -> async::task<void> {
            *skill_damage_rate_value = obj->skill_damage_rate();
            co_return;
        };
        builder.resume = [=]() -> async::task<int> {
            lua->pushinteger(*skill_damage_rate_value);
            co_return 1;
        };
        return builder.run();
    }
    else
    {
        auto value    = lua->tointeger(2);
        auto weak     = obj->weak_from_this();
        auto builder  = lua->new_co_builder();
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            obj->skill_damage_rate(value);
            co_return;
        };
        builder.resume = []() -> async::task<int> {
            co_return 0;
        };
        return builder.run();
    }
}

int builtin::life::builtin_paralysis(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;
    auto argc = lua->argc();
    auto obj  = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    if (argc == 1)
    {
        auto paralysis_value = std::make_shared<bool>();
        auto weak            = obj->weak_from_this();
        auto builder         = lua->new_co_builder();
        builder.weak         = weak;
        builder.yield        = [=]() -> async::task<void> {
            *paralysis_value = obj->paralysis();
            co_return;
        };
        builder.resume = [=]() -> async::task<int> {
            lua->pushboolean(*paralysis_value);
            co_return 1;
        };
        return builder.run();
    }
    else
    {
        auto value    = lua->toboolean(2);
        auto weak     = obj->weak_from_this();
        auto builder  = lua->new_co_builder();
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            obj->paralysis(value);
            co_return;
        };
        builder.resume = []() -> async::task<int> {
            co_return 0;
        };
        return builder.run();
    }
}

int builtin::life::builtin_invincible(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;
    auto argc = lua->argc();
    auto obj  = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    if (argc == 1)
    {
        auto invincible_value = std::make_shared<bool>();
        auto weak             = obj->weak_from_this();
        auto builder          = lua->new_co_builder();
        builder.weak          = weak;
        builder.yield         = [=]() -> async::task<void> {
            *invincible_value = obj->invincible();
            co_return;
        };
        builder.resume = [=]() -> async::task<int> {
            lua->pushboolean(*invincible_value);
            co_return 1;
        };
        return builder.run();
    }
    else
    {
        auto value    = lua->toboolean(2);
        auto weak     = obj->weak_from_this();
        auto builder  = lua->new_co_builder();
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            obj->invincible(value);
            co_return;
        };
        builder.resume = []() -> async::task<int> {
            co_return 0;
        };
        return builder.run();
    }
}

int builtin::life::builtin_cover(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;
    auto argc = lua->argc();
    auto obj  = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    if (argc == 1)
    {
        auto cover_value = std::make_shared<bool>();
        auto weak        = obj->weak_from_this();
        auto builder     = lua->new_co_builder();
        builder.weak     = weak;
        builder.yield    = [=]() -> async::task<void> {
            *cover_value = obj->cover();
            co_return;
        };
        builder.resume = [=]() -> async::task<int> {
            lua->pushboolean(*cover_value);
            co_return 1;
        };
        return builder.run();
    }
    else
    {
        auto value    = lua->toboolean(2);
        auto weak     = obj->weak_from_this();
        auto builder  = lua->new_co_builder();
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            obj->cover(value);
            co_return;
        };
        builder.resume = []() -> async::task<int> {
            co_return 0;
        };
        return builder.run();
    }
}

int builtin::life::builtin_base_hp(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto argc = lua->argc();
    auto obj  = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    auto value    = std::make_shared<uint64_t>();
    auto weak     = obj->weak_from_this();
    auto builder  = lua->new_co_builder();
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        *value = obj->stat.base_hp();
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        lua->pushinteger(*value);
        co_return 1;
    };
    return builder.run();
}

int builtin::life::builtin_buff_hp(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;
    auto argc = lua->argc();
    auto obj  = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    if (argc == 1)
    {
        auto buff_hp_value = std::make_shared<int64_t>();
        auto weak          = obj->weak_from_this();
        auto builder       = lua->new_co_builder();
        builder.weak       = weak;
        builder.yield      = [=]() -> async::task<void> {
            *buff_hp_value = obj->stat.buff_hp();
            co_return;
        };
        builder.resume = [=]() -> async::task<int> {
            lua->pushinteger(*buff_hp_value);
            co_return 1;
        };
        return builder.run();
    }
    else
    {
        auto value    = lua->toint64(2);
        auto weak     = obj->weak_from_this();
        auto builder  = lua->new_co_builder();
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            obj->stat.buff_hp(value);
            co_return;
        };
        builder.resume = []() -> async::task<int> {
            co_return 0;
        };
        return builder.run();
    }
}

int builtin::life::builtin_maxhp(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;
    auto obj = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    auto maxhp_value = std::make_shared<uint64_t>();
    auto weak        = obj->weak_from_this();
    auto builder     = lua->new_co_builder();
    builder.weak     = weak;
    builder.yield    = [=]() -> async::task<void> {
        *maxhp_value = obj->stat.maxhp();
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        lua->pushinteger(*maxhp_value);
        co_return 1;
    };
    return builder.run();
}

int builtin::life::builtin_base_mp(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;
    auto obj = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    auto base_mp_value = std::make_shared<uint64_t>();
    auto weak          = obj->weak_from_this();
    auto builder       = lua->new_co_builder();
    builder.weak       = weak;
    builder.yield      = [=]() -> async::task<void> {
        *base_mp_value = obj->stat.base_mp();
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        lua->pushinteger(*base_mp_value);
        co_return 1;
    };
    return builder.run();
}

int builtin::life::builtin_buff_mp(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;
    auto argc = lua->argc();
    auto obj  = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    if (argc == 1)
    {
        auto buff_mp_value = std::make_shared<int64_t>();
        auto weak          = obj->weak_from_this();
        auto builder       = lua->new_co_builder();
        builder.weak       = weak;
        builder.yield      = [=]() -> async::task<void> {
            *buff_mp_value = obj->stat.buff_mp();
            co_return;
        };
        builder.resume = [=]() -> async::task<int> {
            lua->pushinteger(*buff_mp_value);
            co_return 1;
        };
        return builder.run();
    }
    else
    {
        auto value    = lua->toint64(2);
        auto weak     = obj->weak_from_this();
        auto builder  = lua->new_co_builder();
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            obj->stat.buff_mp(value);
            co_return;
        };
        builder.resume = []() -> async::task<int> {
            co_return 0;
        };
        return builder.run();
    }
}

int builtin::life::builtin_maxmp(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;
    auto obj = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    auto maxmp_value = std::make_shared<uint64_t>();
    auto weak        = obj->weak_from_this();
    auto builder     = lua->new_co_builder();
    builder.weak     = weak;
    builder.yield    = [=]() -> async::task<void> {
        *maxmp_value = obj->stat.maxmp();
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        lua->pushinteger(*maxmp_value);
        co_return 1;
    };
    return builder.run();
}

int builtin::life::builtin_base_str(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;
    auto obj = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    auto base_str_value = std::make_shared<uint8_t>();
    auto weak           = obj->weak_from_this();
    auto builder        = lua->new_co_builder();
    builder.weak        = weak;
    builder.yield       = [=]() -> async::task<void> {
        *base_str_value = obj->stat.base_str();
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        lua->pushinteger(*base_str_value);
        co_return 1;
    };
    return builder.run();
}

int builtin::life::builtin_buff_str(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;
    auto argc = lua->argc();
    auto obj  = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    if (argc == 1)
    {
        auto buff_str_value = std::make_shared<uint8_t>();
        auto weak           = obj->weak_from_this();
        auto builder        = lua->new_co_builder();
        builder.weak        = weak;
        builder.yield       = [=]() -> async::task<void> {
            *buff_str_value = obj->stat.buff_str();
            co_return;
        };
        builder.resume = [=]() -> async::task<int> {
            lua->pushinteger(*buff_str_value);
            co_return 1;
        };
        return builder.run();
    }
    else
    {
        auto value    = lua->tointeger(2);
        auto weak     = obj->weak_from_this();
        auto builder  = lua->new_co_builder();
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            obj->stat.buff_str(value);
            co_return;
        };
        builder.resume = []() -> async::task<int> {
            co_return 0;
        };
        return builder.run();
    }
}

int builtin::life::builtin_str(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;
    auto obj = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    auto str_value = std::make_shared<uint8_t>();
    auto weak      = obj->weak_from_this();
    auto builder   = lua->new_co_builder();
    builder.weak   = weak;
    builder.yield  = [=]() -> async::task<void> {
        *str_value = obj->stat.str();
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        lua->pushinteger(*str_value);
        co_return 1;
    };
    return builder.run();
}

int builtin::life::builtin_base_dex(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;
    auto obj = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    auto base_dex_value = std::make_shared<uint8_t>();
    auto weak           = obj->weak_from_this();
    auto builder        = lua->new_co_builder();
    builder.weak        = weak;
    builder.yield       = [=]() -> async::task<void> {
        *base_dex_value = obj->stat.base_dex();
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        lua->pushinteger(*base_dex_value);
        co_return 1;
    };
    return builder.run();
}

int builtin::life::builtin_buff_dex(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;
    auto argc = lua->argc();
    auto obj  = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    if (argc == 1)
    {
        auto buff_dex_value = std::make_shared<uint8_t>();
        auto weak           = obj->weak_from_this();
        auto builder        = lua->new_co_builder();
        builder.weak        = weak;
        builder.yield       = [=]() -> async::task<void> {
            *buff_dex_value = obj->stat.buff_dex();
            co_return;
        };
        builder.resume = [=]() -> async::task<int> {
            lua->pushinteger(*buff_dex_value);
            co_return 1;
        };
        return builder.run();
    }
    else
    {
        auto value    = lua->tointeger(2);
        auto weak     = obj->weak_from_this();
        auto builder  = lua->new_co_builder();
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            obj->stat.buff_dex(value);
            co_return;
        };
        builder.resume = []() -> async::task<int> {
            co_return 0;
        };
        return builder.run();
    }
}

int builtin::life::builtin_dex(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;
    auto obj = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    auto dex_value = std::make_shared<uint8_t>();
    auto weak      = obj->weak_from_this();
    auto builder   = lua->new_co_builder();
    builder.weak   = weak;
    builder.yield  = [=]() -> async::task<void> {
        *dex_value = obj->stat.dex();
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        lua->pushinteger(*dex_value);
        co_return 1;
    };
    return builder.run();
}

int builtin::life::builtin_base_int(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;
    auto obj = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    auto base_int_value = std::make_shared<uint8_t>();
    auto weak           = obj->weak_from_this();
    auto builder        = lua->new_co_builder();
    builder.weak        = weak;
    builder.yield       = [=]() -> async::task<void> {
        *base_int_value = obj->stat.base_int();
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        lua->pushinteger(*base_int_value);
        co_return 1;
    };
    return builder.run();
}

int builtin::life::builtin_buff_int(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;
    auto argc = lua->argc();
    auto obj  = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    if (argc == 1)
    {
        auto buff_int_value = std::make_shared<uint8_t>();
        auto weak           = obj->weak_from_this();
        auto builder        = lua->new_co_builder();
        builder.weak        = weak;
        builder.yield       = [=]() -> async::task<void> {
            *buff_int_value = obj->stat.buff_int();
            co_return;
        };
        builder.resume = [=]() -> async::task<int> {
            lua->pushinteger(*buff_int_value);
            co_return 1;
        };
        return builder.run();
    }
    else
    {
        auto value    = lua->tointeger(2);
        auto weak     = obj->weak_from_this();
        auto builder  = lua->new_co_builder();
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            obj->stat.buff_int(value);
            co_return;
        };
        builder.resume = []() -> async::task<int> {
            co_return 0;
        };
        return builder.run();
    }
}

int builtin::life::builtin_intelligence(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;
    auto obj = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    auto intelligence_value = std::make_shared<uint8_t>();
    auto weak               = obj->weak_from_this();
    auto builder            = lua->new_co_builder();
    builder.weak            = weak;
    builder.yield           = [=]() -> async::task<void> {
        *intelligence_value = obj->stat.intelligence();
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        lua->pushinteger(*intelligence_value);
        co_return 1;
    };
    return builder.run();
}

int builtin::life::builtin_base_phydef(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;
    auto obj = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    auto base_phydef_value = std::make_shared<int8_t>();
    auto weak              = obj->weak_from_this();
    auto builder           = lua->new_co_builder();
    builder.weak           = weak;
    builder.yield          = [=]() -> async::task<void> {
        *base_phydef_value = obj->stat.base_phydef();
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        lua->pushinteger(*base_phydef_value);
        co_return 1;
    };
    return builder.run();
}

int builtin::life::builtin_buff_phydef(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;
    auto argc = lua->argc();
    auto obj  = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    if (argc == 1)
    {
        auto buff_phydef_value = std::make_shared<int8_t>();
        auto weak              = obj->weak_from_this();
        auto builder           = lua->new_co_builder();
        builder.weak           = weak;
        builder.yield          = [=]() -> async::task<void> {
            *buff_phydef_value = obj->stat.buff_phydef();
            co_return;
        };
        builder.resume = [=]() -> async::task<int> {
            lua->pushinteger(*buff_phydef_value);
            co_return 1;
        };
        return builder.run();
    }
    else
    {
        auto value    = lua->tointeger(2);
        auto weak     = obj->weak_from_this();
        auto builder  = lua->new_co_builder();
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            obj->stat.buff_phydef(value);
            co_return;
        };
        builder.resume = []() -> async::task<int> {
            co_return 0;
        };
        return builder.run();
    }
}

int builtin::life::builtin_phydef(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;
    auto obj = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    auto phydef_value = std::make_shared<int8_t>();
    auto weak         = obj->weak_from_this();
    auto builder      = lua->new_co_builder();
    builder.weak      = weak;
    builder.yield     = [=]() -> async::task<void> {
        *phydef_value = obj->stat.phydef();
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        lua->pushinteger(*phydef_value);
        co_return 1;
    };
    return builder.run();
}

int builtin::life::builtin_base_magdef(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;
    auto obj = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    auto base_magdef_value = std::make_shared<int8_t>();
    auto weak              = obj->weak_from_this();
    auto builder           = lua->new_co_builder();
    builder.weak           = weak;
    builder.yield          = [=]() -> async::task<void> {
        *base_magdef_value = obj->stat.base_magdef();
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        lua->pushinteger(*base_magdef_value);
        co_return 1;
    };
    return builder.run();
}

int builtin::life::builtin_buff_magdef(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;
    auto argc = lua->argc();
    auto obj  = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    if (argc == 1)
    {
        auto buff_magdef_value = std::make_shared<int8_t>();
        auto weak              = obj->weak_from_this();
        auto builder           = lua->new_co_builder();
        builder.weak           = weak;
        builder.yield          = [=]() -> async::task<void> {
            *buff_magdef_value = obj->stat.buff_magdef();
            co_return;
        };
        builder.resume = [=]() -> async::task<int> {
            lua->pushinteger(*buff_magdef_value);
            co_return 1;
        };
        return builder.run();
    }
    else
    {
        auto value    = lua->tointeger(2);
        auto weak     = obj->weak_from_this();
        auto builder  = lua->new_co_builder();
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            obj->stat.buff_magdef(value);
            co_return;
        };
        builder.resume = []() -> async::task<int> {
            co_return 0;
        };
        return builder.run();
    }
}

int builtin::life::builtin_magdef(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;
    auto obj = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    auto magdef_value = std::make_shared<int8_t>();
    auto weak         = obj->weak_from_this();
    auto builder      = lua->new_co_builder();
    builder.weak      = weak;
    builder.yield     = [=]() -> async::task<void> {
        *magdef_value = obj->stat.magdef();
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        lua->pushinteger(*magdef_value);
        co_return 1;
    };
    return builder.run();
}

int builtin::life::builtin_base_dam(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;
    auto obj = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    auto base_dam_value = std::make_shared<uint8_t>();
    auto weak           = obj->weak_from_this();
    auto builder        = lua->new_co_builder();
    builder.weak        = weak;
    builder.yield       = [=]() -> async::task<void> {
        *base_dam_value = obj->stat.base_dam();
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        lua->pushinteger(*base_dam_value);
        co_return 1;
    };
    return builder.run();
}

int builtin::life::builtin_buff_dam(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;
    auto argc = lua->argc();
    auto obj  = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    if (argc == 1)
    {
        auto buff_dam_value = std::make_shared<int8_t>();
        auto weak           = obj->weak_from_this();
        auto builder        = lua->new_co_builder();
        builder.weak        = weak;
        builder.yield       = [=]() -> async::task<void> {
            *buff_dam_value = obj->stat.buff_dam();
            co_return;
        };
        builder.resume = [=]() -> async::task<int> {
            lua->pushinteger(*buff_dam_value);
            co_return 1;
        };
        return builder.run();
    }
    else
    {
        auto value    = lua->tointeger(2);
        auto weak     = obj->weak_from_this();
        auto builder  = lua->new_co_builder();
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            obj->stat.buff_dam(static_cast<int8_t>(value));
            co_return;
        };
        builder.resume = []() -> async::task<int> {
            co_return 0;
        };
        return builder.run();
    }
}

int builtin::life::builtin_dam(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;
    auto obj = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    auto dam_value = std::make_shared<int8_t>();
    auto weak      = obj->weak_from_this();
    auto builder   = lua->new_co_builder();
    builder.weak   = weak;
    builder.yield  = [=]() -> async::task<void> {
        *dam_value = obj->stat.dam();
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        lua->pushinteger(*dam_value);
        co_return 1;
    };
    return builder.run();
}

int builtin::life::builtin_base_hit(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;
    auto obj = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    auto base_hit_value = std::make_shared<uint8_t>();
    auto weak           = obj->weak_from_this();
    auto builder        = lua->new_co_builder();
    builder.weak        = weak;
    builder.yield       = [=]() -> async::task<void> {
        *base_hit_value = obj->stat.base_hit();
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        lua->pushinteger(*base_hit_value);
        co_return 1;
    };
    return builder.run();
}

int builtin::life::builtin_buff_hit(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;
    auto argc = lua->argc();
    auto obj  = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    if (argc == 1)
    {
        auto buff_hit_value = std::make_shared<int8_t>();
        auto weak           = obj->weak_from_this();
        auto builder        = lua->new_co_builder();
        builder.weak        = weak;
        builder.yield       = [=]() -> async::task<void> {
            *buff_hit_value = obj->stat.buff_hit();
            co_return;
        };
        builder.resume = [=]() -> async::task<int> {
            lua->pushinteger(*buff_hit_value);
            co_return 1;
        };
        return builder.run();
    }
    else
    {
        auto value    = lua->tointeger(2);
        auto weak     = obj->weak_from_this();
        auto builder  = lua->new_co_builder();
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            obj->stat.buff_hit(static_cast<int8_t>(value));
            co_return;
        };
        builder.resume = []() -> async::task<int> {
            co_return 0;
        };
        return builder.run();
    }
}

int builtin::life::builtin_hit(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;
    auto obj = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    auto hit_value = std::make_shared<int8_t>();
    auto weak      = obj->weak_from_this();
    auto builder   = lua->new_co_builder();
    builder.weak   = weak;
    builder.yield  = [=]() -> async::task<void> {
        *hit_value = obj->stat.hit();
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        lua->pushinteger(*hit_value);
        co_return 1;
    };
    return builder.run();
}

int builtin::life::builtin_base_speed(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;
    auto obj = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    auto base_speed_value = std::make_shared<uint8_t>();
    auto weak             = obj->weak_from_this();
    auto builder          = lua->new_co_builder();
    builder.weak          = weak;
    builder.yield         = [=]() -> async::task<void> {
        *base_speed_value = obj->stat.base_speed();
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        lua->pushinteger(*base_speed_value);
        co_return 1;
    };
    return builder.run();
}

int builtin::life::builtin_buff_speed(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;
    auto argc = lua->argc();
    auto obj  = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    if (argc == 1)
    {
        auto buff_speed_value = std::make_shared<int8_t>();
        auto weak             = obj->weak_from_this();
        auto builder          = lua->new_co_builder();
        builder.weak          = weak;
        builder.yield         = [=]() -> async::task<void> {
            *buff_speed_value = obj->stat.buff_speed();
            co_return;
        };
        builder.resume = [=]() -> async::task<int> {
            lua->pushinteger(*buff_speed_value);
            co_return 1;
        };
        return builder.run();
    }
    else
    {
        auto value    = lua->tointeger(2);
        auto weak     = obj->weak_from_this();
        auto builder  = lua->new_co_builder();
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            obj->stat.buff_speed(static_cast<int8_t>(value));
            co_return;
        };
        builder.resume = []() -> async::task<int> {
            co_return 0;
        };
        return builder.run();
    }
}

int builtin::life::builtin_speed(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;
    auto obj = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    auto speed_value = std::make_shared<uint8_t>();
    auto weak        = obj->weak_from_this();
    auto builder     = lua->new_co_builder();
    builder.weak     = weak;
    builder.yield    = [=]() -> async::task<void> {
        *speed_value = obj->stat.speed();
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        lua->pushinteger(*speed_value);
        co_return 1;
    };
    return builder.run();
}

int builtin::life::builtin_normal_attack_damage(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;
    auto obj = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    obj->assert_thread();

    auto size     = lua->toenum(2, MOB_SIZE::SMALL);
    auto damage   = std::make_shared<uint64_t>();
    auto weak     = obj->weak_from_this();
    auto builder  = lua->new_co_builder();
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        *damage = obj->normal_attack_damage(size);
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        lua->pushinteger(*damage);
        co_return 1;
    };
    return builder.run();
}

int builtin::life::builtin_update(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;
    auto argc = lua->argc();
    auto obj  = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    auto level    = argc >= 2 ? lua->toenum(2, UPDATE_STATE_LEVEL::HP_MP | UPDATE_STATE_LEVEL::BASED)
                              : UPDATE_STATE_LEVEL::HP_MP | UPDATE_STATE_LEVEL::BASED;
    auto weak     = obj->weak_from_this();
    auto builder  = lua->new_co_builder();
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        obj->update(level);
        co_return;
    };
    builder.resume = []() -> async::task<int> {
        co_return 0;
    };
    return builder.run();
}

int builtin::life::builtin_base_resist(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;
    auto obj = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    auto type     = static_cast<RESIST>(lua->tointeger(2));
    auto value    = std::make_shared<float>();
    auto weak     = obj->weak_from_this();
    auto builder  = lua->new_co_builder();
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        *value = obj->stat.base_resist(type);
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        lua->pushnumber(*value);
        co_return 1;
    };
    return builder.run();
}

int builtin::life::builtin_buff_resist(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;
    auto argc = lua->argc();
    auto obj  = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    auto type = static_cast<RESIST>(lua->tointeger(2));
    if (argc == 2)
    {
        auto value    = std::make_shared<float>();
        auto weak     = obj->weak_from_this();
        auto builder  = lua->new_co_builder();
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            *value = obj->stat.buff_resist(type);
            co_return;
        };
        builder.resume = [=]() -> async::task<int> {
            lua->pushnumber(*value);
            co_return 1;
        };
        return builder.run();
    }
    else
    {
        auto value    = static_cast<float>(lua_tonumber(L, 3));
        auto weak     = obj->weak_from_this();
        auto builder  = lua->new_co_builder();
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            obj->stat.buff_resist(type, value);
            co_return;
        };
        builder.resume = []() -> async::task<int> {
            co_return 0;
        };
        return builder.run();
    }
}

int builtin::life::builtin_resist(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;
    auto obj = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    auto type     = static_cast<RESIST>(lua->tointeger(2));
    auto value    = std::make_shared<float>();
    auto weak     = obj->weak_from_this();
    auto builder  = lua->new_co_builder();
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        *value = obj->stat.resist(type);
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        lua->pushnumber(*value);
        co_return 1;
    };
    return builder.run();
}
