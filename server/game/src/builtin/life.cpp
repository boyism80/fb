#include <fb/game/server.h>
#include <fb/game/builtin/object.h>
#include <fb/game/builtin/life.h>
#include <fb/game/map.h>

using namespace fb::game;
using namespace fb::model;

// clang-format off
IMPLEMENT_LUA_EXTENSION(fb::game::life, "fb.game.life")
{"__eq",                 builtin::object::builtin_eq},
{"message",              builtin::life::builtin_message},
{"hp",                   builtin::life::builtin_hp},
{"mp",                   builtin::life::builtin_mp},
{"heal",                 builtin::life::builtin_heal},
{"damage",               builtin::life::builtin_damage},
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
{"base_dam",             builtin::life::builtin_base_dam},
{"buff_dam",             builtin::life::builtin_buff_dam},
{"dam",                  builtin::life::builtin_dam},
{"base_hit",             builtin::life::builtin_base_hit},
{"buff_hit",             builtin::life::builtin_buff_hit},
{"hit",                  builtin::life::builtin_hit},
{"normal_attack_damage", builtin::life::builtin_normal_attack_damage},
{"update",               builtin::life::builtin_update},
END_LUA_EXTENSION; // clang-format on

int builtin::life::builtin_message(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto obj    = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    if (obj->is(OBJECT_TYPE::CHARACTER) == false)
        return 0;

    auto ch      = std::static_pointer_cast<fb::game::character>(obj);
    auto message = lua->tostring(2);
    auto type    = lua->toenum(3, MESSAGE_TYPE::STATE);

    auto weak = ch->weak_from_this();
    return lua->ensure_yield(*server, weak, [=](auto is_yield) {
        ch->message(message, type);
        return lua->ensure_resume(*server, weak, [=]() {
            return 0;
        });
    });
}

int builtin::life::builtin_hp(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto obj    = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    if (argc == 1)
    {
        auto weak = obj->weak_from_this();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            auto hp_value = obj->stat.hp();

            return lua->ensure_resume(*server, weak, [=]() {
                lua->pushinteger(hp_value);
                return 1;
            });
        });
    }
    else
    {
        auto value  = (uint32_t)lua->tointeger(2);
        auto notify = argc < 3 || lua->toboolean(3);
        auto weak   = obj->weak_from_this();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            obj->stat.hp(value, notify);

            return lua->ensure_resume(*server, weak, [=]() {
                return 0;
            });
        });
    }
}

int builtin::life::builtin_mp(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto obj    = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    if (argc == 1)
    {
        auto weak = obj->weak_from_this();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            auto mp_value = obj->stat.mp();

            return lua->ensure_resume(*server, weak, [=]() {
                lua->pushinteger(mp_value);
                return 1;
            });
        });
    }
    else
    {
        auto value  = (uint32_t)lua->tointeger(2);
        auto notify = argc < 3 || lua->toboolean(3);
        auto weak   = obj->weak_from_this();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            obj->stat.mp(value, notify);

            return lua->ensure_resume(*server, weak, [=]() {
                return 0;
            });
        });
    }
}

int builtin::life::builtin_heal(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto obj    = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    auto value  = (uint32_t)lua->tointeger(2);
    auto notify = argc < 3 || lua->toboolean(3);
    auto weak   = obj->weak_from_this();
    return lua->ensure_yield(*server, weak, [=](auto is_yield) {
        obj->stat.heal(value, nullptr, notify);
        return lua->ensure_resume(*server, weak, [=]() {
            return 0;
        });
    });
}

