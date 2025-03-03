#include <fb/game/context.h>
#include <fb/game/life.h>
#include <fb/game/map.h>

// clang-format off
IMPLEMENT_LUA_EXTENSION(fb::game::life, "fb.game.life")
{"__eq",                fb::game::object::builtin_eq},
{"message",             fb::game::life::builtin_message},
{"hp",                  fb::game::life::builtin_hp},
{"mp",                  fb::game::life::builtin_mp},
{"heal",                fb::game::life::builtin_heal},
{"damage",              fb::game::life::builtin_damage},
{"mp_up",               fb::game::life::builtin_mp_up},
{"mp_down",             fb::game::life::builtin_mp_down},
{"action",              fb::game::life::builtin_action},
{"spell",               fb::game::life::builtin_spell},
{"spells",              fb::game::life::builtin_spells},
{"cast",                fb::game::life::builtin_cast},
{"cc",                  fb::game::life::builtin_cc},
{"add_cc",              fb::game::life::builtin_add_cc},
{"remove_cc",           fb::game::life::builtin_remove_cc},
{"attack",              fb::game::life::builtin_attack},
{"damage_rate",         fb::game::life::builtin_damage_rate},
{"damage_derate",       fb::game::life::builtin_damage_derate},
{"skill_damage_rate",   fb::game::life::builtin_skill_damage_rate},
{"paralysis",           fb::game::life::builtin_paralysis},
{"invincible",          fb::game::life::builtin_invincible},
{"cover",               fb::game::life::builtin_cover},
{"base_hp",             fb::game::life::builtin_base_hp},
{"buff_hp",             fb::game::life::builtin_buff_hp},
{"maxhp",               fb::game::life::builtin_maxhp},
{"base_mp",             fb::game::life::builtin_base_mp},
{"buff_mp",             fb::game::life::builtin_buff_mp},
{"maxmp",               fb::game::life::builtin_maxmp},
{"base_str",            fb::game::life::builtin_base_str},
{"buff_str",            fb::game::life::builtin_buff_str},
{"str",                 fb::game::life::builtin_str},
{"base_dex",            fb::game::life::builtin_base_dex},
{"buff_dex",            fb::game::life::builtin_buff_dex},
{"dex",                 fb::game::life::builtin_dex},
{"base_int",            fb::game::life::builtin_base_int},
{"buff_int",            fb::game::life::builtin_buff_int},
{"int",                 fb::game::life::builtin_intelligence},
{"base_phydef",         fb::game::life::builtin_base_phydef},
{"buff_phydef",         fb::game::life::builtin_buff_phydef},
{"phydef",              fb::game::life::builtin_phydef},
{"base_magdef",         fb::game::life::builtin_base_magdef},
{"buff_magdef",         fb::game::life::builtin_buff_magdef},
{"magdef",              fb::game::life::builtin_magdef},
{"base_dam",            fb::game::life::builtin_base_dam},
{"buff_dam",            fb::game::life::builtin_buff_dam},
{"dam",                 fb::game::life::builtin_dam},
{"base_hit",            fb::game::life::builtin_base_hit},
{"buff_hit",            fb::game::life::builtin_buff_hit},
{"hit",                 fb::game::life::builtin_hit},
END_LUA_EXTENSION; // clang-format on

int fb::game::life::builtin_message(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto obj  = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    if (obj->is(OBJECT_TYPE::CHARACTER) == false)
        return 0;

    auto ch      = static_cast<fb::game::character*>(obj);
    auto message = lua->tostring(2);
    auto type    = argc < 3 ? MESSAGE_TYPE::STATE : static_cast<MESSAGE_TYPE>(lua->tointeger(3));

    return ctx->builtin(*ch, lua, 0, [=]() -> async::task<void> {
        ch->message(message, type);
        co_return;
    });
}

