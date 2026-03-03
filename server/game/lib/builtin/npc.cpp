#include <fb/game/npc.h>
#include <fb/game/builtin/npc.h>
#include <fb/model/model.h>
#include <fb/lua.h>

using namespace fb::game;
using table = fb::model::table;

// clang-format off
IMPLEMENT_LUA_EXTENSION(fb::game::npc, "fb.game.npc")
{"appearance",         builtin::npc::builtin_appearance},
END_LUA_EXTENSION; // clang-format on

int builtin::npc::builtin_appearance(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto npc = lua->touserdata<fb::game::npc>(1);
    if (npc == nullptr)
        return 0;

    auto const& model = npc->based<fb::model::npc>();

    if (!model.appearance.has_value() || !table::appearance.contains(model.appearance.value()))
    {
        lua->new_table();
        lua->pushstring("disguise");
        lua->pushinteger(static_cast<lua_Integer>(model.look));
        lua->settable(-3);
        lua->pushstring("hair_color");
        lua->pushinteger(static_cast<lua_Integer>(model.color));
        lua->settable(-3);
        return 1;
    }

    auto const& p = table::appearance[model.appearance.value()];
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