int builtin::life::builtin_damage(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto obj    = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    auto  value    = (uint32_t)lua->tointeger(2);
    auto  from     = lua->touserdata<fb::game::life>(3);
    bool  critical = false;
    float rate     = 1.0f;
    bool  physical = true;
    bool  fixed    = false;

    auto notify = true;
    if (argc >= 4 && lua_istable(L, 4))
    {
        lua_pushstring(L, "critical");
        lua_rawget(L, 4);
        if (!lua_isnil(L, -1))
            critical = lua_toboolean(L, -1) != 0;
        lua_pop(L, 1);

        lua_pushstring(L, "rate");
        lua_rawget(L, 4);
        if (!lua_isnil(L, -1))
            rate = static_cast<float>(lua_tonumber(L, -1));
        lua_pop(L, 1);

        lua_pushstring(L, "physical");
        lua_rawget(L, 4);
        if (!lua_isnil(L, -1))
            physical = lua_toboolean(L, -1) != 0;
        lua_pop(L, 1);

        lua_pushstring(L, "fixed");
        lua_rawget(L, 4);
        if (!lua_isnil(L, -1))
            fixed = lua_toboolean(L, -1) != 0;
        lua_pop(L, 1);

        lua_pushstring(L, "notify");
        lua_rawget(L, 4);
        if (!lua_isnil(L, -1))
            notify = lua_toboolean(L, -1) != 0;
        lua_pop(L, 1);
    }

    auto weak = obj->weak_from_this();
    return lua->ensure_yield(*server, weak, [=](auto is_yield) {
        obj->stat.damage(value, from, critical, rate, physical, fixed, notify);
        return lua->ensure_resume(*server, weak, [=]() {
            return 0;
        });
    });
}

int builtin::life::builtin_mp_up(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto obj    = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    auto value  = (uint32_t)lua->tointeger(2);
    auto notify = argc < 3 || lua->toboolean(3);
    auto weak   = obj->weak_from_this();
    return lua->ensure_yield(*server, weak, [=](auto is_yield) {
        obj->stat.mp_up(value, nullptr, notify);
        return lua->ensure_resume(*server, weak, [=]() {
            return 0;
        });
    });
}

int builtin::life::builtin_mp_down(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto obj    = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    auto value  = (uint32_t)lua->tointeger(2);
    auto notify = argc < 3 || lua->toboolean(3);
    auto weak   = obj->weak_from_this();
    return lua->ensure_yield(*server, weak, [=](auto is_yield) {
        obj->stat.mp_down(value, nullptr, notify);
        return lua->ensure_resume(*server, weak, [=]() {
            return 0;
        });
    });
}

int builtin::life::builtin_action(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto obj    = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    auto action   = lua->tointeger(2);
    auto duration = lua->tointeger(3, static_cast<int>(DURATION::SPELL));
    auto sound    = (uint8_t)lua->tointeger(4, (uint8_t)0x00);
    auto weak     = obj->weak_from_this();
    return lua->ensure_yield(*server, weak, [=](auto is_yield) {
        obj->action(ACTION(action), DURATION(duration), sound);
        return lua->ensure_resume(*server, weak, [=]() {
            return 0;
        });
    });
}

int builtin::life::builtin_spell(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto obj    = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    if (lua->is_number(2))
    {
        auto index = (int)lua->tointeger(2);
        auto weak  = obj->weak_from_this();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            auto spell = obj->spells[index];

            return lua->ensure_resume(*server, weak, [=]() {
                if (spell == nullptr)
                    lua->pushnil();
                else
                    lua->pushobject(spell);

                return 1;
            });
        });
    }
    else if (lua->is_string(2))
    {
        auto name = lua->tostring(2);
        auto weak = obj->weak_from_this();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            auto spell = obj->spells.find(name);

            return lua->ensure_resume(*server, weak, [=]() {
                if (spell == nullptr)
                    lua->pushnil();
                else
                    lua->pushobject(spell);

                return 1;
            });
        });
    }
    else if (lua->is_userdata<fb::model::spell>(2))
    {
        auto model = lua->touserdata<fb::model::spell>(2);
        auto weak  = obj->weak_from_this();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            auto spell = obj->spells.find(*model);

            return lua->ensure_resume(*server, weak, [=]() {
                if (spell == nullptr)
                    lua->pushnil();
                else
                    lua->pushobject(spell);

                return 1;
            });
        });
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

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto obj    = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    auto weak = obj->weak_from_this();
    return lua->ensure_yield(*server, weak, [=](auto is_yield) {
        auto buffer = std::vector<std::shared_ptr<fb::game::spell>>();
        for (int i = 0; i < CONTAINER_CAPACITY; i++)
        {
            auto spell = obj->spells[i];
            if (spell != nullptr)
                buffer.push_back(spell);
        }

        return lua->ensure_resume(*server, weak, [=]() {
            lua->new_table();
            for (int i = 0; i < buffer.size(); i++)
            {
                lua->pushobject(buffer[i]);
                lua_rawseti(L, -2, i + 1);
            }

            return 1;
        });
    });
}

