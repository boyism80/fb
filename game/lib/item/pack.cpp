#include <algorithm>
#include <fb/game/item.h>
#include <fb/game/context.h>
#include <fb/game/model.h>

fb::game::pack::pack(fb::game::context& context, const fb::model::pack& model) : 
    fb::game::item(context, model)
{
    this->_durability = model.durability;
}

fb::game::pack::pack(const pack& right) : 
    item(right),
    _durability(right._durability)
{ }

fb::game::pack::~pack()
{ }


std::optional<uint32_t> fb::game::pack::durability() const
{
    return this->_durability;
}

void fb::game::pack::durability(uint32_t value)
{
    auto& model = this->based<fb::model::pack>();
    this->_durability = std::max(uint32_t(0), std::min(model.durability, value));
}

std::string fb::game::pack::detailed_name() const
{
    auto& model = this->based<fb::model::pack>();
    auto sstream = std::stringstream();
    sstream << model.name
            << " [" 
            << this->_durability 
            << " 잔]";

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
        this->_owner->items.remove(*this, 0xFF, ITEM_DELETE_TYPE::REDUCE);

    return true;
}