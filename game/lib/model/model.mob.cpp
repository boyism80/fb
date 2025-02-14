#include <fb/game/character.h>
#include <fb/game/map.h>
#include <fb/game/mob.h>
#include <fb/model/model.h>
#include <fb/game/context.h>

int fb::model::mob::builtin_speed(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto mob = thread->touserdata<fb::model::mob>(1);
    if (mob == nullptr)
        return 0;

    thread->pushinteger(mob->speed.total_milliseconds());
    return 1;
}

int fb::model::mob::builtin_size(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto mob = thread->touserdata<fb::model::mob>(1);
    if (mob == nullptr)
        return 0;

    thread->pushinteger(static_cast<uint8_t>(mob->size));
    return 1;
}

int fb::model::mob::builtin_damage(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto mob = thread->touserdata<fb::model::mob>(1);
    if (mob == nullptr)
        return 0;

    thread->pushinteger(mob->damage.min);
    thread->pushinteger(mob->damage.max);
    return 2;
}

int fb::model::mob::builtin_drop(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto model   = thread->touserdata<fb::model::mob>(1);
    if (model == nullptr)
        return 0;

    thread->new_table();
    if (context->model.drop.contains(model->drop))
    {
        auto  i     = 0;
        auto  visit = std::unordered_set<uint32_t>();
        auto& drop  = context->model.drop[model->drop];
        for (auto& dsl : drop.dsl)
        {
            switch (dsl.header)
            {
            case fb::model::enum_value::DSL::item:
                auto params = fb::model::dsl::item(dsl.params);
                if (visit.contains(params.id))
                    continue;

                visit.insert(params.id);
                thread->pushinteger(i + 1);
                thread->pushobject(context->model.item[params.id]);
                lua_settable(lua, -3);
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