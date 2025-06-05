#include <fb/game/context.h>
#include <fb/game/life.h>
#include <fb/game/map.h>

using namespace fb::game;

// clang-format off
IMPLEMENT_LUA_EXTENSION(life, "fb.game.life")
{"__eq",                object::builtin::builtin_eq},
{"message",             life::builtin::builtin_message},
{"hp",                  life::builtin::builtin_hp},
{"mp",                  life::builtin::builtin_mp},
{"heal",                life::builtin::builtin_heal},
{"damage",              life::builtin::builtin_damage},
{"mp_up",               life::builtin::builtin_mp_up},
{"mp_down",             life::builtin::builtin_mp_down},
{"action",              life::builtin::builtin_action},
{"spell",               life::builtin::builtin_spell},
{"spells",              life::builtin::builtin_spells},
{"cast",                life::builtin::builtin_cast},
{"cc",                  life::builtin::builtin_cc},
{"add_cc",              life::builtin::builtin_add_cc},
{"remove_cc",           life::builtin::builtin_remove_cc},
{"attack",              life::builtin::builtin_attack},
{"damage_rate",         life::builtin::builtin_damage_rate},
{"damage_derate",       life::builtin::builtin_damage_derate},
{"skill_damage_rate",   life::builtin::builtin_skill_damage_rate},
{"paralysis",           life::builtin::builtin_paralysis},
{"invincible",          life::builtin::builtin_invincible},
{"cover",               life::builtin::builtin_cover},
{"base_hp",             life::builtin::builtin_base_hp},
{"buff_hp",             life::builtin::builtin_buff_hp},
{"maxhp",               life::builtin::builtin_maxhp},
{"base_mp",             life::builtin::builtin_base_mp},
{"buff_mp",             life::builtin::builtin_buff_mp},
{"maxmp",               life::builtin::builtin_maxmp},
{"base_str",            life::builtin::builtin_base_str},
{"buff_str",            life::builtin::builtin_buff_str},
{"str",                 life::builtin::builtin_str},
{"base_dex",            life::builtin::builtin_base_dex},
{"buff_dex",            life::builtin::builtin_buff_dex},
{"dex",                 life::builtin::builtin_dex},
{"base_int",            life::builtin::builtin_base_int},
{"buff_int",            life::builtin::builtin_buff_int},
{"int",                 life::builtin::builtin_intelligence},
{"base_phydef",         life::builtin::builtin_base_phydef},
{"buff_phydef",         life::builtin::builtin_buff_phydef},
{"phydef",              life::builtin::builtin_phydef},
{"base_magdef",         life::builtin::builtin_base_magdef},
{"buff_magdef",         life::builtin::builtin_buff_magdef},
{"magdef",              life::builtin::builtin_magdef},
{"base_dam",            life::builtin::builtin_base_dam},
{"buff_dam",            life::builtin::builtin_buff_dam},
{"dam",                 life::builtin::builtin_dam},
{"base_hit",            life::builtin::builtin_base_hit},
{"buff_hit",            life::builtin::builtin_buff_hit},
{"hit",                 life::builtin::builtin_hit},
END_LUA_EXTENSION; // clang-format on

int life::builtin::builtin_message(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto obj  = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr || ctx->alive(*obj) == false)
        return 0;

    if (obj->is(OBJECT_TYPE::CHARACTER) == false)
        return 0;

    auto ch      = static_cast<fb::game::character*>(obj);
    auto message = lua->tostring(2);
    auto type    = lua->toenum(3, MESSAGE_TYPE::STATE);

    return ctx->builtin(*ch, lua, 0, [=]() {
        ch->message(message, type);
    });
}

int life::builtin::builtin_hp(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto obj  = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr || ctx->alive(*obj) == false)
        return 0;

    if (argc == 1)
    {
        lua->pushinteger(obj->hp());
        return 1;
    }
    else
    {
        auto value = (uint32_t)lua->tointeger(2);
        obj->hp(value);
        return 0;
    }
}

