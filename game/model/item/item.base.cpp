#include "model/item/item.base.h"
#include "model/acceptor/acceptor.game.h"

fb::game::item::conditions::conditions() : 
    level(0),
    strength(0),
    dexteritry(0),
    intelligence(0),
    cls(0),
    promotion(0),
    sex(fb::game::sex::BOTH)
{ }

fb::game::item::conditions::conditions(const conditions& right) : 
    level(right.level),
    strength(right.strength),
    dexteritry(right.dexteritry),
    intelligence(right.intelligence),
    cls(right.cls),
    promotion(right.promotion),
    sex(right.sex)
{ }

fb::game::item::master::master(uint32_t id, const std::string& name, uint16_t look, uint8_t color, uint16_t capacity, const fb::game::item::conditions& condition) : 
    fb::game::object::master(name, look, color),
    _id(id),
    _price(0),
    _capacity(capacity),
    _condition(condition),
    _penalty(penalties::DROP),
    _trade(true),
    _bundle(false),
    _entrust(0)
{}

fb::game::item::master::master(const fb::game::object::master& master) : 
    fb::game::object::master(master)
{}

fb::game::item::master::~master()
{}

uint32_t fb::game::item::master::id() const
{
    return this->_id;
}

void fb::game::item::master::id(uint32_t id)
{
    this->_id = id;
}

uint32_t fb::game::item::master::price() const
{
    return this->_price;
}

void fb::game::item::master::price(uint32_t value)
{
    this->_price = value;
}

uint16_t fb::game::item::master::capacity() const
{
    return this->_capacity;
}

void fb::game::item::master::capacity(uint16_t value)
{
    this->_capacity = value;
}

bool fb::game::item::master::trade() const
{
    return this->_trade._enabled;
}

void fb::game::item::master::trade(bool value)
{
    this->_trade._enabled = value;
}

bool fb::game::item::master::entrust_enabled() const
{
    return this->_entrust._enabled;
}

void fb::game::item::master::entrust_enabled(bool value)
{
    this->_entrust._enabled = value;
}

uint32_t fb::game::item::master::entrust_price() const
{
    return this->_entrust._price;
}

void fb::game::item::master::entrust_price(uint32_t value)
{
    this->_entrust._price = value;
}

const fb::game::item::conditions& fb::game::item::master::condition() const
{
    return this->_condition;
}

void fb::game::item::master::condition(const item::conditions& value)
{
    this->_condition = value;
}

fb::game::item::penalties fb::game::item::master::penalty() const
{
    return this->_penalty;
}

void fb::game::item::master::penalty(penalties value)
{
    this->_penalty = value;
}

const std::string& fb::game::item::master::desc() const
{
    return this->_desc;
}

void fb::game::item::master::desc(const std::string& value)
{
    this->_desc = value;
}

const std::string& fb::game::item::master::active_script() const
{
    return this->_active_script;
}

void fb::game::item::master::active_script(const std::string& value)
{
    this->_active_script = value;
}

int fb::game::item::master::builtin_make(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto acceptor = thread->env<fb::game::acceptor>("acceptor");
    auto master = thread->touserdata<fb::game::item::master>(1);
    auto object = master->make(acceptor);

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

    acceptor->send(*object, fb::protocol::game::response::object::show(*object), acceptor::scope::PIVOT);

    object->to_lua(lua);
    return 1;
}

fb::game::item::attrs fb::game::item::master::attr() const
{
    auto                    attr = attrs::ITEM_ATTR_NONE;
    if(this->capacity() > 1)
        attr = attrs(attr | attrs::ITEM_ATTR_BUNDLE);
    return attr;
}

bool fb::game::item::master::attr(fb::game::item::attrs flag) const
{
    return (this->attr() & flag) == flag;
}

fb::game::item* fb::game::item::master::make(fb::game::item::listener* listener) const
{
    return new fb::game::item(this, listener);
}






//
// class item
//

fb::game::item::item(const fb::game::item::master* master, fb::game::item::listener* listener, uint16_t count) : 
    fb::game::object(master, listener),
    _owner(nullptr),
    _count(count)
{}