int fb::game::life::builtin_hp(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto argc    = lua->argc();
    auto object  = lua->touserdata<fb::game::life>(1);
    if (object == nullptr)
        return 0;

    if (argc == 1)
    {
        lua->pushinteger(object->hp());
        return 1;
    }
    else
    {
        auto value = (uint32_t)lua->tointeger(2);
        object->hp(value);
        return 0;
    }
}

int fb::game::life::builtin_mp(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto argc    = lua->argc();
    auto object  = lua->touserdata<fb::game::life>(1);
    if (object == nullptr)
        return 0;

    if (argc == 1)
    {
        lua->pushinteger(object->mp());
        return 1;
    }
    else
    {
        auto value = (uint32_t)lua->tointeger(2);
        object->mp(value);
        return 0;
    }
}

int fb::game::life::builtin_heal(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto argc    = lua->argc();
    auto obj     = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    auto value = (uint32_t)lua->tointeger(2);
    return context->builtin(*obj, lua, 0, [=]() -> async::task<void> {
        obj->heal(value);
        co_return;
    });
}

int fb::game::life::builtin_damage(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto argc    = lua->argc();
    auto obj     = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    auto value = (uint32_t)lua->tointeger(2);
    auto from  = static_cast<fb::game::object*>(nullptr);
    if (argc >= 3 && lua_type(L, 3) == LUA_TUSERDATA)
        from = lua->touserdata<fb::game::object>(3);

    auto critical = argc >= 4 ? lua->toboolean(4) : false;
    return context->builtin(*obj, lua, 0, [=]() -> async::task<void> {
        obj->damage(value, from, critical);
        co_return;
    });
}

int fb::game::life::builtin_mp_up(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto argc    = lua->argc();
    auto obj     = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    auto value = (uint32_t)lua->tointeger(2);
    return context->builtin(*obj, lua, 0, [=]() -> async::task<void> {
        obj->mp_up(value);
        co_return;
    });
}

int fb::game::life::builtin_mp_down(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto argc    = lua->argc();
    auto obj     = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    auto value = (uint32_t)lua->tointeger(2);
    return context->builtin(*obj, lua, 0, [=]() -> async::task<void> {
        obj->mp_down(value);
        co_return;
    });
}

int fb::game::life::builtin_action(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto argc    = lua->argc();
    auto obj     = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    auto action   = lua->tointeger(2);
    auto duration = argc < 3 ? static_cast<int>(DURATION::SPELL) : lua->tointeger(3);
    auto sound    = argc < 4 ? (uint8_t)0x00 : (uint8_t)lua->tointeger(4);
    return context->builtin(*obj, lua, 0, [=]() -> async::task<void> {
        obj->action(ACTION(action), DURATION(duration), sound);
        co_return;
    });
}

int fb::game::life::builtin_spell(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto argc    = lua->argc();
    auto life    = lua->touserdata<fb::game::life>(1);
    if (life == nullptr)
        return 0;

    auto index = (int)lua->tointeger(2);
    return context->builtin(*life, lua, 1, [=]() -> async::task<void> {
        auto spell = life->spells[index];
        if (spell == nullptr)
            lua->pushnil();
        else
            lua->pushobject(spell);
        co_return;
    });
}

int fb::game::life::builtin_spells(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto argc    = lua->argc();
    auto life    = lua->touserdata<fb::game::life>(1);
    if (life == nullptr)
        return 0;

    return context->builtin(*life, lua, 1, [=]() -> async::task<void> {
        lua->new_table();
        for (int i = 0; i < CONTAINER_CAPACITY; i++)
        {
            auto spell = life->spells[i];
            if (spell == nullptr)
                continue;

            lua->pushobject(spell);
            lua_rawseti(L, -2, i);
        }
        co_return;
    });
}

