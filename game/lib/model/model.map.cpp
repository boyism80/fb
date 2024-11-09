#include <model.h>

int fb::model::map::builtin_name(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto map = thread->touserdata<fb::model::map>(1);
    if (map == nullptr)
        return 0;

    thread->pushstring(map->name);
    return 1;
}

fb::model::map* fb::model::__map::name2map(const std::string& name) const
{
    for (auto& [k, v] : *this)
    {
        if (v.name == name)
            return &v;
    }

    return nullptr;
}