fb::game::item::item(const fb::game::item& right) : 
    fb::game::object(right._master, right.get_listener<fb::game::object::listener>()),
    _owner(nullptr),
    _count(right._count)
{ }

fb::game::item::~item()
{ }

const std::string fb::game::item::name_styled() const
{
    std::stringstream       sstream;
    sstream << this->name();

    if(this->attr(item::attrs::ITEM_ATTR_BUNDLE) && this->_count > 1)
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
{}



std::string fb::game::item::tip_message() const
{
    std::stringstream       sstream;

    sstream << "가격: " << this->price();
    const std::string& desc = this->desc();
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
    return this->capacity() - this->_count;
}

uint16_t fb::game::item::count() const
{
    return this->_count;
}

void fb::game::item::count(uint16_t value)
{
    this->_count = std::min(value, this->capacity());
}

bool fb::game::item::empty() const
{
    return this->_count == 0;
}

uint32_t fb::game::item::price() const
{
    return static_cast<const master*>(this->_master)->_price;
}

uint16_t fb::game::item::capacity() const
{
    return static_cast<const master*>(this->_master)->_capacity;
}

bool fb::game::item::unique() const
{
    return !static_cast<const master*>(this->_master)->_trade._enabled;
}

bool fb::game::item::entrust_enabled() const
{
    return static_cast<const master*>(this->_master)->_entrust._enabled;
}

uint32_t fb::game::item::entrust_price() const
{
    return static_cast<const master*>(this->_master)->_entrust._price;
}

const fb::game::item::conditions& fb::game::item::condition() const
{
    return static_cast<const master*>(this->_master)->_condition;
}

fb::game::item::penalties fb::game::item::penalty() const
{
    return static_cast<const master*>(this->_master)->_penalty;
}

const std::string& fb::game::item::desc() const
{
    return static_cast<const master*>(this->_master)->_desc;
}

const std::string& fb::game::item::active_script() const
{
    return static_cast<const master*>(this->_master)->_active_script;
}

fb::game::item::attrs fb::game::item::attr() const
{
    return static_cast<const master*>(this->_master)->attr();
}

bool fb::game::item::attr(fb::game::item::attrs flag) const
{
    return static_cast<const master*>(this->_master)->attr(flag);
}

fb::game::session* fb::game::item::owner() const
{
    return this->_owner;
}

void fb::game::item::owner(fb::game::session* owner)
{
    this->_owner = owner;
    if(owner == nullptr)
        this->set_listener(nullptr);
    else
        this->set_listener(owner->get_listener<fb::game::item::listener>());
}

bool fb::game::item::active()
{
    if(this->empty())
        this->_owner->items.remove(*this);

    return false;
}

fb::game::item* fb::game::item::split(uint16_t count)
{
    if(this->unique())
        throw std::runtime_error(message::exception::CANNOT_DROP_ITEM);

    auto listener = this->_owner != nullptr ? this->_owner->get_listener<fb::game::session::listener>() : nullptr;
    if(this->attr(item::attrs::ITEM_ATTR_BUNDLE) && this->_count > count)
    {
        this->_count -= count;
        return this->based<fb::game::item>()->make(listener, count);
    }
    else
    {
        return this;
    }
}

void fb::game::item::merge(fb::game::item& item)
{
    if(this->attr(fb::game::item::attrs::ITEM_ATTR_BUNDLE) == false)
        return;

    if(this->based() != item.based())
        return;
 
    auto before = this->_count;
    auto remain = this->fill(item.count());
    item.count(remain);

    auto listener = this->_owner->get_listener<fb::game::session::listener>();
    if(listener != nullptr)
    {
        if(before != this->_count)
            listener->on_item_update(static_cast<session&>(*this->_owner), this->_owner->items.index(*this));

        if(remain > 0 && this->_count == this->based<fb::game::item>()->capacity())
            listener->on_notify(*this->_owner, fb::game::message::item::CANNOT_PICKUP_ANYMORE);
    }
}