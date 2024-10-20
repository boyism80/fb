#include <item.h>
#include <context.h>

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


void fb::game::item::on_map_changed(fb::game::map* map)
{
    if(map == nullptr)
        this->_dropped_time = std::nullopt;
    else
        this->_dropped_time = datetime();
}

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

std::string fb::game::item::inven_name() const
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

std::string fb::game::item::trade_name() const
{
    return this->inven_name();
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

const fb::game::item::nullable_time& fb::game::item::dropped_time() const
{
    return this->_dropped_time;
}

fb::game::character* fb::game::item::owner() const
{
    return this->_owner;
}

void fb::game::item::owner(fb::game::character* owner)
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

    auto listener = this->_owner->get_listener<fb::game::character>();
    
    if(listener != nullptr)
    {
        if(before != this->_count)
            listener->on_item_update(static_cast<character&>(*this->_owner), this->_owner->items.index(*this));

        if(remain > 0 && this->_count == model.capacity)
            listener->on_notify(*this->_owner, fb::game::message::item::CANNOT_PICKUP_ANYMORE);
    }
}


fb::protocol::db::Item fb::game::item::to_protocol() const
{
    if (this->_owner == nullptr)
        throw std::runtime_error("cannot convert to protocol because owner is empty");

    auto& model = this->based<fb::model::item>();
    auto result = fb::protocol::db::Item();
    result.user = this->_owner->id();
    result.index = -1;
    result.parts = (uint16_t)EQUIPMENT_PARTS::UNKNOWN;
    result.deposited = -1;
    result.model = model.id;
    result.count = this->_count;
    result.durability = this->durability();
    result.custom_name = "";
    return result;
}