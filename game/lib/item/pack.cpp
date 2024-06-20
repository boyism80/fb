#include <algorithm>
#include <fb/game/item.h>
#include <fb/game/context.h>

fb::game::pack::model::model(const fb::model::item::config& config) : fb::model::item(config),
    durability(config.capacity)
{ }

fb::game::pack::model::~model()
{ }

ITEM_ATTRIBUTE fb::game::pack::model::attr() const
{
    return ITEM_ATTRIBUTE::PACK;
}

fb::game::pack::pack(fb::game::context& context, const fb::game::pack::model* model) : 
    fb::game::item(context, model)
{
    this->_durability = model->durability;
}

fb::game::pack::pack(const pack& right) : 
    item(right),
    _durability(right._durability)
{ }

fb::game::pack::~pack()
{ }


std::optional<uint16_t> fb::game::pack::durability() const
{
    return this->_durability;
}

void fb::game::pack::durability(uint16_t value)
{
    auto model = this->based<fb::game::pack>();
    this->_durability = std::max(uint16_t(0), std::min(model->durability, value));
}

const std::string fb::game::pack::name_styled() const
{
    std::stringstream sstream;
    sstream << this->name() << " [" << this->_durability << " 잔]";

    return sstream.str();
}

bool fb::game::pack::active()
{
    if(this->_durability <= 0)
        return false;

    this->_durability--;
    if(this->_durability <= 0)
        this->count(0);
    
    auto listener = this->_owner->get_listener<fb::game::session>();
    if(listener != nullptr)
        listener->on_item_update(*this->_owner, this->_owner->items.index(*this));

    if(this->empty())
        this->_owner->items.remove(*this, 0xFF, DELETE_TYPE::REDUCE);

    return true;
}