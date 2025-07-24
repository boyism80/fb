#include <fb/model/model.h>

fb::model::spell* fb::model::__spell::name2spell(const std::string& name) const
{
    for (auto& [k, v] : *this)
    {
        if (v.name == name)
            return &v;
    }
    return nullptr;
}

int fb::model::spell::builtin_type(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto spell = lua->touserdata<fb::model::spell>(1);
    if (spell == nullptr)
        return 0;

    lua->pushinteger(spell->type);
    return 1;
}

int fb::model::spell::builtin_name(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto spell = lua->touserdata<fb::model::spell>(1);
    if (spell == nullptr)
        return 0;

    lua->pushstring(spell->name);
    return 1;
}

int fb::model::spell::builtin_message(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto spell = lua->touserdata<fb::model::spell>(1);
    if (spell == nullptr)
        return 0;

    lua->pushstring(spell->message);
    return 1;
}