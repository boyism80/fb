#include <algorithm>
#include <context.h>
#include <item.h>
#include <fb/model/model.h>

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
    auto& model       = this->based<fb::model::pack>();
    this->_durability = std::max(uint32_t(0), std::min(model.durability, value));
}

std::string fb::game::pack::inven_name() const
{
    auto& model   = this->based<fb::model::pack>();
    auto  sstream = std::stringstream();
    sstream << model.name << " [" << this->_durability << " 잔]";

    return sstream.str();
}

async::task<bool> fb::game::pack::active()
{
    if (this->_durability <= 0)
        co_return false;

    this->_durability--;
    if (this->_durability <= 0)
        this->count(0);

    auto listener = this->_owner->get_listener<fb::game::character>();
    if (listener != nullptr)
        co_await listener->on_item_update(*this->_owner, this->_owner->items.index(*this));

    if (this->empty())
        std::ignore = co_await this->_owner->items.remove(*this, 0xFF, ITEM_DELETE_TYPE::REDUCE);

    co_return true;
}