int fb::game::life::builtin_cast(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto argc    = lua->argc();
    auto offset  = 1;
    auto me      = lua->touserdata<fb::game::life>(offset++);
    if (me == nullptr)
        return 0;

    fb::game::life* you = nullptr;
    if (argc == 2)
        you = me;
    else if (lua_type(L, offset) == LUA_TUSERDATA)
        you = lua->touserdata<fb::game::life>(offset++);
    else
        you = nullptr;

    auto map = me->map();
    if (map == nullptr || map->objects.contains(*you) == false)
        return 0;

    auto name  = lua->tostring(offset++);
    auto spell = context->model.spell.name2spell(name);
    if (spell == nullptr)
        return 0;

    auto x = lua::new_context();
    if (x == nullptr)
        return 0;

#if defined DEBUG | defined _DEBUG
    x->load("scripts/spell.lua");
    x->load(spell->script);
#endif
    x->func(spell->cast);
    x->pushobject(me);

    if (argc > 2)
    {
        if (you != nullptr)
            x->pushobject(you);
        else
            x->pushnil();
    }

    x->pushobject(spell);
    x->resume(argc);
    return 0;
}

int fb::game::life::builtin_cc(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto argc    = lua->argc();
    auto me      = lua->touserdata<fb::game::life>(1);
    if (me == nullptr)
        return 0;

    auto cc = static_cast<CROWD_CONTROL>(lua->tointeger(2));
    auto n  = (argc == 1 ? 1 : 0);
    return context->builtin(*me, lua, n, [=]() -> async::task<void> {
        if (argc == 1)
            lua->pushinteger(static_cast<uint32_t>(me->crowd_control()));
        else
            me->crowd_control(cc);

        co_return;
    });
}

int fb::game::life::builtin_add_cc(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto argc    = lua->argc();
    auto me      = lua->touserdata<fb::game::life>(1);
    if (me == nullptr)
        return 0;

    auto cc = static_cast<CROWD_CONTROL>(lua->tointeger(2));
    return context->builtin(*me, lua, 0, [=]() -> async::task<void> {
        me->add_cc(cc);
        co_return;
    });
}

int fb::game::life::builtin_remove_cc(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto argc    = lua->argc();
    auto me      = lua->touserdata<fb::game::life>(1);
    if (me == nullptr)
        return 0;

    auto cc = static_cast<CROWD_CONTROL>(lua->tointeger(2));
    return context->builtin(*me, lua, 0, [=]() -> async::task<void> {
        me->remove_cc(cc);
        co_return;
    });
}

int fb::game::life::builtin_attack(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto argc    = lua->argc();
    auto me      = lua->touserdata<fb::game::life>(1);
    if (me == nullptr)
        return 0;

    auto duration = argc >= 2 ? static_cast<DURATION>(lua->tointeger(2)) : DURATION::ATTACK;
    return context->builtin(*me, lua, 0, [=]() -> async::task<void> {
        me->attack(duration);
        co_return;
    });
}

int fb::game::life::builtin_damage_rate(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto argc    = lua->argc();
    auto me      = lua->touserdata<fb::game::life>(1);
    if (me == nullptr)
        return 0;

    auto value = lua->tointeger(2);
    auto n     = (argc == 1 ? 1 : 0);
    return context->builtin(*me, lua, n, [=]() -> async::task<void> {
        if (argc == 1)
            lua->pushinteger(me->damage_rate());
        else
            me->damage_rate(value);

        co_return;
    });
}

int fb::game::life::builtin_damage_derate(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto argc    = lua->argc();
    auto me      = lua->touserdata<fb::game::life>(1);
    if (me == nullptr)
        return 0;

    auto value = lua->tointeger(2);
    auto n     = (argc == 1 ? 1 : 0);
    return context->builtin(*me, lua, n, [=]() -> async::task<void> {
        if (argc == 1)
            lua->pushinteger(me->damage_derate());
        else
            me->damage_derate(value);

        co_return;
    });
}

