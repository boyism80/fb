#include <fb/game/context.h>
#include <fb/game/life.h>
#include <fb/game/map.h>

// clang-format off
IMPLEMENT_LUA_EXTENSION(fb::game::life, "fb.game.life")
{"__eq",                fb::game::object::builtin_eq},
{"hp",                  fb::game::life::builtin_hp},
{"mp",                  fb::game::life::builtin_mp},
{"heal",                fb::game::life::builtin_heal},
{"damage",              fb::game::life::builtin_damage},
{"mp_up",               fb::game::life::builtin_mp_up},
{"mp_down",             fb::game::life::builtin_mp_down},
{"base_hp",             fb::game::life::builtin_base_hp},
{"base_mp",             fb::game::life::builtin_base_mp},
{"action",              fb::game::life::builtin_action},
{"spell",               fb::game::life::builtin_spell},
{"cast",                fb::game::life::builtin_cast},
{"cc",                  fb::game::life::builtin_cc},
{"add_cc",              fb::game::life::builtin_add_cc},
{"remove_cc",           fb::game::life::builtin_remove_cc},
{"attack",              fb::game::life::builtin_attack},
{"damage_rate",         fb::game::life::builtin_damage_rate},
{"skill_damage_rate",   fb::game::life::builtin_skill_damage_rate},
END_LUA_EXTENSION; // clang-format on

int fb::game::life::builtin_hp(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto argc    = thread->argc();
    auto object  = thread->touserdata<fb::game::life>(1);
    if (object == nullptr)
        return 0;

    if (argc == 1)
    {
        thread->pushinteger(object->hp());
        return 1;
    }
    else
    {
        auto value = (uint32_t)thread->tointeger(2);
        object->hp(value);
        return 0;
    }
}

int fb::game::life::builtin_mp(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto argc    = thread->argc();
    auto object  = thread->touserdata<fb::game::life>(1);
    if (object == nullptr)
        return 0;

    if (argc == 1)
    {
        thread->pushinteger(object->mp());
        return 1;
    }
    else
    {
        auto value = (uint32_t)thread->tointeger(2);
        object->mp(value);
        return 0;
    }
}

int fb::game::life::builtin_base_hp(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto object  = thread->touserdata<fb::game::life>(1);
    if (object == nullptr)
        return 0;

    auto& model = object->based<fb::model::life>();

    thread->pushinteger(model.hp);
    return 1;
}

int fb::game::life::builtin_base_mp(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto object  = thread->touserdata<fb::game::life>(1);
    if (object == nullptr)
        return 0;

    auto& model = object->based<fb::model::life>();

    thread->pushinteger(model.mp);
    return 1;
}

int fb::game::life::builtin_heal(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto argc    = thread->argc();
    auto obj     = thread->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    auto value = (uint32_t)thread->tointeger(2);
    if (obj->matched_thread())
    {
        obj->heal(value);
        return 0;
    }
    else
    {
        context->threads.enqueue(*obj, [=](auto&) -> async::task<void> {
            obj->heal(value);
            co_return;
        });
        return thread->yield(0);
    }
}

int fb::game::life::builtin_damage(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto argc    = thread->argc();
    auto obj     = thread->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    auto value    = (uint32_t)thread->tointeger(2);
    auto from     = argc >= 3 ? thread->touserdata<fb::game::object>(3) : nullptr;
    auto critical = argc >= 4 ? thread->toboolean(4) : false;
    if (obj->matched_thread())
    {
        obj->damage(value, from, critical);
        return 0;
    }
    else
    {
        context->threads.enqueue(*obj, [=](auto&) -> async::task<void> {
            obj->damage(value, from, critical);
            co_return;
        });
        return thread->yield(0);
    }
}

int fb::game::life::builtin_mp_up(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto argc    = thread->argc();
    auto obj     = thread->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    auto value = (uint32_t)thread->tointeger(2);
    if (obj->matched_thread())
    {
        obj->mp_up(value);
        return 0;
    }
    else
    {
        context->threads.enqueue(*obj, [=](auto&) -> async::task<void> {
            obj->mp_up(value);
            co_return;
        });
        return thread->yield(0);
    }
}

int fb::game::life::builtin_mp_down(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto argc    = thread->argc();
    auto obj     = thread->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    auto value = (uint32_t)thread->tointeger(2);
    if (obj->matched_thread())
    {
        obj->mp_down(value);
        return 0;
    }
    else
    {
        context->threads.enqueue(*obj, [=](auto&) -> async::task<void> {
            obj->mp_down(value);
            co_return;
        });
        return thread->yield(0);
    }
}

int fb::game::life::builtin_action(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto argc    = thread->argc();
    auto obj     = thread->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    auto action   = thread->tointeger(2);
    auto duration = argc < 3 ? static_cast<int>(DURATION::SPELL) : thread->tointeger(3);
    auto sound    = argc < 4 ? (uint8_t)0x00 : (uint8_t)thread->tointeger(4);
    obj->action(ACTION(action), DURATION(duration), sound);
    return 0;
}