int life::builtin::builtin_mp(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto obj  = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr || ctx->alive(*obj) == false)
        return 0;

    if (argc == 1)
    {
        lua->pushinteger(obj->mp());
        return 1;
    }
    else
    {
        auto value = (uint32_t)lua->tointeger(2);
        obj->mp(value);
        return 0;
    }
}

int life::builtin::builtin_heal(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto obj  = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr || ctx->alive(*obj) == false)
        return 0;

    auto value = (uint32_t)lua->tointeger(2);
    return ctx->builtin(*obj, lua, 0, [=]() {
        obj->heal(value);
    });
}

int life::builtin::builtin_damage(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto obj  = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr || ctx->alive(*obj) == false)
        return 0;

    auto value    = (uint32_t)lua->tointeger(2);
    auto from     = lua->touserdata<fb::game::life>(3);
    auto critical = lua->toboolean(4, false);
    return ctx->builtin(*obj, lua, 0, [=]() {
        obj->damage(value, from, critical);
    });
}

int life::builtin::builtin_mp_up(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto obj  = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr || ctx->alive(*obj) == false)
        return 0;

    auto value = (uint32_t)lua->tointeger(2);
    return ctx->builtin(*obj, lua, 0, [=]() {
        obj->mp_up(value);
    });
}

int life::builtin::builtin_mp_down(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto obj  = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr || ctx->alive(*obj) == false)
        return 0;

    auto value = (uint32_t)lua->tointeger(2);
    return ctx->builtin(*obj, lua, 0, [=]() {
        obj->mp_down(value);
    });
}

int life::builtin::builtin_action(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto obj  = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr || ctx->alive(*obj) == false)
        return 0;

    auto action   = lua->tointeger(2);
    auto duration = lua->tointeger(3, static_cast<int>(DURATION::SPELL));
    auto sound    = (uint8_t)lua->tointeger(4, (uint8_t)0x00);
    return ctx->builtin(*obj, lua, 0, [=]() {
        obj->action(ACTION(action), DURATION(duration), sound);
    });
}

int life::builtin::builtin_spell(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto obj  = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr || ctx->alive(*obj) == false)
        return 0;

    auto index = (int)lua->tointeger(2);
    return ctx->builtin(*obj, lua, 1, [=]() {
        auto spell = obj->spells[index];
        if (spell == nullptr)
            lua->pushnil();
        else
            lua->pushobject(spell);
    });
}

int life::builtin::builtin_spells(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto obj  = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr || ctx->alive(*obj) == false)
        return 0;

    return ctx->builtin(*obj, lua, 1, [=]() {
        lua->new_table();
        for (int i = 0; i < CONTAINER_CAPACITY; i++)
        {
            auto spell = obj->spells[i];
            if (spell == nullptr)
                continue;

            lua->pushobject(spell);
            lua_rawseti(L, -2, i + 1);
        }
    });
}

int life::builtin::builtin_cast(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx    = lua->env<fb::game::context>("context");
    auto argc   = lua->argc();
    auto offset = 1;
    auto obj    = lua->touserdata<fb::game::life>(offset++);
    if (obj == nullptr || ctx->alive(*obj) == false)
        return 0;

    fb::game::life* you = nullptr;
    if (argc == 2)
        you = obj;
    else if (lua_type(L, offset) == LUA_TUSERDATA)
        you = lua->touserdata<fb::game::life>(offset++);
    else
        you = nullptr;

    auto map = obj->map();
    if (map == nullptr || map->objects.contains(*you) == false)
        return 0;

    auto name  = lua->tostring(offset++);
    auto spell = ctx->model.spell.name2spell(name);
    if (spell == nullptr)
        return 0;

    auto x = lua::new_context();
    if (x == nullptr)
        return 0;

    auto count = 2;
#if defined DEBUG | defined _DEBUG
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

int life::builtin::builtin_cc(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto obj  = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr || ctx->alive(*obj) == false)
        return 0;

    auto cc = static_cast<CROWD_CONTROL>(lua->tointeger(2));
    auto n  = (argc == 1 ? 1 : 0);
    return ctx->builtin(*obj, lua, n, [=]() {
        if (argc == 1)
            lua->pushinteger(static_cast<uint32_t>(obj->crowd_control()));
        else
            obj->crowd_control(cc);
    });
}