int fb::game::life::builtin_skill_damage_rate(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto argc    = lua->argc();
    auto me      = lua->touserdata<fb::game::life>(1);
    if (me == nullptr)
        return 0;

    auto value = lua->tointeger(2);
    auto n     = (argc == 1 ? 1 : 0);
    return context->builtin(*me, lua, n, [=]() -> async::task<void> {
        if (argc == 1)
            lua->pushinteger(me->skill_damage_rate());
        else
            me->skill_damage_rate(value);

        co_return;
    });
}

int fb::game::life::builtin_paralysis(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto argc    = lua->argc();
    auto obj     = lua->touserdata<life>(1);
    if (obj == nullptr)
        return 0;

    auto value = lua->toboolean(2);
    auto n     = (argc == 1 ? 1 : 0);
    return context->builtin(*obj, lua, n, [=]() -> async::task<void> {
        if (argc == 1)
            lua->pushboolean(obj->paralysis());
        else
            obj->paralysis(value);

        co_return;
    });
}

int fb::game::life::builtin_invincible(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto argc    = lua->argc();
    auto obj     = lua->touserdata<life>(1);
    if (obj == nullptr)
        return 0;

    auto value = lua->toboolean(2);
    auto n     = (argc == 1 ? 1 : 0);
    return context->builtin(*obj, lua, n, [=]() -> async::task<void> {
        if (argc == 1)
            lua->pushboolean(obj->invincible());
        else
            obj->invincible(value);

        co_return;
    });
}

int fb::game::life::builtin_cover(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto argc    = lua->argc();
    auto obj     = lua->touserdata<life>(1);
    if (obj == nullptr)
        return 0;

    auto value = lua->toboolean(2);
    auto n     = (argc == 1 ? 1 : 0);
    return context->builtin(*obj, lua, n, [=]() -> async::task<void> {
        if (argc == 1)
            lua->pushboolean(obj->cover());
        else
            obj->cover(value);

        co_return;
    });
}

int fb::game::life::builtin_base_hp(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto object  = lua->touserdata<fb::game::life>(1);
    if (object == nullptr)
        return 0;

    return context->builtin(*object, lua, 1, [=]() -> async::task<void> {
        lua->pushinteger(object->base_hp());
        co_return;
    });

    return 1;
}

int fb::game::life::builtin_buff_hp(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto argc    = lua->argc();
    auto object  = lua->touserdata<fb::game::life>(1);
    if (object == nullptr)
        return 0;

    auto value = lua->tointeger(2);
    auto n     = (argc == 1 ? 1 : 0);
    return context->builtin(*object, lua, n, [=]() -> async::task<void> {
        if (argc == 1)
            lua->pushinteger(object->buff_hp());
        else
            object->buff_hp(value);

        co_return;
    });
}

int fb::game::life::builtin_maxhp(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto object  = lua->touserdata<fb::game::life>(1);
    if (object == nullptr)
        return 0;

    return context->builtin(*object, lua, 1, [=]() -> async::task<void> {
        lua->pushinteger(object->maxhp());
        co_return;
    });
}

int fb::game::life::builtin_base_mp(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto object  = lua->touserdata<fb::game::life>(1);
    if (object == nullptr)
        return 0;

    return context->builtin(*object, lua, 1, [=]() -> async::task<void> {
        lua->pushinteger(object->base_mp());
        co_return;
    });
}

int fb::game::life::builtin_buff_mp(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto argc    = lua->argc();
    auto object  = lua->touserdata<fb::game::life>(1);
    if (object == nullptr)
        return 0;

    auto value = lua->tointeger(2);
    auto n     = (argc == 1 ? 1 : 0);
    return context->builtin(*object, lua, n, [=]() -> async::task<void> {
        if (argc == 1)
            lua->pushinteger(object->buff_mp());
        else
            object->buff_mp(value);

        co_return;
    });
}