int fb::game::life::builtin_spell(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto argc    = thread->argc();
    auto life    = thread->touserdata<fb::game::life>(1);
    if (life == nullptr)
        return 0;

    auto index = (int)thread->tointeger(2);

    thread->pushobject(life->spells[index]);
    return 1;
}

int fb::game::life::builtin_cast(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto argc    = thread->argc();
    auto me      = thread->touserdata<fb::game::life>(1);
    if (me == nullptr)
        return 0;

    auto you = thread->touserdata<fb::game::life>(2);
    if (you == nullptr)
        you = nullptr;

    auto map = me->map();
    if (map == nullptr || map->objects.contains(*you) == false)
        return 0;

    auto name  = thread->tostring(3);
    auto spell = context->model.spell.name2spell(name);
    if (spell == nullptr)
        return 0;

    auto x = lua::new_context();
    if (x == nullptr)
        return 0;

    x->from(spell->cast.c_str()).func("on_cast").pushobject(me);

    if (you != nullptr)
        x->pushobject(you);
    else
        x->pushnil();

    x->pushobject(spell).resume(3);

    return 0;
}

int fb::game::life::builtin_cc(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto argc    = thread->argc();
    auto me      = thread->touserdata<fb::game::life>(1);
    if (me == nullptr)
        return 0;

    if (argc == 1)
    {
        thread->pushinteger(static_cast<uint32_t>(me->crowd_control()));
        return 1;
    }
    else
    {
        auto cc = static_cast<CROWD_CONTROL>(thread->tointeger(2));
        me->crowd_control(cc);
        return 0;
    }

    return 0;
}

int fb::game::life::builtin_add_cc(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto argc    = thread->argc();
    auto me      = thread->touserdata<fb::game::life>(1);
    if (me == nullptr)
        return 0;

    auto cc = static_cast<CROWD_CONTROL>(thread->tointeger(2));
    me->add_cc(cc);
    return 0;
}

int fb::game::life::builtin_remove_cc(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto argc    = thread->argc();
    auto me      = thread->touserdata<fb::game::life>(1);
    if (me == nullptr)
        return 0;

    auto cc = static_cast<CROWD_CONTROL>(thread->tointeger(2));
    me->remove_cc(cc);
    return 0;
}

int fb::game::life::builtin_attack(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto argc    = thread->argc();
    auto me      = thread->touserdata<fb::game::life>(1);
    if (me == nullptr)
        return 0;

    auto duration = argc >= 2 ? static_cast<DURATION>(thread->tointeger(2)) : DURATION::ATTACK;
    if (me->matched_thread())
    {
        me->attack(duration);
        return 0;
    }
    else
    {
        static auto static_func = [](fb::lua::context* thread, life* me, DURATION duration) {
            me->attack(duration);
            thread->resume(0);
        };
        context->threads.enqueue(*me, [=](auto&) -> async::task<void> {
            static_func(thread, me, duration);
            co_return;
        });
        return thread->yield(0);
    }
}

int fb::game::life::builtin_damage_rate(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto argc    = thread->argc();
    auto me      = thread->touserdata<fb::game::life>(1);
    if (me == nullptr)
        return 0;

    if (argc == 1)
    {
        if (me->matched_thread())
        {
            thread->pushinteger(me->damage_rate());
            return 1;
        }
        else
        {
            static auto static_func = [](fb::lua::context* thread, life* me) {
                thread->pushinteger(me->damage_rate());
                thread->resume(1);
            };
            context->threads.enqueue(*me, [=](auto&) -> async::task<void> {
                static_func(thread, me);
                co_return;
            });
            return thread->yield(1);
        }
    }
    else
    {
        auto value = thread->tointeger(2);
        if (me->matched_thread())
        {
            me->damage_rate(value);
            return 0;
        }
        else
        {
            static auto static_func = [](fb::lua::context* thread, life* me, uint16_t value) {
                me->damage_rate(value);
                thread->resume(0);
            };
            context->threads.enqueue(*me, [=](auto&) -> async::task<void> {
                static_func(thread, me, value);
                co_return;
            });
            return thread->yield(0);
        }
    }
}

int fb::game::life::builtin_skill_damage_rate(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto argc    = thread->argc();
    auto me      = thread->touserdata<fb::game::life>(1);
    if (me == nullptr)
        return 0;

    if (argc == 1)
    {
        if (me->matched_thread())
        {
            thread->pushinteger(me->skill_damage_rate());
            return 1;
        }
        else
        {
            static auto static_func = [](fb::lua::context* thread, life* me) {
                thread->pushinteger(me->skill_damage_rate());
                thread->resume(1);
            };
            context->threads.enqueue(*me, [=](auto&) -> async::task<void> {
                static_func(thread, me);
                co_return;
            });
            return thread->yield(1);
        }
    }
    else
    {
        auto value = thread->tointeger(2);
        if (me->matched_thread())
        {
            me->skill_damage_rate(value);
            return 0;
        }
        else
        {
            static auto static_func = [](fb::lua::context* thread, life* me, uint16_t value) {
                me->skill_damage_rate(value);
                thread->resume(0);
            };
            context->threads.enqueue(*me, [=](auto&) -> async::task<void> {
                static_func(thread, me, value);
                co_return;
            });
            return thread->yield(0);
        }
    }
}