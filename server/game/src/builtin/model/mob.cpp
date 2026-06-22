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
    auto model = lua->touserdata<fb::model::mob>(1);
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
