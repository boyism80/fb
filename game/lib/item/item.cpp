#include <fb/game/context.h>
#include <fb/game/item.h>

using namespace fb::game;

item::item(fb::game::context& context, const fb::model::item& model, const initial_params& params) :
    object(context, model, params),
    listener(context.listener),
    _count(params.count)
{ }

item::item(const item& right) :
    object(right.context, right._model, initial_params{.count = right._count}),
    listener(right.listener)
{ }

item::~item()
{ }

std::optional<uint32_t> item::durability() const
{
    return std::nullopt;
}

void item::durability(uint32_t value)
{ }

async::task<bool> item::map(std::shared_ptr<fb::game::map> map,
                            const fb::model::point16_t&    position,
                            DESTROY_TYPE                   destroy_type)
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

void fb::game::item::death_cid(std::optional<uint32_t> cid)
{
    this->_death_cid = cid;
}

std::optional<uint32_t> fb::game::item::death_cid() const
{
    return this->_death_cid;
}

bool item::active()
{
    if (this->_container == nullptr)
        return false;

    if (this->empty())
        std::ignore = this->_container->remove(this->shared_from_this_as<fb::game::item>());

    auto& model = this->based<fb::model::item>();
    if (model.script.empty())
        return false;

    if (model.on_active.empty())
        return false;

    // Execute item activation script
    auto lua = fb::lua::new_context();
    if (lua != nullptr)
    {
#if defined DEBUG | defined _DEBUG
        lua->load(model.script);
#endif
        lua->func(model.on_active);
        lua->pushobject(this->_container->owner);
        lua->pushobject(*this);
        std::ignore = lua->call(2);
    }

    // Call listener for packet response
    this->listener.on_item_active(this->_container->owner, *this);
    return true;
}

std::shared_ptr<fb::game::item> item::split(uint16_t count)
{
    auto& model = this->based<fb::model::item>();
    if (model.attr(ITEM_ATTRIBUTE::BUNDLE) && this->_count > count)
    {
        this->_count -= count;
        return std::static_pointer_cast<fb::game::item>(model.make(this->context, count));
    }
    else
    {
        return this->shared_from_this_as<fb::game::item>();
    }
}

void item::merge(std::shared_ptr<fb::game::item> item)
{
    if (this->_container == nullptr)
        return;

    auto& model = this->based<fb::model::item>();
    if (model.attr(ITEM_ATTRIBUTE::BUNDLE) == false)
        return;

    if (model != item->based())
        return;

    auto& owner  = this->_container->owner;
    auto  before = this->_count;
    auto  remain = this->fill(item->count());
    item->count(remain);

    if (before != this->_count)
        this->listener.on_item_update(static_cast<character&>(owner),
                                      owner.items.index(this->shared_from_this_as<fb::game::item>()));

    if (remain > 0 && this->_count == model.capacity)
        owner.message(_TEXT(MESSAGE_ITEM_CANNOT_PICKUP_ANYMORE));
}

fb::thread* fb::game::item::thread() const
{
    if (this->_map != nullptr)
        return this->context.threads.modular(this->_map->model.id);

    if (this->_container != nullptr)
        return this->_container->owner.thread();

    return this->context.threads.current();
}

void fb::game::item::assert_thread() const
{
    if (this->_container != nullptr)
        this->_container->owner.assert_thread();
    else if (this->_map == nullptr)
        return;
    else
        object::assert_thread();
}

void fb::game::item::container(fb::game::items* container)
{
    this->_container = container;
}

fb::protocol::internal::Item item::to_protocol(EQUIPMENT_PARTS parts) const
{
    if (this->_container == nullptr)
        throw std::runtime_error("cannot convert to protocol because container is empty");

    auto& model        = this->based<fb::model::item>();
    auto  result       = fb::protocol::internal::Item();
    result.user        = this->_container->owner.id();
    result.index       = -1;
    result.parts       = static_cast<uint16_t>(parts);
    result.stored      = -1;
    result.model       = model.id;
    result.count       = this->_count;
    result.durability  = this->durability();
    result.custom_name = "";
    return result;
}