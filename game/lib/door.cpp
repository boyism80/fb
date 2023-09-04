#include <fb/game/door.h>
#include <fb/game/map.h>
#include <fb/game/session.h>
#include <fb/game/context.h>

bool fb::game::door::model::matched(const fb::game::map& map, const point16_t& position, bool open) const
{
    auto door_size = this->size();
    for(int i = 0; i < door_size; i++)
    {
        auto now = map(position.x + i, position.y);
        if(now == nullptr)
            return false;

        auto dst = open ? this->at(i).open : this->at(i).close;

        if(now->object != dst)
            return false;
    }

    return true;
}

bool fb::game::door::model::find(const fb::game::map& map, point16_t& position, bool open) const
{
    bool init = true;
    for(int offset = position.y * map.width() + position.x; offset < map.width() * map.height(); offset++)
    {
        auto row = offset / map.width();
        auto col = offset % map.width();

        auto pos = point16_t(col, row);
        if(this->matched(map, pos, open) == false)
            continue;

        position = pos;
        return true;
    }

    return false;
}

fb::game::door::door(fb::game::map* owner, fb::game::door::model& model, const point16_t position, bool opened) : 
    _owner(owner),
    _model(model), 
    position(position), 
    _opened(opened)
{ }

fb::game::door::~door()
{ }

const fb::game::door::model& fb::game::door::based() const
{
    return this->_model;
}

bool fb::game::door::toggle()
{
    this->_opened = !this->_opened;

    for(int i = 0; i < this->_model.size(); i++)
    {
        auto& object = (*this->_owner)(this->position.x + i, this->position.y)->object;
        auto  door_e = this->_opened ? this->_model[i].open : this->_model[i].close;

        object = door_e;
    }

    return true;
}

bool fb::game::door::opened() const
{
    return this->_opened;
}

bool fb::game::door::locked() const
{
    return this->_locked;
}

void fb::game::door::lock(bool value)
{
    this->_locked = value;
}

const fb::game::map& fb::game::door::map() const
{
    return *this->_owner;
}

int fb::game::door::builtin_toggle(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto argc = thread->argc();
    auto door = thread->touserdata<fb::game::door>(1);

    door->toggle();
    lua_pushboolean(lua, door->opened());
    return 1;
}

int fb::game::door::builtin_locked(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto argc = thread->argc();
    auto door = thread->touserdata<fb::game::door>(1);

    lua_pushboolean(lua, door->locked());
    return 1;
}

int fb::game::door::builtin_lock(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto argc = thread->argc();
    auto door = thread->touserdata<fb::game::door>(1);
    auto value = thread->toboolean(2);

    door->lock(value);
    lua_pushboolean(lua, door->locked());
    return 1;
}

int fb::game::door::builtin_opened(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto argc = thread->argc();
    auto door = thread->touserdata<fb::game::door>(1);

    lua_pushboolean(lua, door->opened());
    return 1;
}

int fb::game::door::builtin_update(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto argc = thread->argc();
    auto door = thread->touserdata<fb::game::door>(1);

    auto model = door->based();
    auto context = thread->env<fb::game::context>("context");

    const auto& map = door->map();
    const auto size = size8_t((uint8_t)model.size(), 1);
    context->send(fb::protocol::game::response::map::update(map, door->position, size), map);
    return 0;
}

fb::game::doors::doors()
{ }

fb::game::doors::~doors()
{ }

void fb::game::doors::add(map* map, fb::game::door::model& model, const point16_t position, bool opened)
{
    this->push_back(std::make_unique<fb::game::door>(map, model, position, opened));
}

fb::game::door* fb::game::doors::find(const point16_t position)
{
    for(const auto& door : *this)
    {
        const auto& model = door->based();
        for(int i = 0; i < model.size(); i++)
        {
            if(position == point16_t(door->position.x + i, door->position.y))
                return door.get();
        }
    }

    return nullptr;
}

fb::game::door* fb::game::doors::find(const fb::game::session& session)
{
    auto direction = session.direction();
    if(direction == fb::game::direction::LEFT || direction == fb::game::direction::RIGHT)
        return nullptr;

    auto forward = point16_t(session.x(), session.y() + (direction == fb::game::direction::TOP ? -1 : 1));
    return this->find(forward);
}