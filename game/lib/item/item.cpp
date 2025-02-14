#include <fb/game/context.h>
#include <fb/game/item.h>

using namespace fb::game;

item::item(fb::game::context& context, const fb::model::item& model, const initial_params& params) :
    object(context, model, params),
    _count(params.count)
{ }

item::item(const item& right) :
    object(right.context, right._model, initial_params{.count = right._count})
{ }

item::~item()
{ }

std::optional<uint32_t> item::durability() const
{
    return std::nullopt;
}

void item::durability(uint32_t value)
{ }

async::task<bool> item::map(fb::game::map* map, const fb::model::point16_t& position, DESTROY_TYPE destroy_type)
{
    auto result = co_await object::map(map, position);
    if (!result)
        co_return false;

    if (map == nullptr)
        this->_dropped_time = std::nullopt;
    else
        this->_dropped_time = fb::model::datetime();

    co_return true;
}

std::string item::tip_message() const
{
    std::stringstream sstream;
    auto&             model = this->based<fb::model::item>();

    sstream << "가격: " << model.price;
    const std::string& desc = model.desc;
    if (desc.empty() == false)
        sstream << std::endl << std::endl << desc;
    return sstream.str();
}

std::string item::inven_name() const
{
    auto& model = this->based<fb::model::item>();
    auto  count = this->_count - this->_trade_count;
    if (model.attr(ITEM_ATTRIBUTE::BUNDLE) && count > 1)
    {
        auto sstream = std::stringstream();
        sstream << model.name << ' ' << count << "개";
        return sstream.str();
    }
    else
    {
        return model.name;
    }
}

std::string item::trade_name() const
{
    auto& model = this->based<fb::model::item>();

    if (model.attr(ITEM_ATTRIBUTE::BUNDLE) && this->_trade_count > 1)
    {
        auto sstream = std::stringstream();
        sstream << model.name << ' ' << this->_trade_count << "개";
        return sstream.str();
    }
    else
    {
        return model.name;
    }
}

uint16_t item::fill(uint16_t count)
{
    // 추가하고 남은 갯수 리턴
    auto space    = this->free_space();
    auto addition = std::min(space, count);

    this->_count += addition;
    return std::max(0, count - space);
}

uint16_t item::free_space() const
{
    auto& model = this->based<fb::model::item>();
    return model.capacity - this->_count;
}

uint16_t item::count() const
{
    return this->_count;
}

void item::count(uint16_t value)
{
    this->_count = value;
}

uint16_t item::trade_count() const
{
    return this->_trade_count;
}

void item::trade_count(uint16_t value)
{
    this->_trade_count = value;
}

bool item::empty() const
{
    return this->_count == 0;
}

const item::nullable_time& item::dropped_time() const
{
    return this->_dropped_time;
}

character* item::owner() const
{
    return this->_owner;
}

void item::owner(character* owner)
{
    this->_owner = owner;
}

bool item::active()
{
    if (this->empty())
        std::ignore = this->_owner->items.remove(*this);

    return false;
}

item* item::split(uint16_t count)
{
    auto& model = this->based<fb::model::item>();
    if (model.attr(ITEM_ATTRIBUTE::BUNDLE) && this->_count > count)
    {
        this->_count -= count;
        return model.make(this->context, count);
    }
    else
    {
        return this;
    }
}

void item::merge(item& item)
{
    auto& model = this->based<fb::model::item>();
    if (model.attr(ITEM_ATTRIBUTE::BUNDLE) == false)
        return;

    if (model != item.based())
        return;

    auto before = this->_count;
    auto remain = this->fill(item.count());
    item.count(remain);

    auto listener = this->_owner->get_listener<character>();

    if (listener != nullptr)
    {
        if (before != this->_count)
            listener->on_item_update(static_cast<character&>(*this->_owner), this->_owner->items.index(*this));
    }

    if (remain > 0 && this->_count == model.capacity)
        this->_owner->message(_TEXT(MESSAGE_ITEM_CANNOT_PICKUP_ANYMORE));
}

void fb::game::item::assert_thread() const
{
    if (this->_owner != nullptr)
        this->_owner->assert_thread();
    else if (this->_map == nullptr)
        return;
    else
        object::assert_thread();
}

fb::protocol::internal::Item item::to_protocol(EQUIPMENT_PARTS parts) const
{
    if (this->_owner == nullptr)
        throw std::runtime_error("cannot convert to protocol because owner is empty");

    auto& model        = this->based<fb::model::item>();
    auto  result       = fb::protocol::internal::Item();
    result.user        = this->_owner->id();
    result.index       = -1;
    result.parts       = static_cast<uint16_t>(parts);
    result.deposited   = -1;
    result.model       = model.id;
    result.count       = this->_count;
    result.durability  = this->durability();
    result.custom_name = "";
    return result;
}