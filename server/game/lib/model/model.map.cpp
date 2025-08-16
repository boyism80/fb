#include <fb/model/model.h>
#include <fb/game/server.h>

fb::model::map* fb::model::__map::name2map(const std::string& name) const
{
    for (auto& [k, v] : *this)
    {
        if (v.name == name)
            return &v;
    }

    return nullptr;
}