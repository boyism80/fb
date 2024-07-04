#include <fb/game/model.h>

const fb::model::sell* fb::model::__sell::find(uint32_t pursuit, const fb::model::item& item) const
{
    if (this->contains(pursuit) == false)
        return nullptr;

    for (auto& [_, sell] : (*this)[pursuit])
    {
        if (sell.item == item.id)
            return &sell;
    }

    return nullptr;
}

const fb::model::sell* fb::model::__sell::find(const fb::model::npc& npc, const fb::model::item& item) const
{
    for (auto pursuit : npc.sell)
    {
        auto found = this->find(pursuit, item);
        if (found != nullptr)
            return found;
    }

    return nullptr;
}