int builtin::life::builtin_cast(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
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

    auto map = obj->map();
    if (map == nullptr || map->objects.contains(you) == false)
        return 0;

    auto name  = lua->tostring(offset++);
    auto spell = table::spell.name2spell(name);
    if (spell == nullptr)
        return 0;

    auto x = lua::new_context();
    if (x == nullptr)
        return 0;

    auto count = 2;
#if defined DEBUG || defined _DEBUG
    x->load("scripts/spell.lua");
    x->load(spell->script);
#endif
    x->func(spell->cast);
    x->pushobject(obj);

    if (spell->type == SPELL_TYPE::TARGET)
    {
        if (you != nullptr)
            x->pushobject(you);
        else
            x->pushnil();

        count++;
    }

    x->pushobject(spell);
    std::ignore = x->call(count);
    return 0;
}

int builtin::life::builtin_cc(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto obj    = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    if (argc == 1)
    {
        auto weak = obj->weak_from_this();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            auto cc_value = static_cast<uint32_t>(obj->cc);

            return lua->ensure_resume(*server, weak, [=]() {
                lua->pushinteger(cc_value);
                return 1;
            });
        });
    }
    else
    {
        auto cc   = static_cast<CROWD_CONTROL>(lua->tointeger(2));
        auto weak = obj->weak_from_this();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            obj->cc.set(cc);

            return lua->ensure_resume(*server, weak, [=]() {
                return 0;
            });
        });
    }
}

int builtin::life::builtin_add_cc(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto obj    = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    if (argc == 1)
    {
        auto weak = obj->weak_from_this();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            auto cc_value = static_cast<uint32_t>(obj->cc);

            return lua->ensure_resume(*server, weak, [=]() {
                lua->pushinteger(cc_value);
                return 1;
            });
        });
    }
    else
    {
        auto cc   = static_cast<CROWD_CONTROL>(lua->tointeger(2));
        auto weak = obj->weak_from_this();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            obj->cc.add(cc);

            return lua->ensure_resume(*server, weak, [=]() {
                return 0;
            });
        });
    }
}

int builtin::life::builtin_remove_cc(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto obj    = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    auto cc   = static_cast<CROWD_CONTROL>(lua->tointeger(2));
    auto weak = obj->weak_from_this();
    return lua->ensure_yield(*server, weak, [=](auto is_yield) {
        obj->cc.remove(cc);
        return lua->ensure_resume(*server, weak, [=]() {
            return 0;
        });
    });
}

int builtin::life::builtin_attack(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto obj    = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    auto duration = lua->toenum(2, DURATION::ATTACK);
    auto weak     = obj->weak_from_this();
    return lua->ensure_yield(*server, weak, [=](auto is_yield) {
        async::awaitable_then(obj->attack(duration), [=](auto result) {
            try
            {
                result();
            }
            catch (const std::exception& e)
            {
                // Nothing to do
            }
            lua->ensure_resume(*server, weak, [=]() {
                return 0;
            });
        });
        return 0;
    });
}

int builtin::life::builtin_damage_rate(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto obj    = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    if (argc == 1)
    {
        auto weak = obj->weak_from_this();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            auto damage_rate_value = obj->damage_rate();

            return lua->ensure_resume(*server, weak, [=]() {
                lua->pushinteger(damage_rate_value);
                return 1;
            });
        });
    }
    else
    {
        auto value = lua->tointeger(2);
        auto weak  = obj->weak_from_this();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            obj->damage_rate(value);

            return lua->ensure_resume(*server, weak, [=]() {
                return 0;
            });
        });
    }
}