int life::builtin::builtin_add_cc(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto obj  = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr || ctx->alive(*obj) == false)
        return 0;

    auto cc = static_cast<CROWD_CONTROL>(lua->tointeger(2));
    return ctx->builtin(*obj, lua, 0, [=]() {
        obj->add_cc(cc);
    });
}

int life::builtin::builtin_remove_cc(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto obj  = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr || ctx->alive(*obj) == false)
        return 0;

    auto cc = static_cast<CROWD_CONTROL>(lua->tointeger(2));
    return ctx->builtin(*obj, lua, 0, [=]() {
        obj->remove_cc(cc);
    });
}

int life::builtin::builtin_attack(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto obj  = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr || ctx->alive(*obj) == false)
        return 0;

    auto duration = lua->toenum(2, DURATION::ATTACK);
    return ctx->builtin(*obj, lua, 0, [=]() {
        obj->attack(duration);
    });
}

int life::builtin::builtin_damage_rate(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto obj  = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr || ctx->alive(*obj) == false)
        return 0;

    auto value = lua->tointeger(2);
    auto n     = (argc == 1 ? 1 : 0);
    return ctx->builtin(*obj, lua, n, [=]() {
        if (argc == 1)
            lua->pushinteger(obj->damage_rate());
        else
            obj->damage_rate(value);
    });
}

int life::builtin::builtin_damage_derate(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto obj  = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr || ctx->alive(*obj) == false)
        return 0;

    auto value = lua->tointeger(2);
    auto n     = (argc == 1 ? 1 : 0);
    return ctx->builtin(*obj, lua, n, [=]() {
        if (argc == 1)
            lua->pushinteger(obj->damage_derate());
        else
            obj->damage_derate(value);
    });
}

int life::builtin::builtin_skill_damage_rate(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto obj  = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr || ctx->alive(*obj) == false)
        return 0;

    auto value = lua->tointeger(2);
    auto n     = (argc == 1 ? 1 : 0);
    return ctx->builtin(*obj, lua, n, [=]() {
        if (argc == 1)
            lua->pushinteger(obj->skill_damage_rate());
        else
            obj->skill_damage_rate(value);
    });
}

int life::builtin::builtin_paralysis(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto obj  = lua->touserdata<life>(1);
    if (obj == nullptr || ctx->alive(*obj) == false)
        return 0;

    auto value = lua->toboolean(2);
    auto n     = (argc == 1 ? 1 : 0);
    return ctx->builtin(*obj, lua, n, [=]() {
        if (argc == 1)
            lua->pushboolean(obj->paralysis());
        else
            obj->paralysis(value);
    });
}

int life::builtin::builtin_invincible(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto obj  = lua->touserdata<life>(1);
    if (obj == nullptr || ctx->alive(*obj) == false)
        return 0;

    auto value = lua->toboolean(2);
    auto n     = (argc == 1 ? 1 : 0);
    return ctx->builtin(*obj, lua, n, [=]() {
        if (argc == 1)
            lua->pushboolean(obj->invincible());
        else
            obj->invincible(value);
    });
}

int life::builtin::builtin_cover(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto obj  = lua->touserdata<life>(1);
    if (obj == nullptr || ctx->alive(*obj) == false)
        return 0;

    auto value = lua->toboolean(2);
    auto n     = (argc == 1 ? 1 : 0);
    return ctx->builtin(*obj, lua, n, [=]() {
        if (argc == 1)
            lua->pushboolean(obj->cover());
        else
            obj->cover(value);
    });
}

int life::builtin::builtin_base_hp(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx = lua->env<fb::game::context>("context");
    auto obj = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr || ctx->alive(*obj) == false)
        return 0;

    return ctx->builtin(*obj, lua, 1, [=]() {
        lua->pushinteger(obj->base_hp());
    });

    return 1;
}

int life::builtin::builtin_buff_hp(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto obj  = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr || ctx->alive(*obj) == false)
        return 0;

    auto value = lua->tointeger(2);
    auto n     = (argc == 1 ? 1 : 0);
    return ctx->builtin(*obj, lua, n, [=]() {
        if (argc == 1)
            lua->pushinteger(obj->buff_hp());
        else
            obj->buff_hp(value);
    });
}

