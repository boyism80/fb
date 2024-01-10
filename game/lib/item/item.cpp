#include <fb/game/item.h>
#include <fb/game/context.h>

const fb::game::item::conditions fb::game::item::DEFAULT_CONDITION;

fb::game::item::model::model(const fb::game::item::model::config& config) : 
    fb::game::object::model(config),
    id(config.id),
    price(config.price),
    condition(config.condition),
    penalty(config.penalty),
    capacity(config.capacity),
    trade(config.trade),
    storage(config.storage),
    desc(config.desc),
    active_script(config.active_script)
{ }

fb::game::item::model::~model()
{ }

fb::game::item::attrs fb::game::item::model::attr() const
{
    auto                    attr = attrs::NONE;
    if(this->capacity > 1)
        attr = attrs((uint32_t)attr | (uint32_t)attrs::BUNDLE);
    return attr;
}

bool fb::game::item::model::attr(fb::game::item::attrs flag) const
{
    return ((uint32_t)this->attr() & (uint32_t)flag) == (uint32_t)flag;
}

int fb::game::item::model::builtin_make(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto context = thread->env<fb::game::context>("context");
    auto model = thread->touserdata<fb::game::item::model>(1);
    auto object = model->make(*context);

    auto map = thread->touserdata<fb::game::map>(2);
    object->map(map);

    if(lua_istable(lua, 3))
    {
        lua_rawgeti(lua, 3, 1);
        object->x((uint16_t)thread->tointeger(-1));
        lua_remove(lua, -1);

        lua_rawgeti(lua, 3, 2);
        object->y((uint16_t)thread->tointeger(-1));
        lua_remove(lua, -1);
    }
    else
    {
        object->position((uint16_t)thread->tointeger(3), (uint16_t)thread->tointeger(4));
    }

    context->send(*object, fb::protocol::game::response::object::show(*object), fb::game::context::scope::PIVOT);
    thread->pushobject(object);
    return 1;
}

int fb::game::item::model::builtin_attr(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto context = thread->env<fb::game::context>("context");
    auto model = thread->touserdata<fb::game::item::model>(1);
    auto flag = (fb::game::item::attrs)thread->tointeger(2);

    thread->pushboolean(model->attr(flag));
    return 1;
}

int fb::game::item::model::builtin_capacity(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto context = thread->env<fb::game::context>("context");
    auto model = thread->touserdata<fb::game::item::model>(1);

    thread->pushinteger(model->capacity);
    return 1;
}





//
// class item
//

fb::game::item::item(fb::game::context& context, const fb::game::item::model* model, const fb::game::item::config& config) : 
    fb::game::object(context, model, config),
    _count(config.count)
{ }

fb::game::item::item(const fb::game::item& right) : 
    fb::game::object(right.context, right._model, fb::game::item::config { .count = right._count })
{ }

fb::game::item::~item()
{ }

const std::string fb::game::item::name_styled() const
{
    std::stringstream       sstream;
    sstream << this->name();

    if(this->attr(item::attrs::BUNDLE) && this->_count > 1)
        sstream << ' ' << this->_count << "개";

    return sstream.str();
}

const std::string fb::game::item::name_trade() const
{
    return this->name_styled();
}

std::optional<uint16_t> fb::game::item::durability() const
{
    return std::nullopt;
}

void fb::game::item::durability(std::optional<uint16_t> value)
{ }



std::string fb::game::item::tip_message() const
{
    std::stringstream       sstream;
    auto                    model = this->based<fb::game::item>();

    sstream << "가격: " << model->price;
    const std::string& desc = model->desc;
    if(desc.empty() == false)
        sstream << std::endl << std::endl << desc;
    return sstream.str();
}

uint16_t fb::game::item::fill(uint16_t count)
{
    // 추가하고 남은 갯수 리턴
    auto                    space = this->free_space();
    auto                    addition = std::min(space, count);

    this->_count += addition;
    return std::max(0, count - space);
}

uint16_t fb::game::item::reduce(uint16_t count)
{
    count = std::min(this->_count, count);
    this->_count -= count;
    return this->_count;
}

uint16_t fb::game::item::free_space() const
{
    auto model = this->based<fb::game::item>();
    return model->capacity - this->_count;
}

uint16_t fb::game::item::count() const
{
    return this->_count;
}

void fb::game::item::count(uint16_t value)
{
    auto model = this->based<fb::game::item>();
    this->_count = std::min(value, model->capacity);
}

bool fb::game::item::empty() const
{
    return this->_count == 0;
}

fb::game::item::attrs fb::game::item::attr() const
{
    return static_cast<const model*>(this->_model)->attr();
}

bool fb::game::item::attr(fb::game::item::attrs flag) const
{
    return static_cast<const model*>(this->_model)->attr(flag);
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
    auto model = this->based<fb::game::item>();
    if(model->trade.enabled == false)
        throw std::runtime_error(message::exception::CANNOT_DROP_ITEM);

    if(this->attr(item::attrs::BUNDLE) && this->_count > count)
    {
        this->_count -= count;
        return this->based<fb::game::item>()->make(this->context, count);
    }
    else
    {
        return this;
    }
}

void fb::game::item::merge(fb::game::item& item)
{
    if(this->attr(fb::game::item::attrs::BUNDLE) == false)
        return;

    if(this->based() != item.based())
        return;
 
    auto before = this->_count;
    auto remain = this->fill(item.count());
    item.count(remain);

    auto listener = this->_owner->get_listener<fb::game::session>();
    auto model = this->based<fb::game::item>();
    if(before != this->_count)
        listener->on_item_update(static_cast<session&>(*this->_owner), this->_owner->items.index(*this));

    if(remain > 0 && this->_count == model->capacity)
        listener->on_notify(*this->_owner, fb::game::message::item::CANNOT_PICKUP_ANYMORE);
}

int fb::game::item::builtin_count(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto context = thread->env<fb::game::context>("context");
    auto item = thread->touserdata<fb::game::item>(1);

    thread->pushinteger(item->count());
    return 1;
}