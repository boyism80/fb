#include <fb/model/model.h>
#include <fb/game/server.h>

fb::model::npc* fb::model::__npc::name2npc(const std::string& name) const
{
    for (auto& npc : *this)
    {
        if (npc.second.name == name)
            return &npc.second;
    }

    return nullptr;
}