int life::builtin::builtin_maxhp(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx = lua->env<fb::game::context>("context");
    auto obj = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr || ctx->alive(*obj) == false)
        return 0;

    return ctx->builtin(*obj, lua, 1, [=]() {
        lua->pushinteger(obj->maxhp());
    });
}

int life::builtin::builtin_base_mp(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx = lua->env<fb::game::context>("context");
    auto obj = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr || ctx->alive(*obj) == false)
        return 0;

    return ctx->builtin(*obj, lua, 1, [=]() {
        lua->pushinteger(obj->base_mp());
    });
}

int life::builtin::builtin_buff_mp(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto obj  = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr || ctx->alive(*obj) == false)
        return 0;

    auto value = lua->tointeger(2);
    auto n     = (argc == 1 ? 1 : 0);
    return ctx->builtin(*obj, lua, n, [=]() {
        if (argc == 1)
            lua->pushinteger(obj->buff_mp());
        else
            obj->buff_mp(value);
    });
}

int life::builtin::builtin_maxmp(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx = lua->env<fb::game::context>("context");
    auto obj = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr || ctx->alive(*obj) == false)
        return 0;

    return ctx->builtin(*obj, lua, 1, [=]() {
        lua->pushinteger(obj->maxmp());
    });
}

int life::builtin::builtin_base_str(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx = lua->env<fb::game::context>("context");
    auto obj = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr || ctx->alive(*obj) == false)
        return 0;

    return ctx->builtin(*obj, lua, 1, [=]() {
        lua->pushinteger(obj->base_str());
    });
}

int life::builtin::builtin_buff_str(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto obj  = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr || ctx->alive(*obj) == false)
        return 0;

    auto value = lua->tointeger(2);
    auto n     = (argc == 1 ? 1 : 0);
    return ctx->builtin(*obj, lua, n, [=]() {
        if (argc == 1)
            lua->pushinteger(obj->buff_str());
        else
            obj->buff_str(value);
    });
}

int life::builtin::builtin_str(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx = lua->env<fb::game::context>("context");
    auto obj = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr || ctx->alive(*obj) == false)
        return 0;

    return ctx->builtin(*obj, lua, 1, [=]() {
        lua->pushinteger(obj->str());
    });
}

int life::builtin::builtin_base_dex(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx = lua->env<fb::game::context>("context");
    auto obj = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr || ctx->alive(*obj) == false)
        return 0;

    return ctx->builtin(*obj, lua, 1, [=]() {
        lua->pushinteger(obj->base_dex());
    });
}

int life::builtin::builtin_buff_dex(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto obj  = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr || ctx->alive(*obj) == false)
        return 0;

    auto value = lua->tointeger(2);
    auto n     = (argc == 1 ? 1 : 0);
    return ctx->builtin(*obj, lua, n, [=]() {
        if (argc == 1)
            lua->pushinteger(obj->buff_dex());
        else
            obj->buff_dex(value);
    });
}

int life::builtin::builtin_dex(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx = lua->env<fb::game::context>("context");
    auto obj = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr || ctx->alive(*obj) == false)
        return 0;

    return ctx->builtin(*obj, lua, 1, [=]() {
        lua->pushinteger(obj->dex());
    });
}

int life::builtin::builtin_base_int(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx = lua->env<fb::game::context>("context");
    auto obj = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr || ctx->alive(*obj) == false)
        return 0;

    return ctx->builtin(*obj, lua, 1, [=]() {
        lua->pushinteger(obj->base_int());
    });
}

int life::builtin::builtin_buff_int(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto obj  = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr || ctx->alive(*obj) == false)
        return 0;

    auto value = lua->tointeger(2);
    auto n     = (argc == 1 ? 1 : 0);
    return ctx->builtin(*obj, lua, n, [=]() {
        if (argc == 1)
            lua->pushinteger(obj->buff_int());
        else
            obj->buff_int(value);
    });
}