int builtin::life::builtin_damage_derate(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto obj    = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    if (argc == 1)
    {
        auto weak = obj->weak_from_this();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            auto damage_derate_value = obj->damage_derate();

            return lua->ensure_resume(*server, weak, [=]() {
                lua->pushinteger(damage_derate_value);
                return 1;
            });
        });
    }
    else
    {
        auto value = lua->tointeger(2);
        auto weak  = obj->weak_from_this();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            obj->damage_derate(value);

            return lua->ensure_resume(*server, weak, [=]() {
                return 0;
            });
        });
    }
}

int builtin::life::builtin_skill_damage_rate(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto obj    = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    if (argc == 1)
    {
        auto weak = obj->weak_from_this();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            auto skill_damage_rate_value = obj->skill_damage_rate();

            return lua->ensure_resume(*server, weak, [=]() {
                lua->pushinteger(skill_damage_rate_value);
                return 1;
            });
        });
    }
    else
    {
        auto value = lua->tointeger(2);
        auto weak  = obj->weak_from_this();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            obj->skill_damage_rate(value);

            return lua->ensure_resume(*server, weak, [=]() {
                return 0;
            });
        });
    }
}

int builtin::life::builtin_paralysis(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto obj    = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    if (argc == 1)
    {
        auto weak = obj->weak_from_this();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            auto paralysis_value = obj->paralysis();

            return lua->ensure_resume(*server, weak, [=]() {
                lua->pushboolean(paralysis_value);
                return 1;
            });
        });
    }
    else
    {
        auto value = lua->toboolean(2);
        auto weak  = obj->weak_from_this();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            obj->paralysis(value);

            return lua->ensure_resume(*server, weak, [=]() {
                return 0;
            });
        });
    }
}

int builtin::life::builtin_invincible(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto obj    = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    if (argc == 1)
    {
        auto weak = obj->weak_from_this();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            auto invincible_value = obj->invincible();

            return lua->ensure_resume(*server, weak, [=]() {
                lua->pushboolean(invincible_value);
                return 1;
            });
        });
    }
    else
    {
        auto value = lua->toboolean(2);
        auto weak  = obj->weak_from_this();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            obj->invincible(value);

            return lua->ensure_resume(*server, weak, [=]() {
                return 0;
            });
        });
    }
}

int builtin::life::builtin_cover(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto obj    = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    if (argc == 1)
    {
        auto weak = obj->weak_from_this();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            auto cover_value = obj->cover();

            return lua->ensure_resume(*server, weak, [=]() {
                lua->pushboolean(cover_value);
                return 1;
            });
        });
    }
    else
    {
        auto value = lua->toboolean(2);
        auto weak  = obj->weak_from_this();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            obj->cover(value);

            return lua->ensure_resume(*server, weak, [=]() {
                return 0;
            });
        });
    }
}

int builtin::life::builtin_base_hp(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto argc   = lua->argc();
    auto server = lua->env<fb::game::server>("server");
    auto obj    = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    auto weak = obj->weak_from_this();
    return lua->ensure_yield(*server, weak, [=](auto is_yield) {
        auto value = obj->stat.base_hp();
        return lua->ensure_resume(*server, weak, [=]() {
            lua->pushinteger(value);
            return 1;
        });
    });
}

int builtin::life::builtin_buff_hp(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto obj    = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    if (argc == 1)
    {
        auto weak = obj->weak_from_this();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            auto buff_hp_value = obj->stat.buff_hp();

            return lua->ensure_resume(*server, weak, [=]() {
                lua->pushinteger(buff_hp_value);
                return 1;
            });
        });
    }
    else
    {
        auto value = lua->tointeger(2);
        auto weak  = obj->weak_from_this();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            obj->stat.buff_hp(static_cast<int32_t>(value));

            return lua->ensure_resume(*server, weak, [=]() {
                return 0;
            });
        });
    }
}