int fb::game::life::builtin_maxmp(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto object  = lua->touserdata<fb::game::life>(1);
    if (object == nullptr)
        return 0;

    return context->builtin(*object, lua, 1, [=]() -> async::task<void> {
        lua->pushinteger(object->maxmp());
        co_return;
    });
}

int fb::game::life::builtin_base_str(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto object  = lua->touserdata<fb::game::life>(1);
    if (object == nullptr)
        return 0;

    return context->builtin(*object, lua, 1, [=]() -> async::task<void> {
        lua->pushinteger(object->base_str());
        co_return;
    });
}

int fb::game::life::builtin_buff_str(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto argc    = lua->argc();
    auto object  = lua->touserdata<fb::game::life>(1);
    if (object == nullptr)
        return 0;

    auto value = lua->tointeger(2);
    auto n     = (argc == 1 ? 1 : 0);
    return context->builtin(*object, lua, n, [=]() -> async::task<void> {
        if (argc == 1)
            lua->pushinteger(object->buff_str());
        else
            object->buff_str(value);

        co_return;
    });
}

int fb::game::life::builtin_str(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto object  = lua->touserdata<fb::game::life>(1);
    if (object == nullptr)
        return 0;

    return context->builtin(*object, lua, 1, [=]() -> async::task<void> {
        lua->pushinteger(object->str());
        co_return;
    });
}

int fb::game::life::builtin_base_dex(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto object  = lua->touserdata<fb::game::life>(1);
    if (object == nullptr)
        return 0;

    return context->builtin(*object, lua, 1, [=]() -> async::task<void> {
        lua->pushinteger(object->base_dex());
        co_return;
    });
}

int fb::game::life::builtin_buff_dex(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto argc    = lua->argc();
    auto object  = lua->touserdata<fb::game::life>(1);
    if (object == nullptr)
        return 0;

    auto value = lua->tointeger(2);
    auto n     = (argc == 1 ? 1 : 0);
    return context->builtin(*object, lua, n, [=]() -> async::task<void> {
        if (argc == 1)
            lua->pushinteger(object->buff_dex());
        else
            object->buff_dex(value);

        co_return;
    });
}

int fb::game::life::builtin_dex(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto object  = lua->touserdata<fb::game::life>(1);
    if (object == nullptr)
        return 0;

    return context->builtin(*object, lua, 1, [=]() -> async::task<void> {
        lua->pushinteger(object->dex());
        co_return;
    });
}

int fb::game::life::builtin_base_int(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto object  = lua->touserdata<fb::game::life>(1);
    if (object == nullptr)
        return 0;

    return context->builtin(*object, lua, 1, [=]() -> async::task<void> {
        lua->pushinteger(object->base_int());
        co_return;
    });
}

int fb::game::life::builtin_buff_int(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto argc    = lua->argc();
    auto object  = lua->touserdata<fb::game::life>(1);
    if (object == nullptr)
        return 0;

    auto value = lua->tointeger(2);
    auto n     = (argc == 1 ? 1 : 0);
    return context->builtin(*object, lua, n, [=]() -> async::task<void> {
        if (argc == 1)
            lua->pushinteger(object->buff_int());
        else
            object->buff_int(value);

        co_return;
    });
}

int fb::game::life::builtin_intelligence(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto object  = lua->touserdata<fb::game::life>(1);
    if (object == nullptr)
        return 0;

    return context->builtin(*object, lua, 1, [=]() -> async::task<void> {
        lua->pushinteger(object->intelligence());
        co_return;
    });
}

int fb::game::life::builtin_base_phydef(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto object  = lua->touserdata<fb::game::life>(1);
    if (object == nullptr)
        return 0;

    return context->builtin(*object, lua, 1, [=]() -> async::task<void> {
        lua->pushinteger(object->base_phydef());
        co_return;
    });
}

