#include <algorithm>
#include <fb/game/server.h>
#include <fb/game/item.h>
#include <fb/model/model.h>

using namespace fb::game;

pack::pack(fb::game::server& server, const fb::model::pack& model, const initial_params& params) :
    fb::game::item(server, model, params)
{
    this->_durability = model.durability;
}

pack::pack(const pack& right) :
    item(right),
    _durability(right._durability)
{ }

pack::~pack()
{ }

const fb::model::pack& pack::model() const
{
    return static_cast<const fb::model::pack&>(fb::model::table::item[this->_model_id]);
}

std::optional<uint32_t> pack::durability() const
{
    return this->_durability;
}

void pack::durability(uint32_t value)
{
    auto& model       = this->model();
    this->_durability = std::max(uint32_t(0), std::min(model.durability, value));
}

std::string pack::inven_name() const
{
    auto& model = this->model();
    return std::format("{} [{}]잔", model.name, this->_durability);
}

async::task<bool> pack::active()
{
    auto owner = this->_container->owner();
    if (owner == nullptr)
        co_return false;

    if (this->_container == nullptr)
        co_return false;

    if (this->_durability <= 0)
        co_return false;

    std::ignore = co_await fb::game::item::active();
    this->_durability--;
    if (this->_durability <= 0)
        this->count(0);

    auto shared = this->shared_from_this_as<fb::game::item>();
    owner->listener.on_item_update(*owner, this->_container->index(shared));
    if (this->empty())
        std::ignore = this->_container->remove(shared, 0xFF, ITEM_DELETE_TYPE::REDUCE);

    co_return true;
}