int builtin::life::builtin_maxhp(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto obj    = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    auto weak = obj->weak_from_this();
    return lua->ensure_yield(*server, weak, [=](auto is_yield) {
        auto maxhp_value = obj->stat.maxhp();

        return lua->ensure_resume(*server, weak, [=]() {
            lua->pushinteger(maxhp_value);
            return 1;
        });
    });
}

int builtin::life::builtin_base_mp(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto obj    = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    auto weak = obj->weak_from_this();
    return lua->ensure_yield(*server, weak, [=](auto is_yield) {
        auto base_mp_value = obj->stat.base_mp();

        return lua->ensure_resume(*server, weak, [=]() {
            lua->pushinteger(base_mp_value);
            return 1;
        });
    });
}

int builtin::life::builtin_buff_mp(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto obj    = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    if (argc == 1)
    {
        auto weak = obj->weak_from_this();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            auto buff_mp_value = obj->stat.buff_mp();

            return lua->ensure_resume(*server, weak, [=]() {
                lua->pushinteger(buff_mp_value);
                return 1;
            });
        });
    }
    else
    {
        auto value = lua->tointeger(2);
        auto weak  = obj->weak_from_this();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            obj->stat.buff_mp(static_cast<int32_t>(value));

            return lua->ensure_resume(*server, weak, [=]() {
                return 0;
            });
        });
    }
}

int builtin::life::builtin_maxmp(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto obj    = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    auto weak = obj->weak_from_this();
    return lua->ensure_yield(*server, weak, [=](auto is_yield) {
        auto maxmp_value = obj->stat.maxmp();

        return lua->ensure_resume(*server, weak, [=]() {
            lua->pushinteger(maxmp_value);
            return 1;
        });
    });
}

int builtin::life::builtin_base_str(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto obj    = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    auto weak = obj->weak_from_this();
    return lua->ensure_yield(*server, weak, [=](auto is_yield) {
        auto base_str_value = obj->stat.base_str();

        return lua->ensure_resume(*server, weak, [=]() {
            lua->pushinteger(base_str_value);
            return 1;
        });
    });
}

int builtin::life::builtin_buff_str(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto obj    = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    if (argc == 1)
    {
        auto weak = obj->weak_from_this();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            auto buff_str_value = obj->stat.buff_str();

            return lua->ensure_resume(*server, weak, [=]() {
                lua->pushinteger(buff_str_value);
                return 1;
            });
        });
    }
    else
    {
        auto value = lua->tointeger(2);
        auto weak  = obj->weak_from_this();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            obj->stat.buff_str(value);

            return lua->ensure_resume(*server, weak, [=]() {
                return 0;
            });
        });
    }
}

int builtin::life::builtin_str(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto obj    = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    auto weak = obj->weak_from_this();
    return lua->ensure_yield(*server, weak, [=](auto is_yield) {
        auto str_value = obj->stat.str();

        return lua->ensure_resume(*server, weak, [=]() {
            lua->pushinteger(str_value);
            return 1;
        });
    });
}

int builtin::life::builtin_base_dex(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto obj    = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    auto weak = obj->weak_from_this();
    return lua->ensure_yield(*server, weak, [=](auto is_yield) {
        auto base_dex_value = obj->stat.base_dex();

        return lua->ensure_resume(*server, weak, [=]() {
            lua->pushinteger(base_dex_value);
            return 1;
        });
    });
}

int builtin::life::builtin_buff_dex(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto obj    = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    if (argc == 1)
    {
        auto weak = obj->weak_from_this();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            auto buff_dex_value = obj->stat.buff_dex();

            return lua->ensure_resume(*server, weak, [=]() {
                lua->pushinteger(buff_dex_value);
                return 1;
            });
        });
    }
    else
    {
        auto value = lua->tointeger(2);
        auto weak  = obj->weak_from_this();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            obj->stat.buff_dex(value);

            return lua->ensure_resume(*server, weak, [=]() {
                return 0;
            });
        });
    }
}

