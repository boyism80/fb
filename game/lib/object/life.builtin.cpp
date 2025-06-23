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
    if (obj == nullptr)
        return 0;

    if (obj->is(OBJECT_TYPE::CHARACTER) == false)
        return 0;

    auto ch      = std::static_pointer_cast<fb::game::character>(obj);
    auto message = lua->tostring(2);
    auto type    = lua->toenum(3, MESSAGE_TYPE::STATE);

    auto weak = ch->weak_from_this();
    return lua->ensure_yield(*ctx, weak, [=]() {
        ch->message(message, type);
        return lua->ensure_resume(*ctx, weak, [=]() {
            return 0;
        });
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
    if (obj == nullptr)
        return 0;

    if (argc == 1)
    {
        auto weak = obj->weak_from_this();
        return lua->ensure_yield(*ctx, weak, [=]() {
            auto hp_value = obj->hp();

            return lua->ensure_resume(*ctx, weak, [=]() {
                lua->pushinteger(hp_value);
                return 1;
            });
        });
    }
    else
    {
        auto value = (uint32_t)lua->tointeger(2);
        auto weak  = obj->weak_from_this();
        return lua->ensure_yield(*ctx, weak, [=]() {
            obj->hp(value);

            return lua->ensure_resume(*ctx, weak, [=]() {
                return 0;
            });
        });
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
    if (obj == nullptr)
        return 0;

    if (argc == 1)
    {
        auto weak = obj->weak_from_this();
        return lua->ensure_yield(*ctx, weak, [=]() {
            auto mp_value = obj->mp();

            return lua->ensure_resume(*ctx, weak, [=]() {
                lua->pushinteger(mp_value);
                return 1;
            });
        });
    }
    else
    {
        auto value = (uint32_t)lua->tointeger(2);
        auto weak  = obj->weak_from_this();
        return lua->ensure_yield(*ctx, weak, [=]() {
            obj->mp(value);

            return lua->ensure_resume(*ctx, weak, [=]() {
                return 0;
            });
        });
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
    if (obj == nullptr)
        return 0;

    auto value = (uint32_t)lua->tointeger(2);
    auto weak  = obj->weak_from_this();
    return lua->ensure_yield(*ctx, weak, [=]() {
        obj->heal(value);
        return lua->ensure_resume(*ctx, weak, [=]() {
            return 0;
        });
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
    if (obj == nullptr)
        return 0;

    auto value    = (uint32_t)lua->tointeger(2);
    auto from     = lua->touserdata<fb::game::life>(3);
    auto critical = lua->toboolean(4, false);
    auto weak     = obj->weak_from_this();
    return lua->ensure_yield(*ctx, weak, [=]() {
        obj->damage(value, from, critical);
        return lua->ensure_resume(*ctx, weak, [=]() {
            return 0;
        });
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
    if (obj == nullptr)
        return 0;

    auto value = (uint32_t)lua->tointeger(2);
    auto weak  = obj->weak_from_this();
    return lua->ensure_yield(*ctx, weak, [=]() {
        obj->mp_up(value);
        return lua->ensure_resume(*ctx, weak, [=]() {
            return 0;
        });
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
    if (obj == nullptr)
        return 0;

    auto value = (uint32_t)lua->tointeger(2);
    auto weak  = obj->weak_from_this();
    return lua->ensure_yield(*ctx, weak, [=]() {
        obj->mp_down(value);
        return lua->ensure_resume(*ctx, weak, [=]() {
            return 0;
        });
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
    if (obj == nullptr)
        return 0;

    auto action   = lua->tointeger(2);
    auto duration = lua->tointeger(3, static_cast<int>(DURATION::SPELL));
    auto sound    = (uint8_t)lua->tointeger(4, (uint8_t)0x00);
    auto weak     = obj->weak_from_this();
    return lua->ensure_yield(*ctx, weak, [=]() {
        obj->action(ACTION(action), DURATION(duration), sound);
        return lua->ensure_resume(*ctx, weak, [=]() {
            return 0;
        });
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
    if (obj == nullptr)
        return 0;

    if (lua->is_number(2))
    {
        auto index = (int)lua->tointeger(2);
        auto weak  = obj->weak_from_this();
        return lua->ensure_yield(*ctx, weak, [=]() {
            auto spell = obj->spells[index];

            return lua->ensure_resume(*ctx, weak, [=]() {
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
        return lua->ensure_yield(*ctx, weak, [=]() {
            auto spell = obj->spells.find(name);

            return lua->ensure_resume(*ctx, weak, [=]() {
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
        return lua->ensure_yield(*ctx, weak, [=]() {
            auto spell = obj->spells.find(*model);

            return lua->ensure_resume(*ctx, weak, [=]() {
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

int life::builtin::builtin_spells(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto obj  = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    auto weak = obj->weak_from_this();
    return lua->ensure_yield(*ctx, weak, [=]() {
        auto buffer = std::vector<std::shared_ptr<fb::game::spell>>();
        for (int i = 0; i < CONTAINER_CAPACITY; i++)
        {
            auto spell = obj->spells[i];
            if (spell != nullptr)
                buffer.push_back(spell);
        }

        return lua->ensure_resume(*ctx, weak, [=]() {
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

int life::builtin::builtin_cast(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx    = lua->env<fb::game::context>("context");
    auto argc   = lua->argc();
    auto offset = 1;
    auto obj    = lua->touserdata<fb::game::life>(offset++);
    if (obj == nullptr)
        return 0;

    std::shared_ptr<fb::game::life> you;
    if (argc == 2)
        you = std::static_pointer_cast<fb::game::life>(obj);
    else if (lua->is_userdata<fb::game::life>(offset))
        you = std::static_pointer_cast<fb::game::life>(lua->touserdata<fb::game::life>(offset++));
    else
        you = nullptr;

    if (you == nullptr)
        return 0;

    auto map = obj->map();
    if (map == nullptr || map->objects.contains(you) == false)
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
    if (obj == nullptr)
        return 0;

    if (argc == 1)
    {
        auto weak = obj->weak_from_this();
        return lua->ensure_yield(*ctx, weak, [=]() {
            auto cc_value = static_cast<uint32_t>(obj->crowd_control());

            return lua->ensure_resume(*ctx, weak, [=]() {
                lua->pushinteger(cc_value);
                return 1;
            });
        });
    }
    else
    {
        auto cc   = static_cast<CROWD_CONTROL>(lua->tointeger(2));
        auto weak = obj->weak_from_this();
        return lua->ensure_yield(*ctx, weak, [=]() {
            obj->crowd_control(cc);

            return lua->ensure_resume(*ctx, weak, [=]() {
                return 0;
            });
        });
    }
}

int life::builtin::builtin_add_cc(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto obj  = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    if (argc == 1)
    {
        auto weak = obj->weak_from_this();
        return lua->ensure_yield(*ctx, weak, [=]() {
            auto cc_value = static_cast<uint32_t>(obj->crowd_control());

            return lua->ensure_resume(*ctx, weak, [=]() {
                lua->pushinteger(cc_value);
                return 1;
            });
        });
    }
    else
    {
        auto cc   = static_cast<CROWD_CONTROL>(lua->tointeger(2));
        auto weak = obj->weak_from_this();
        return lua->ensure_yield(*ctx, weak, [=]() {
            obj->add_cc(cc);

            return lua->ensure_resume(*ctx, weak, [=]() {
                return 0;
            });
        });
    }
}

int life::builtin::builtin_remove_cc(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto obj  = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    auto cc   = static_cast<CROWD_CONTROL>(lua->tointeger(2));
    auto weak = obj->weak_from_this();
    return lua->ensure_yield(*ctx, weak, [=]() {
        obj->remove_cc(cc);
        return lua->ensure_resume(*ctx, weak, [=]() {
            return 0;
        });
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
    if (obj == nullptr)
        return 0;

    auto duration = lua->toenum(2, DURATION::ATTACK);
    auto weak     = obj->weak_from_this();
    return lua->ensure_yield(*ctx, weak, [=]() {
        obj->attack(duration);
        return lua->ensure_resume(*ctx, weak, [=]() {
            return 0;
        });
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
    if (obj == nullptr)
        return 0;

    if (argc == 1)
    {
        auto weak = obj->weak_from_this();
        return lua->ensure_yield(*ctx, weak, [=]() {
            auto damage_rate_value = obj->damage_rate();

            return lua->ensure_resume(*ctx, weak, [=]() {
                lua->pushinteger(damage_rate_value);
                return 1;
            });
        });
    }
    else
    {
        auto value = lua->tointeger(2);
        auto weak  = obj->weak_from_this();
        return lua->ensure_yield(*ctx, weak, [=]() {
            obj->damage_rate(value);

            return lua->ensure_resume(*ctx, weak, [=]() {
                return 0;
            });
        });
    }
}

int life::builtin::builtin_damage_derate(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto obj  = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    if (argc == 1)
    {
        auto weak = obj->weak_from_this();
        return lua->ensure_yield(*ctx, weak, [=]() {
            auto damage_derate_value = obj->damage_derate();

            return lua->ensure_resume(*ctx, weak, [=]() {
                lua->pushinteger(damage_derate_value);
                return 1;
            });
        });
    }
    else
    {
        auto value = lua->tointeger(2);
        auto weak  = obj->weak_from_this();
        return lua->ensure_yield(*ctx, weak, [=]() {
            obj->damage_derate(value);

            return lua->ensure_resume(*ctx, weak, [=]() {
                return 0;
            });
        });
    }
}

int life::builtin::builtin_skill_damage_rate(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto obj  = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    if (argc == 1)
    {
        auto weak = obj->weak_from_this();
        return lua->ensure_yield(*ctx, weak, [=]() {
            auto skill_damage_rate_value = obj->skill_damage_rate();

            return lua->ensure_resume(*ctx, weak, [=]() {
                lua->pushinteger(skill_damage_rate_value);
                return 1;
            });
        });
    }
    else
    {
        auto value = lua->tointeger(2);
        auto weak  = obj->weak_from_this();
        return lua->ensure_yield(*ctx, weak, [=]() {
            obj->skill_damage_rate(value);

            return lua->ensure_resume(*ctx, weak, [=]() {
                return 0;
            });
        });
    }
}

int life::builtin::builtin_paralysis(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto obj  = lua->touserdata<life>(1);
    if (obj == nullptr)
        return 0;

    if (argc == 1)
    {
        auto weak = obj->weak_from_this();
        return lua->ensure_yield(*ctx, weak, [=]() {
            auto paralysis_value = obj->paralysis();

            return lua->ensure_resume(*ctx, weak, [=]() {
                lua->pushboolean(paralysis_value);
                return 1;
            });
        });
    }
    else
    {
        auto value = lua->toboolean(2);
        auto weak  = obj->weak_from_this();
        return lua->ensure_yield(*ctx, weak, [=]() {
            obj->paralysis(value);

            return lua->ensure_resume(*ctx, weak, [=]() {
                return 0;
            });
        });
    }
}

int life::builtin::builtin_invincible(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto obj  = lua->touserdata<life>(1);
    if (obj == nullptr)
        return 0;

    if (argc == 1)
    {
        auto weak = obj->weak_from_this();
        return lua->ensure_yield(*ctx, weak, [=]() {
            auto invincible_value = obj->invincible();

            return lua->ensure_resume(*ctx, weak, [=]() {
                lua->pushboolean(invincible_value);
                return 1;
            });
        });
    }
    else
    {
        auto value = lua->toboolean(2);
        auto weak  = obj->weak_from_this();
        return lua->ensure_yield(*ctx, weak, [=]() {
            obj->invincible(value);

            return lua->ensure_resume(*ctx, weak, [=]() {
                return 0;
            });
        });
    }
}

int life::builtin::builtin_cover(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto obj  = lua->touserdata<life>(1);
    if (obj == nullptr)
        return 0;

    if (argc == 1)
    {
        auto weak = obj->weak_from_this();
        return lua->ensure_yield(*ctx, weak, [=]() {
            auto cover_value = obj->cover();

            return lua->ensure_resume(*ctx, weak, [=]() {
                lua->pushboolean(cover_value);
                return 1;
            });
        });
    }
    else
    {
        auto value = lua->toboolean(2);
        auto weak  = obj->weak_from_this();
        return lua->ensure_yield(*ctx, weak, [=]() {
            obj->cover(value);

            return lua->ensure_resume(*ctx, weak, [=]() {
                return 0;
            });
        });
    }
}

int life::builtin::builtin_base_hp(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto argc = lua->argc();
    auto ctx  = lua->env<fb::game::context>("context");
    auto obj  = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    auto weak = obj->weak_from_this();
    return lua->ensure_yield(*ctx, weak, [=]() {
        auto value = obj->base_hp();
        return lua->ensure_resume(*ctx, weak, [=]() {
            lua->pushinteger(value);
            return 1;
        });
    });
}

int life::builtin::builtin_buff_hp(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx  = lua->env<fb::game::context>("context");
    auto argc = lua->argc();
    auto obj  = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    if (argc == 1)
    {
        auto weak = obj->weak_from_this();
        return lua->ensure_yield(*ctx, weak, [=]() {
            auto buff_hp_value = obj->buff_hp();

            return lua->ensure_resume(*ctx, weak, [=]() {
                lua->pushinteger(buff_hp_value);
                return 1;
            });
        });
    }
    else
    {
        auto value = lua->tointeger(2);
        auto weak  = obj->weak_from_this();
        return lua->ensure_yield(*ctx, weak, [=]() {
            obj->buff_hp(value);

            return lua->ensure_resume(*ctx, weak, [=]() {
                return 0;
            });
        });
    }
}

int life::builtin::builtin_maxhp(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx = lua->env<fb::game::context>("context");
    auto obj = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    auto weak = obj->weak_from_this();
    return lua->ensure_yield(*ctx, weak, [=]() {
        auto maxhp_value = obj->maxhp();

        return lua->ensure_resume(*ctx, weak, [=]() {
            lua->pushinteger(maxhp_value);
            return 1;
        });
    });
}

int life::builtin::builtin_base_mp(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx = lua->env<fb::game::context>("context");
    auto obj = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    auto weak = obj->weak_from_this();
    return lua->ensure_yield(*ctx, weak, [=]() {
        auto base_mp_value = obj->base_mp();

        return lua->ensure_resume(*ctx, weak, [=]() {
            lua->pushinteger(base_mp_value);
            return 1;
        });
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
    if (obj == nullptr)
        return 0;

    if (argc == 1)
    {
        auto weak = obj->weak_from_this();
        return lua->ensure_yield(*ctx, weak, [=]() {
            auto buff_mp_value = obj->buff_mp();

            return lua->ensure_resume(*ctx, weak, [=]() {
                lua->pushinteger(buff_mp_value);
                return 1;
            });
        });
    }
    else
    {
        auto value = lua->tointeger(2);
        auto weak  = obj->weak_from_this();
        return lua->ensure_yield(*ctx, weak, [=]() {
            obj->buff_mp(value);

            return lua->ensure_resume(*ctx, weak, [=]() {
                return 0;
            });
        });
    }
}

int life::builtin::builtin_maxmp(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx = lua->env<fb::game::context>("context");
    auto obj = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    auto weak = obj->weak_from_this();
    return lua->ensure_yield(*ctx, weak, [=]() {
        auto maxmp_value = obj->maxmp();

        return lua->ensure_resume(*ctx, weak, [=]() {
            lua->pushinteger(maxmp_value);
            return 1;
        });
    });
}

int life::builtin::builtin_base_str(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx = lua->env<fb::game::context>("context");
    auto obj = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    auto weak = obj->weak_from_this();
    return lua->ensure_yield(*ctx, weak, [=]() {
        auto base_str_value = obj->base_str();

        return lua->ensure_resume(*ctx, weak, [=]() {
            lua->pushinteger(base_str_value);
            return 1;
        });
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
    if (obj == nullptr)
        return 0;

    if (argc == 1)
    {
        auto weak = obj->weak_from_this();
        return lua->ensure_yield(*ctx, weak, [=]() {
            auto buff_str_value = obj->buff_str();

            return lua->ensure_resume(*ctx, weak, [=]() {
                lua->pushinteger(buff_str_value);
                return 1;
            });
        });
    }
    else
    {
        auto value = lua->tointeger(2);
        auto weak  = obj->weak_from_this();
        return lua->ensure_yield(*ctx, weak, [=]() {
            obj->buff_str(value);

            return lua->ensure_resume(*ctx, weak, [=]() {
                return 0;
            });
        });
    }
}

int life::builtin::builtin_str(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx = lua->env<fb::game::context>("context");
    auto obj = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    auto weak = obj->weak_from_this();
    return lua->ensure_yield(*ctx, weak, [=]() {
        auto str_value = obj->str();

        return lua->ensure_resume(*ctx, weak, [=]() {
            lua->pushinteger(str_value);
            return 1;
        });
    });
}

int life::builtin::builtin_base_dex(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx = lua->env<fb::game::context>("context");
    auto obj = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    auto weak = obj->weak_from_this();
    return lua->ensure_yield(*ctx, weak, [=]() {
        auto base_dex_value = obj->base_dex();

        return lua->ensure_resume(*ctx, weak, [=]() {
            lua->pushinteger(base_dex_value);
            return 1;
        });
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
    if (obj == nullptr)
        return 0;

    if (argc == 1)
    {
        auto weak = obj->weak_from_this();
        return lua->ensure_yield(*ctx, weak, [=]() {
            auto buff_dex_value = obj->buff_dex();

            return lua->ensure_resume(*ctx, weak, [=]() {
                lua->pushinteger(buff_dex_value);
                return 1;
            });
        });
    }
    else
    {
        auto value = lua->tointeger(2);
        auto weak  = obj->weak_from_this();
        return lua->ensure_yield(*ctx, weak, [=]() {
            obj->buff_dex(value);

            return lua->ensure_resume(*ctx, weak, [=]() {
                return 0;
            });
        });
    }
}

int life::builtin::builtin_dex(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx = lua->env<fb::game::context>("context");
    auto obj = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    auto weak = obj->weak_from_this();
    return lua->ensure_yield(*ctx, weak, [=]() {
        auto dex_value = obj->dex();

        return lua->ensure_resume(*ctx, weak, [=]() {
            lua->pushinteger(dex_value);
            return 1;
        });
    });
}

int life::builtin::builtin_base_int(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx = lua->env<fb::game::context>("context");
    auto obj = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    auto weak = obj->weak_from_this();
    return lua->ensure_yield(*ctx, weak, [=]() {
        auto base_int_value = obj->base_int();

        return lua->ensure_resume(*ctx, weak, [=]() {
            lua->pushinteger(base_int_value);
            return 1;
        });
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
    if (obj == nullptr)
        return 0;

    if (argc == 1)
    {
        auto weak = obj->weak_from_this();
        return lua->ensure_yield(*ctx, weak, [=]() {
            auto buff_int_value = obj->buff_int();

            return lua->ensure_resume(*ctx, weak, [=]() {
                lua->pushinteger(buff_int_value);
                return 1;
            });
        });
    }
    else
    {
        auto value = lua->tointeger(2);
        auto weak  = obj->weak_from_this();
        return lua->ensure_yield(*ctx, weak, [=]() {
            obj->buff_int(value);

            return lua->ensure_resume(*ctx, weak, [=]() {
                return 0;
            });
        });
    }
}

int life::builtin::builtin_intelligence(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx = lua->env<fb::game::context>("context");
    auto obj = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    auto weak = obj->weak_from_this();
    return lua->ensure_yield(*ctx, weak, [=]() {
        auto intelligence_value = obj->intelligence();

        return lua->ensure_resume(*ctx, weak, [=]() {
            lua->pushinteger(intelligence_value);
            return 1;
        });
    });
}

int life::builtin::builtin_base_phydef(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx = lua->env<fb::game::context>("context");
    auto obj = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    auto weak = obj->weak_from_this();
    return lua->ensure_yield(*ctx, weak, [=]() {
        auto base_phydef_value = obj->base_phydef();

        return lua->ensure_resume(*ctx, weak, [=]() {
            lua->pushinteger(base_phydef_value);
            return 1;
        });
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
    if (obj == nullptr)
        return 0;

    if (argc == 1)
    {
        auto weak = obj->weak_from_this();
        return lua->ensure_yield(*ctx, weak, [=]() {
            auto buff_phydef_value = obj->buff_phydef();

            return lua->ensure_resume(*ctx, weak, [=]() {
                lua->pushinteger(buff_phydef_value);
                return 1;
            });
        });
    }
    else
    {
        auto value = lua->tointeger(2);
        auto weak  = obj->weak_from_this();
        return lua->ensure_yield(*ctx, weak, [=]() {
            obj->buff_phydef(value);

            return lua->ensure_resume(*ctx, weak, [=]() {
                return 0;
            });
        });
    }
}

int life::builtin::builtin_phydef(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx = lua->env<fb::game::context>("context");
    auto obj = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    auto weak = obj->weak_from_this();
    return lua->ensure_yield(*ctx, weak, [=]() {
        auto phydef_value = obj->phydef();

        return lua->ensure_resume(*ctx, weak, [=]() {
            lua->pushinteger(phydef_value);
            return 1;
        });
    });
}

int life::builtin::builtin_base_magdef(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx = lua->env<fb::game::context>("context");
    auto obj = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    auto weak = obj->weak_from_this();
    return lua->ensure_yield(*ctx, weak, [=]() {
        auto base_magdef_value = obj->base_magdef();

        return lua->ensure_resume(*ctx, weak, [=]() {
            lua->pushinteger(base_magdef_value);
            return 1;
        });
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
    if (obj == nullptr)
        return 0;

    if (argc == 1)
    {
        auto weak = obj->weak_from_this();
        return lua->ensure_yield(*ctx, weak, [=]() {
            auto buff_magdef_value = obj->buff_magdef();

            return lua->ensure_resume(*ctx, weak, [=]() {
                lua->pushinteger(buff_magdef_value);
                return 1;
            });
        });
    }
    else
    {
        auto value = lua->tointeger(2);
        auto weak  = obj->weak_from_this();
        return lua->ensure_yield(*ctx, weak, [=]() {
            obj->buff_magdef(value);

            return lua->ensure_resume(*ctx, weak, [=]() {
                return 0;
            });
        });
    }
}

int life::builtin::builtin_magdef(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx = lua->env<fb::game::context>("context");
    auto obj = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    auto weak = obj->weak_from_this();
    return lua->ensure_yield(*ctx, weak, [=]() {
        auto magdef_value = obj->magdef();

        return lua->ensure_resume(*ctx, weak, [=]() {
            lua->pushinteger(magdef_value);
            return 1;
        });
    });
}

int life::builtin::builtin_base_dam(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx = lua->env<fb::game::context>("context");
    auto obj = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    auto weak = obj->weak_from_this();
    return lua->ensure_yield(*ctx, weak, [=]() {
        auto base_dam_value = obj->base_dam();

        return lua->ensure_resume(*ctx, weak, [=]() {
            lua->pushinteger(base_dam_value);
            return 1;
        });
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
    if (obj == nullptr)
        return 0;

    if (argc == 1)
    {
        auto weak = obj->weak_from_this();
        return lua->ensure_yield(*ctx, weak, [=]() {
            auto buff_dam_value = obj->buff_dam();

            return lua->ensure_resume(*ctx, weak, [=]() {
                lua->pushinteger(buff_dam_value);
                return 1;
            });
        });
    }
    else
    {
        auto value = lua->tointeger(2);
        auto weak  = obj->weak_from_this();
        return lua->ensure_yield(*ctx, weak, [=]() {
            obj->buff_dam(value);

            return lua->ensure_resume(*ctx, weak, [=]() {
                return 0;
            });
        });
    }
}

int life::builtin::builtin_dam(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx = lua->env<fb::game::context>("context");
    auto obj = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    auto weak = obj->weak_from_this();
    return lua->ensure_yield(*ctx, weak, [=]() {
        auto dam_value = obj->dam();

        return lua->ensure_resume(*ctx, weak, [=]() {
            lua->pushinteger(dam_value);
            return 1;
        });
    });
}

int life::builtin::builtin_base_hit(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx = lua->env<fb::game::context>("context");
    auto obj = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    auto weak = obj->weak_from_this();
    return lua->ensure_yield(*ctx, weak, [=]() {
        auto base_hit_value = obj->base_hit();

        return lua->ensure_resume(*ctx, weak, [=]() {
            lua->pushinteger(base_hit_value);
            return 1;
        });
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
    if (obj == nullptr)
        return 0;

    if (argc == 1)
    {
        auto weak = obj->weak_from_this();
        return lua->ensure_yield(*ctx, weak, [=]() {
            auto buff_hit_value = obj->buff_hit();

            return lua->ensure_resume(*ctx, weak, [=]() {
                lua->pushinteger(buff_hit_value);
                return 1;
            });
        });
    }
    else
    {
        auto value = lua->tointeger(2);
        auto weak  = obj->weak_from_this();
        return lua->ensure_yield(*ctx, weak, [=]() {
            obj->buff_hit(value);

            return lua->ensure_resume(*ctx, weak, [=]() {
                return 0;
            });
        });
    }
}

int life::builtin::builtin_hit(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ctx = lua->env<fb::game::context>("context");
    auto obj = lua->touserdata<fb::game::life>(1);
    if (obj == nullptr)
        return 0;

    auto weak = obj->weak_from_this();
    return lua->ensure_yield(*ctx, weak, [=]() {
        auto hit_value = obj->hit();

        return lua->ensure_resume(*ctx, weak, [=]() {
            lua->pushinteger(hit_value);
            return 1;
        });
    });
}