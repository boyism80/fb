#include <algorithm>
#include <fb/game/server.h>
#include <fb/game/item.h>
#include <fb/model/model.h>

using namespace fb::game;

pack::pack(fb::game::server& server, const fb::model::pack& model) :
    fb::game::item(server, model)
{
    this->_durability = model.durability;
}

pack::pack(const pack& right) :
    item(right),
    _durability(right._durability)
{ }

pack::~pack()
{ }

std::optional<uint32_t> pack::durability() const
{
    return this->_durability;
}

void pack::durability(uint32_t value)
{
    auto& model       = this->based<fb::model::pack>();
    this->_durability = std::max(uint32_t(0), std::min(model.durability, value));
}

std::string pack::inven_name() const
{
    auto& model = this->based<fb::model::pack>();
    return std::format("{} [{}]잔", model.name, this->_durability);
}

bool pack::active()
{
    auto owner = this->_container->owner();
    if (owner == nullptr)
        return false;

    if (this->_container == nullptr)
        return false;

    if (this->_durability <= 0)
        return false;

    fb::game::item::active();
    this->_durability--;
    if (this->_durability <= 0)
        this->count(0);

    auto shared = this->shared_from_this_as<fb::game::item>();
    owner->listener.on_item_update(*owner, this->_container->index(shared));
    if (this->empty())
        std::ignore = this->_container->remove(shared, 0xFF, ITEM_DELETE_TYPE::REDUCE);

    return true;
}