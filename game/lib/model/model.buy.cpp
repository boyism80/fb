#include <model.h>

const fb::model::buy* fb::model::__buy::find(uint32_t pursuit, const fb::model::item& item) const
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

const fb::model::buy* fb::model::__buy::find(const fb::model::npc& npc, const fb::model::item& item) const
{
	if(npc.buy.has_value() == false)
		return nullptr;

	return this->find(npc.buy.value(), item);
}