int builtin::life::builtin_dex(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto obj    = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    auto weak = obj->weak_from_this();
    return lua->ensure_yield(*server, weak, [=](auto is_yield) {
        auto dex_value = obj->stat.dex();

        return lua->ensure_resume(*server, weak, [=]() {
            lua->pushinteger(dex_value);
            return 1;
        });
    });
}

int builtin::life::builtin_base_int(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto obj    = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    auto weak = obj->weak_from_this();
    return lua->ensure_yield(*server, weak, [=](auto is_yield) {
        auto base_int_value = obj->stat.base_int();

        return lua->ensure_resume(*server, weak, [=]() {
            lua->pushinteger(base_int_value);
            return 1;
        });
    });
}

int builtin::life::builtin_buff_int(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto obj    = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    if (argc == 1)
    {
        auto weak = obj->weak_from_this();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            auto buff_int_value = obj->stat.buff_int();

            return lua->ensure_resume(*server, weak, [=]() {
                lua->pushinteger(buff_int_value);
                return 1;
            });
        });
    }
    else
    {
        auto value = lua->tointeger(2);
        auto weak  = obj->weak_from_this();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            obj->stat.buff_int(value);

            return lua->ensure_resume(*server, weak, [=]() {
                return 0;
            });
        });
    }
}

int builtin::life::builtin_intelligence(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto obj    = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    auto weak = obj->weak_from_this();
    return lua->ensure_yield(*server, weak, [=](auto is_yield) {
        auto intelligence_value = obj->stat.intelligence();

        return lua->ensure_resume(*server, weak, [=]() {
            lua->pushinteger(intelligence_value);
            return 1;
        });
    });
}

int builtin::life::builtin_base_phydef(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto obj    = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    auto weak = obj->weak_from_this();
    return lua->ensure_yield(*server, weak, [=](auto is_yield) {
        auto base_phydef_value = obj->stat.base_phydef();

        return lua->ensure_resume(*server, weak, [=]() {
            lua->pushinteger(base_phydef_value);
            return 1;
        });
    });
}

int builtin::life::builtin_buff_phydef(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto obj    = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    if (argc == 1)
    {
        auto weak = obj->weak_from_this();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            auto buff_phydef_value = obj->stat.buff_phydef();

            return lua->ensure_resume(*server, weak, [=]() {
                lua->pushinteger(buff_phydef_value);
                return 1;
            });
        });
    }
    else
    {
        auto value = lua->tointeger(2);
        auto weak  = obj->weak_from_this();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            obj->stat.buff_phydef(value);

            return lua->ensure_resume(*server, weak, [=]() {
                return 0;
            });
        });
    }
}

int builtin::life::builtin_phydef(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto obj    = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    auto weak = obj->weak_from_this();
    return lua->ensure_yield(*server, weak, [=](auto is_yield) {
        auto phydef_value = obj->stat.phydef();

        return lua->ensure_resume(*server, weak, [=]() {
            lua->pushinteger(phydef_value);
            return 1;
        });
    });
}

int builtin::life::builtin_base_magdef(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto obj    = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    auto weak = obj->weak_from_this();
    return lua->ensure_yield(*server, weak, [=](auto is_yield) {
        auto base_magdef_value = obj->stat.base_magdef();

        return lua->ensure_resume(*server, weak, [=]() {
            lua->pushinteger(base_magdef_value);
            return 1;
        });
    });
}

int builtin::life::builtin_buff_magdef(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto obj    = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    if (argc == 1)
    {
        auto weak = obj->weak_from_this();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            auto buff_magdef_value = obj->stat.buff_magdef();

            return lua->ensure_resume(*server, weak, [=]() {
                lua->pushinteger(buff_magdef_value);
                return 1;
            });
        });
    }
    else
    {
        auto value = lua->tointeger(2);
        auto weak  = obj->weak_from_this();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            obj->stat.buff_magdef(value);

            return lua->ensure_resume(*server, weak, [=]() {
                return 0;
            });
        });
    }
}

