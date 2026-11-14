#include <fb/model/model.h>

#include <cstdint>

uint64_t fb::model::buy::calculate_price() const
{
    return this->price.value_or(fb::model::table::item[this->item].price / 2);
}

const fb::model::buy* fb::model::___buy::find(uint32_t pursuit, const fb::model::item& item) const
{
    if (this->contains(pursuit) == false)
        return nullptr;

    for (auto& [_, buy] : (*this)[pursuit])
    {
        if (buy.item == item.id)
            return &buy;
    }

    return nullptr;
}

const fb::model::buy* fb::model::___buy::find(const fb::model::npc& npc, const fb::model::item& item) const
{
    if (npc.buy.has_value() == false)
        return nullptr;

    return this->find(npc.buy.value(), item);
}