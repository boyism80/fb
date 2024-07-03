#include <fb/game/item.h>
#include <fb/game/context.h>

fb::game::item::item(fb::game::context& context, const fb::model::item& model, const fb::game::item::config& config) : 
    fb::game::object(context, model, config),
    _count(config.count)
{ }

fb::game::item::item(const fb::game::item& right) : 
    fb::game::object(right.context, right._model, fb::game::item::config { .count = right._count })
{ }

fb::game::item::~item()
{ }

std::optional<uint32_t> fb::game::item::durability() const
{
    return std::nullopt;
}

void fb::game::item::durability(uint32_t value)
{ }



std::string fb::game::item::tip_message() const
{
    std::stringstream       sstream;
    auto&                   model = this->based<fb::model::item>();

    sstream << "가격: " << model.price;
    const std::string& desc = model.desc;
    if(desc.empty() == false)
        sstream << std::endl << std::endl << desc;
    return sstream.str();
}

std::string fb::game::item::detailed_name() const
{
    auto& model = this->based<fb::model::item>();

    if(model.attr(ITEM_ATTRIBUTE::BUNDLE) && this->_count > 1)
    {
        auto sstream = std::stringstream();
        sstream << model.name
                << ' ' << this->_count 
                << "개";
        return sstream.str();
    }
    else
    {
        return model.name;
    }
}

uint16_t fb::game::item::fill(uint16_t count)
{
    // 추가하고 남은 갯수 리턴
    auto                    space = this->free_space();
    auto                    addition = std::min(space, count);

    this->_count += addition;
    return std::max(0, count - space);
}

uint16_t fb::game::item::free_space() const
{
    auto& model = this->based<fb::model::item>();
    return model.capacity - this->_count;
}

uint16_t fb::game::item::count() const
{
    return this->_count;
}

void fb::game::item::count(uint16_t value)
{
    this->_count = value;
}

bool fb::game::item::empty() const
{
    return this->_count == 0;
}

fb::game::session* fb::game::item::owner() const
{
    return this->_owner;
}

void fb::game::item::owner(fb::game::session* owner)
{
    this->_owner = owner;
}

bool fb::game::item::active()
{
    if(this->empty())
        this->_owner->items.remove(*this);

    return false;
}

fb::game::item* fb::game::item::split(uint16_t count)
{
    auto& model = this->based<fb::model::item>();
    if(model.attr(ITEM_ATTRIBUTE::BUNDLE) && this->_count > count)
    {
        this->_count -= count;
        return model.make(this->context, count);
    }
    else
    {
        return this;
    }
}

void fb::game::item::merge(fb::game::item& item)
{
    auto& model = this->based<fb::model::item>();
    if(model.attr(ITEM_ATTRIBUTE::BUNDLE) == false)
        return;

    if(model != item.based())
        return;
 
    auto before = this->_count;
    auto remain = this->fill(item.count());
    item.count(remain);

    auto listener = this->_owner->get_listener<fb::game::session>();
    
    if(listener != nullptr)
    {
        if(before != this->_count)
            listener->on_item_update(static_cast<session&>(*this->_owner), this->_owner->items.index(*this));

        if(remain > 0 && this->_count == model.capacity)
            listener->on_notify(*this->_owner, fb::game::message::item::CANNOT_PICKUP_ANYMORE);
    }
}
