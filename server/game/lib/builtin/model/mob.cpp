#include <fb/game/server.h>
#include <fb/model/model.h>
#include <fb/game/builtin/model.h>

using namespace fb::game;
using table = fb::model::table;

// clang-format off
IMPLEMENT_LUA_EXTENSION(fb::model::mob, "fb.model.mob")
{"speed",               builtin::model::mob::builtin_speed},
{"size",                builtin::model::mob::builtin_size},
{"damage",              builtin::model::mob::builtin_damage},
{"drop",                builtin::model::mob::builtin_drop},
{"exp",                 builtin::model::mob::builtin_exp},
{"script",              builtin::model::mob::builtin_script},
{"on_spell_hit",        builtin::model::mob::builtin_on_spell_hit},
{"appearance",          builtin::model::mob::builtin_appearance},
END_LUA_EXTENSION; // clang-format on

int builtin::model::mob::builtin_speed(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto mob = lua->touserdata<fb::model::mob>(1);
    if (mob == nullptr)
        return 0;

    lua->pushinteger(mob->speed.total_milliseconds());
    return 1;
}

int builtin::model::mob::builtin_size(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto mob = lua->touserdata<fb::model::mob>(1);
    if (mob == nullptr)
        return 0;

    lua->pushinteger(static_cast<uint8_t>(mob->size));
    return 1;
}

int builtin::model::mob::builtin_damage(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto mob = lua->touserdata<fb::model::mob>(1);
    if (mob == nullptr)
        return 0;

    lua->pushinteger(mob->damage.min);
    lua->pushinteger(mob->damage.max);
    return 2;
}

int builtin::model::mob::builtin_drop(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto model  = lua->touserdata<fb::model::mob>(1);
    if (model == nullptr)
        return 0;

    lua->new_table();
    if (table::drop.contains(model->drop))
    {
        auto  i     = 0;
        auto  visit = std::unordered_set<uint32_t>();
        auto& drop  = table::drop[model->drop];
        for (auto& dsl : drop.dsl)
        {
            switch (dsl.header)
            {
            case fb::model::enum_value::DSL::item:
                auto params = fb::model::dsl::item(dsl.params);
                if (visit.contains(params.id))
                    continue;

                visit.insert(params.id);
                lua->pushinteger(i + 1);
                lua->pushobject(table::item[params.id]);
                lua_settable(L, -3);
                i++;
                break;
            }
        }
    }
    return 1;
}

int builtin::model::mob::builtin_exp(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto mob = lua->touserdata<fb::model::mob>(1);
    if (mob == nullptr)
        return 0;

    lua->pushinteger(static_cast<lua_Integer>(mob->exp));
    return 1;
}

int builtin::model::mob::builtin_script(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto mob = lua->touserdata<fb::model::mob>(1);
    if (mob == nullptr)
        return 0;

    if (mob->script.empty())
        lua->pushnil();
    else
        lua->pushstring(mob->script);
    return 1;
}

int builtin::model::mob::builtin_on_spell_hit(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto mob = lua->touserdata<fb::model::mob>(1);
    if (mob == nullptr)
        return 0;

    if (mob->on_spell_hit.empty())
        lua->pushnil();
    else
        lua->pushstring(mob->on_spell_hit);

    return 1;
}

int builtin::model::mob::builtin_appearance(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto mob = lua->touserdata<fb::model::mob>(1);
    if (mob == nullptr)
        return 0;

    if (!mob->appearance.has_value() || !table::appearance.contains(mob->appearance.value()))
    {
        lua->new_table();
        lua->pushstring("disguise");
        lua->pushinteger(static_cast<lua_Integer>(mob->look));
        lua->settable(-3);
        lua->pushstring("hair_color");
        lua->pushinteger(static_cast<lua_Integer>(mob->color));
        lua->settable(-3);
        return 1;
    }

    auto const& p = table::appearance[mob->appearance.value()];
    lua->new_table();
    lua->pushstring("gender");
    lua->pushinteger(static_cast<lua_Integer>(static_cast<uint8_t>(p.gender)));
    lua->settable(-3);
    lua->pushstring("state");
    lua->pushinteger(static_cast<lua_Integer>(static_cast<uint8_t>(p.state)));
    lua->settable(-3);
    lua->pushstring("hair");
    lua->pushinteger(static_cast<lua_Integer>(p.hair));
    lua->settable(-3);
    lua->pushstring("hair_color");
    lua->pushinteger(static_cast<lua_Integer>(p.hair_color));
    lua->settable(-3);
    if (p.weapon.has_value())
    {
        lua->pushstring("weapon");
        lua->pushinteger(static_cast<lua_Integer>(p.weapon.value()));
        lua->settable(-3);
    }
    if (p.weapon_color.has_value())
    {
        lua->pushstring("weapon_color");
        lua->pushinteger(static_cast<lua_Integer>(p.weapon_color.value()));
        lua->settable(-3);
    }
    if (p.armor.has_value())
    {
        lua->pushstring("armor");
        lua->pushinteger(static_cast<lua_Integer>(p.armor.value()));
        lua->settable(-3);
    }
    if (p.armor_color.has_value())
    {
        lua->pushstring("armor_color");
        lua->pushinteger(static_cast<lua_Integer>(p.armor_color.value()));
        lua->settable(-3);
    }
    if (p.shield.has_value())
    {
        lua->pushstring("shield");
        lua->pushinteger(static_cast<lua_Integer>(p.shield.value()));
        lua->settable(-3);
    }
    if (p.shield_color.has_value())
    {
        lua->pushstring("shield_color");
        lua->pushinteger(static_cast<lua_Integer>(p.shield_color.value()));
        lua->settable(-3);
    }
    if (p.disguise.has_value())
    {
        lua->pushstring("disguise");
        lua->pushinteger(static_cast<lua_Integer>(p.disguise.value()));
        lua->settable(-3);
    }
    return 1;
}