int life::builtin::builtin_intelligence(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx = lua->env<fb::game::context>("context");
    auto obj = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr || ctx->alive(*obj) == false)
        return 0;

    return ctx->builtin(*obj, lua, 1, [=]() {
        lua->pushinteger(obj->intelligence());
    });
}

int life::builtin::builtin_base_phydef(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx = lua->env<fb::game::context>("context");
    auto obj = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr || ctx->alive(*obj) == false)
        return 0;

    return ctx->builtin(*obj, lua, 1, [=]() {
        lua->pushinteger(obj->base_phydef());
    });
}

int life::builtin::builtin_buff_phydef(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto obj  = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr || ctx->alive(*obj) == false)
        return 0;

    auto value = lua->tointeger(2);
    auto n     = (argc == 1 ? 1 : 0);
    return ctx->builtin(*obj, lua, n, [=]() {
        if (argc == 1)
            lua->pushinteger(obj->buff_phydef());
        else
            obj->buff_phydef(value);
    });
}

int life::builtin::builtin_phydef(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx = lua->env<fb::game::context>("context");
    auto obj = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr || ctx->alive(*obj) == false)
        return 0;

    return ctx->builtin(*obj, lua, 1, [=]() {
        lua->pushinteger(obj->phydef());
    });
}

int life::builtin::builtin_base_magdef(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx = lua->env<fb::game::context>("context");
    auto obj = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr || ctx->alive(*obj) == false)
        return 0;

    return ctx->builtin(*obj, lua, 1, [=]() {
        lua->pushinteger(obj->base_magdef());
    });
}

int life::builtin::builtin_buff_magdef(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto obj  = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr || ctx->alive(*obj) == false)
        return 0;

    auto value = lua->tointeger(2);
    auto n     = (argc == 1 ? 1 : 0);
    return ctx->builtin(*obj, lua, n, [=]() {
        if (argc == 1)
            lua->pushinteger(obj->buff_magdef());
        else
            obj->buff_magdef(value);
    });
}

int life::builtin::builtin_magdef(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx = lua->env<fb::game::context>("context");
    auto obj = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr || ctx->alive(*obj) == false)
        return 0;

    return ctx->builtin(*obj, lua, 1, [=]() {
        lua->pushinteger(obj->magdef());
    });
}

int life::builtin::builtin_base_dam(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx = lua->env<fb::game::context>("context");
    auto obj = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr || ctx->alive(*obj) == false)
        return 0;

    return ctx->builtin(*obj, lua, 1, [=]() {
        lua->pushinteger(obj->base_dam());
    });
}

int life::builtin::builtin_buff_dam(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto obj  = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr || ctx->alive(*obj) == false)
        return 0;

    auto value = lua->tointeger(2);
    auto n     = (argc == 1 ? 1 : 0);
    return ctx->builtin(*obj, lua, n, [=]() {
        if (argc == 1)
            lua->pushinteger(obj->buff_dam());
        else
            obj->buff_dam(value);
    });
}

int life::builtin::builtin_dam(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx = lua->env<fb::game::context>("context");
    auto obj = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr || ctx->alive(*obj) == false)
        return 0;

    return ctx->builtin(*obj, lua, 1, [=]() {
        lua->pushinteger(obj->dam());
    });
}

int life::builtin::builtin_base_hit(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx = lua->env<fb::game::context>("context");
    auto obj = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr || ctx->alive(*obj) == false)
        return 0;

    return ctx->builtin(*obj, lua, 1, [=]() {
        lua->pushinteger(obj->base_hit());
    });
}

int life::builtin::builtin_buff_hit(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto obj  = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr || ctx->alive(*obj) == false)
        return 0;

    auto value = lua->tointeger(2);
    auto n     = (argc == 1 ? 1 : 0);
    return ctx->builtin(*obj, lua, n, [=]() {
        if (argc == 1)
            lua->pushinteger(obj->buff_hit());
        else
            obj->buff_hit(value);
    });
}

int life::builtin::builtin_hit(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx = lua->env<fb::game::context>("context");
    auto obj = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr || ctx->alive(*obj) == false)
        return 0;

    return ctx->builtin(*obj, lua, 1, [=]() {
        lua->pushinteger(obj->hit());
    });
}