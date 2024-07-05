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

std::string fb::game::pack::vname(NAME_OPTION option) const
{
    auto sstream = std::stringstream();
    auto& model = this->based<fb::model::pack>();
    sstream << model.name;

    switch(option)
    {
        case NAME_OPTION::DURABILITY:
        case NAME_OPTION::TRADE:
        {
            sstream << " [" 
                    << this->_durability 
                    << " 잔]";
        }
        break;
    }

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