int fb::game::life::builtin_buff_phydef(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto argc    = lua->argc();
    auto object  = lua->touserdata<fb::game::life>(1);
    if (object == nullptr)
        return 0;

    auto value = lua->tointeger(2);
    auto n     = (argc == 1 ? 1 : 0);
    return context->builtin(*object, lua, n, [=]() -> async::task<void> {
        if (argc == 1)
            lua->pushinteger(object->buff_phydef());
        else
            object->buff_phydef(value);

        co_return;
    });
}

int fb::game::life::builtin_phydef(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto object  = lua->touserdata<fb::game::life>(1);
    if (object == nullptr)
        return 0;

    return context->builtin(*object, lua, 1, [=]() -> async::task<void> {
        lua->pushinteger(object->phydef());
        co_return;
    });
}

int fb::game::life::builtin_base_magdef(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto object  = lua->touserdata<fb::game::life>(1);
    if (object == nullptr)
        return 0;

    return context->builtin(*object, lua, 1, [=]() -> async::task<void> {
        lua->pushinteger(object->base_magdef());
        co_return;
    });
}

int fb::game::life::builtin_buff_magdef(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto argc    = lua->argc();
    auto object  = lua->touserdata<fb::game::life>(1);
    if (object == nullptr)
        return 0;

    auto value = lua->tointeger(2);
    auto n     = (argc == 1 ? 1 : 0);
    return context->builtin(*object, lua, n, [=]() -> async::task<void> {
        if (argc == 1)
            lua->pushinteger(object->buff_magdef());
        else
            object->buff_magdef(value);

        co_return;
    });
}

int fb::game::life::builtin_magdef(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto object  = lua->touserdata<fb::game::life>(1);
    if (object == nullptr)
        return 0;

    return context->builtin(*object, lua, 1, [=]() -> async::task<void> {
        lua->pushinteger(object->magdef());
        co_return;
    });
}

int fb::game::life::builtin_base_dam(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto object  = lua->touserdata<fb::game::life>(1);
    if (object == nullptr)
        return 0;

    return context->builtin(*object, lua, 1, [=]() -> async::task<void> {
        lua->pushinteger(object->base_dam());
        co_return;
    });
}

int fb::game::life::builtin_buff_dam(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto argc    = lua->argc();
    auto object  = lua->touserdata<fb::game::life>(1);
    if (object == nullptr)
        return 0;

    auto value = lua->tointeger(2);
    auto n     = (argc == 1 ? 1 : 0);
    return context->builtin(*object, lua, n, [=]() -> async::task<void> {
        if (argc == 1)
            lua->pushinteger(object->buff_dam());
        else
            object->buff_dam(value);

        co_return;
    });
}

int fb::game::life::builtin_dam(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto object  = lua->touserdata<fb::game::life>(1);
    if (object == nullptr)
        return 0;

    return context->builtin(*object, lua, 1, [=]() -> async::task<void> {
        lua->pushinteger(object->dam());
        co_return;
    });
}

int fb::game::life::builtin_base_hit(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto object  = lua->touserdata<fb::game::life>(1);
    if (object == nullptr)
        return 0;

    return context->builtin(*object, lua, 1, [=]() -> async::task<void> {
        lua->pushinteger(object->base_hit());
        co_return;
    });
}

int fb::game::life::builtin_buff_hit(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto argc    = lua->argc();
    auto object  = lua->touserdata<fb::game::life>(1);
    if (object == nullptr)
        return 0;

    auto value = lua->tointeger(2);
    auto n     = (argc == 1 ? 1 : 0);
    return context->builtin(*object, lua, n, [=]() -> async::task<void> {
        if (argc == 1)
            lua->pushinteger(object->buff_hit());
        else
            object->buff_hit(value);

        co_return;
    });
}

int fb::game::life::builtin_hit(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto object  = lua->touserdata<fb::game::life>(1);
    if (object == nullptr)
        return 0;

    return context->builtin(*object, lua, 1, [=]() -> async::task<void> {
        lua->pushinteger(object->hit());
        co_return;
    });
}