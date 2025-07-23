#include <fb/game/character.h>
#include <fb/game/map.h>
#include <fb/game/mob.h>
#include <fb/model/model.h>
#include <fb/game/server.h>

int fb::model::mob::builtin_speed(lua_State* L)
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

int fb::model::mob::builtin_size(lua_State* L)
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

int fb::model::mob::builtin_damage(lua_State* L)
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

int fb::model::mob::builtin_drop(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto model  = lua->touserdata<fb::model::mob>(1);
    if (model == nullptr)
        return 0;

    lua->new_table();
    if (server->model.drop.contains(model->drop))
    {
        auto  i     = 0;
        auto  visit = std::unordered_set<uint32_t>();
        auto& drop  = server->model.drop[model->drop];
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
                lua->pushobject(server->model.item[params.id]);
                lua_settable(L, -3);
                i++;
                break;
            }
        }
    }
    return 1;
}

fb::model::mob* fb::model::__mob::name2mob(const std::string& name) const
{
    for (auto& [k, v] : *this)
    {
        if (v.name == name)
            return &v;
    }
    return nullptr;
}