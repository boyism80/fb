#include <fb/model/model.h>

#include <cstdint>

uint64_t fb::model::sell::calculate_price() const
{
    return this->price.value_or(fb::model::table::item[this->item].price);
}

const fb::model::sell* fb::model::___sell::find(uint32_t pursuit, const fb::model::item& item) const
{
    if (this->contains(pursuit) == false)
        return nullptr;

    for (auto& sell : (*this)[pursuit])
    {
        if (sell.item == item.id)
            return &sell;
    }

    return nullptr;
}

const fb::model::sell* fb::model::___sell::find(const fb::model::npc& npc, const fb::model::item& item) const
{
    for (auto pursuit : npc.sell)
    {
        auto found = this->find(pursuit, item);
        if (found != nullptr)
            return found;
    }

    return nullptr;
}