int builtin::life::builtin_magdef(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto obj    = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    auto weak = obj->weak_from_this();
    return lua->ensure_yield(*server, weak, [=](auto is_yield) {
        auto magdef_value = obj->stat.magdef();

        return lua->ensure_resume(*server, weak, [=]() {
            lua->pushinteger(magdef_value);
            return 1;
        });
    });
}

int builtin::life::builtin_base_dam(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto obj    = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    auto weak = obj->weak_from_this();
    return lua->ensure_yield(*server, weak, [=](auto is_yield) {
        auto base_dam_value = obj->stat.base_dam();

        return lua->ensure_resume(*server, weak, [=]() {
            lua->pushinteger(base_dam_value);
            return 1;
        });
    });
}

int builtin::life::builtin_buff_dam(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto obj    = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    if (argc == 1)
    {
        auto weak = obj->weak_from_this();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            auto buff_dam_value = obj->stat.buff_dam();

            return lua->ensure_resume(*server, weak, [=]() {
                lua->pushinteger(buff_dam_value);
                return 1;
            });
        });
    }
    else
    {
        auto value = lua->tointeger(2);
        auto weak  = obj->weak_from_this();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            obj->stat.buff_dam(static_cast<int8_t>(value));

            return lua->ensure_resume(*server, weak, [=]() {
                return 0;
            });
        });
    }
}

int builtin::life::builtin_dam(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto obj    = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    auto weak = obj->weak_from_this();
    return lua->ensure_yield(*server, weak, [=](auto is_yield) {
        auto dam_value = obj->stat.dam();

        return lua->ensure_resume(*server, weak, [=]() {
            lua->pushinteger(dam_value);
            return 1;
        });
    });
}

int builtin::life::builtin_base_hit(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto obj    = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    auto weak = obj->weak_from_this();
    return lua->ensure_yield(*server, weak, [=](auto is_yield) {
        auto base_hit_value = obj->stat.base_hit();

        return lua->ensure_resume(*server, weak, [=]() {
            lua->pushinteger(base_hit_value);
            return 1;
        });
    });
}

int builtin::life::builtin_buff_hit(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto obj    = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    if (argc == 1)
    {
        auto weak = obj->weak_from_this();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            auto buff_hit_value = obj->stat.buff_hit();

            return lua->ensure_resume(*server, weak, [=]() {
                lua->pushinteger(buff_hit_value);
                return 1;
            });
        });
    }
    else
    {
        auto value = lua->tointeger(2);
        auto weak  = obj->weak_from_this();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            obj->stat.buff_hit(static_cast<int8_t>(value));

            return lua->ensure_resume(*server, weak, [=]() {
                return 0;
            });
        });
    }
}

int builtin::life::builtin_hit(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto obj    = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    auto weak = obj->weak_from_this();
    return lua->ensure_yield(*server, weak, [=](auto is_yield) {
        auto hit_value = obj->stat.hit();

        return lua->ensure_resume(*server, weak, [=]() {
            lua->pushinteger(hit_value);
            return 1;
        });
    });
}

int builtin::life::builtin_normal_attack_damage(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto obj    = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    obj->assert_thread();

    auto size = lua->toenum(2, MOB_SIZE::SMALL);
    auto weak = obj->weak_from_this();
    return lua->ensure_yield(*server, weak, [=](auto is_yield) {
        auto damage = obj->normal_attack_damage(size);
        return lua->ensure_resume(*server, weak, [=]() {
            lua->pushinteger(damage);
            return 1;
        });
    });
}

int builtin::life::builtin_update(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto obj    = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    auto level = argc >= 2 ? lua->toenum(2, UPDATE_STATE_LEVEL::HP_MP | UPDATE_STATE_LEVEL::BASED)
                           : UPDATE_STATE_LEVEL::HP_MP | UPDATE_STATE_LEVEL::BASED;
    auto weak  = obj->weak_from_this();
    return lua->ensure_yield(*server, weak, [=](auto is_yield) {
        obj->update(level);
        return lua->ensure_resume(*server, weak, [=]() {
            return 0;
        });
    });
}