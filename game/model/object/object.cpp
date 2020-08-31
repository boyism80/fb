#include <model/object/object.h>
#include <model/map/map.h>
#include <model/session/session.h>
#include <model/mob/mob.h>
#include <model/acceptor/acceptor.game.h>
#include <model/listener/listener.h>
#include <builtin/builtin_function.h>

fb::game::object::master::master(const std::string& name, uint16_t look, uint8_t color) : 
    _name(name),
    _look(look),
    _color(color)
{
}

fb::game::object::master::~master()
{
}

uint8_t fb::game::object::master::dialog_look_type() const
{
    return this->look() > 0xBFFF ? 0x02 : 0x01;
}

fb::game::object::types fb::game::object::master::type() const
{
    return types::UNKNOWN;
}

const std::string& fb::game::object::master::name() const
{
    return this->_name;
}

void fb::game::object::master::name(const std::string& value)
{
    this->_name = value;
}

uint16_t fb::game::object::master::look() const
{
    return this->_look;
}

void fb::game::object::master::look(uint16_t value)
{
    this->_look = value;
}

uint8_t fb::game::object::master::color() const
{
    return this->_color;
}

void fb::game::object::master::color(uint8_t value)
{
    this->_color = value;
}

int fb::game::object::master::builtin_name(lua_State* lua)
{
    auto object = *(fb::game::object::master**)lua_touserdata(lua, 1);

    lua_pushstring(lua, object->_name.c_str());
    return 1;
}

int fb::game::object::master::builtin_look(lua_State* lua)
{
    auto object = *(fb::game::object::master**)lua_touserdata(lua, 1);

    lua_pushinteger(lua, object->_look);
    return 1;
}

int fb::game::object::master::builtin_color(lua_State* lua)
{
    auto object = *(fb::game::object::master**)lua_touserdata(lua, 1);

    lua_pushinteger(lua, object->_color);
    return 1;
}

int fb::game::object::master::builtin_dialog(lua_State* lua)
{
    // Ex) npc:dialog(session, "hello", true, true);
    return ::builtin_dialog<object::master>(lua);
}



// fb::game::object
fb::game::object::object(const fb::game::object::master* master, listener* listener, uint32_t id, const point16_t position, fb::game::direction direction, fb::game::map* map) : 
    luable(id),
    _listener(listener),
    _sequence(0xFFFFFFFF),
    _master(master),
    _position(position),
    _direction(direction),
    _map(map),
    buffs(*this),
    _visible(true)
{
}

fb::game::object::object(const object& right) :
    object(right._master, right._listener, right.sequence(), right._position, right._direction, right._map)
{
}

fb::game::object::~object()
{
    if(this->_map != nullptr)
        this->_map->objects.remove(*this);
}

const fb::game::object::master* fb::game::object::based() const
{
    return this->_master;
}

bool fb::game::object::is(object::types type)
{
    auto mine = this->type();
    return (type & mine) == mine;
}

const std::string& fb::game::object::name() const
{
    return this->_master->name();
}

uint16_t fb::game::object::look() const
{
    return this->_master->look();
}

uint8_t fb::game::object::color() const
{
    return this->_master->color();
}

fb::game::object::types fb::game::object::type() const
{
    return this->_master->type();
}

const fb::game::point16_t& fb::game::object::before() const
{
    return this->_before;
}

const fb::game::point16_t& fb::game::object::position() const
{
    return this->_position;
}

bool fb::game::object::position(uint16_t x, uint16_t y)
{
    if(this->_map == nullptr)
        return false;

    if(this->_position.x == x && this->_position.y == y)
        return true;

    this->_before.x = this->_position.x;
    this->_before.y = this->_position.y;

    this->_position.x = std::max(0, std::min(this->_map->width() - 1, int32_t(x)));
    this->_position.y = std::max(0, std::min(this->_map->height() - 1, int32_t(y)));

    for(auto x : this->_map->objects)
    {
        if(x == this)
            continue;

        bool                before = this->sight(*x, true);
        bool                after  = this->sight(*x);

        bool                show = (!before && after);
        bool                hide = (before && !after);

        if(show && this->_listener != nullptr)
            this->_listener->on_show(*this, *x, true);
        else if(hide && this->_listener != nullptr)
            this->_listener->on_hide(*this, *x);
    }

    for(auto x : this->_map->objects)
    {
        if(x == this)
            continue;

        bool                before_sight = x->sight(*this, false, true);
        bool                after_sight  = x->sight(*this);

        bool                show = (!before_sight && after_sight);
        bool                hide = (before_sight && !after_sight);

        if(show && this->_listener != nullptr)
            this->_listener->on_show(*x, *this, true);
        else if(hide && this->_listener != nullptr)
            this->_listener->on_hide(*x, *this);
    }

    return true;
}

bool fb::game::object::position(const point16_t position)
{
    return this->position(position.x, position.y);
}

bool fb::game::object::move()
{
    return this->move(this->_direction);
}

bool fb::game::object::move(fb::game::direction direction)
{
    if(this->_map == nullptr)
        return false;

    auto after = this->_position;
    switch(direction)
    {
    case fb::game::direction::TOP:
        after.y--;
        break;

    case fb::game::direction::BOTTOM:
        after.y++;
        break;

    case fb::game::direction::LEFT:
        after.x--;
        break;

    case fb::game::direction::RIGHT:
        after.x++;
        break;
    }

    if(this->_map->movable(after) == false)
        return false;

    if(this->direction(direction) == false)
        return false;

    this->position(after);
    if(this->_listener != nullptr)
        this->_listener->on_move(*this);

    return true;
}

const fb::game::point16_t& fb::game::object::position_forward() const
{
    point16_t forward(this->_position);
    forward.forward(this->_direction);
    if(this->_map->movable(forward))
        return forward;


    point16_t left(this->_position);
    left.left(this->_direction);
    if(this->_map->movable(left))
        return left;


    point16_t right(this->_position);
    right.right(this->_direction);
    if(this->_map->movable(right))
        return right;

    point16_t backward(this->_position);
    backward.backward(this->_direction);
    if(this->_map->movable(backward))
        return backward;

    return this->_position;
}

uint16_t fb::game::object::x() const
{
    return this->_position.x;
}

bool fb::game::object::x(uint16_t value)
{
    return this->position(value, this->_position.y);
}

uint16_t fb::game::object::y() const
{
    return this->_position.y;
}

bool fb::game::object::y(uint16_t value)
{
    return this->position(this->_position.x, value);
}

fb::game::direction fb::game::object::direction() const
{
    return this->_direction;
}

bool fb::game::object::direction(fb::game::direction value)
{
    if(value != fb::game::direction::LEFT && 
       value != fb::game::direction::TOP && 
       value != fb::game::direction::RIGHT && 
       value != fb::game::direction::BOTTOM)
        return false;

    if(this->_direction == value)
        return true;

    this->_direction = value;
    if(this->_listener != nullptr)
        this->_listener->on_direction(*this);

    return true;
}

fb::game::map* fb::game::object::map() const
{
    return this->_map;
}

bool fb::game::object::sight(const point16_t& position, bool before) const
{
    return fb::game::object::sight(before ? this->_before : this->_position, position, this->_map);
}

bool fb::game::object::sight(const fb::game::object& object, bool before_me, bool before_you) const
{
    if(this->_map == nullptr)
        return false;

    if(this->_map != object.map())
        return false;

    if(object.visible() == false)
        return false;

    return this->sight(before_you ? object._before : object._position, before_me);
}

bool fb::game::object::sight(const point16_t me, const point16_t you, const fb::game::map* map)
{
    point16_t begin, end;

    if(me.x <= fb::game::map::HALF_SCREEN_WIDTH) // 최좌측
    {
        begin.x = 0;
        end.x   = fb::game::map::MAX_SCREEN_WIDTH;
    }
    else if(me.x >= map->width() - fb::game::map::HALF_SCREEN_WIDTH) // 최우측
    {
        begin.x = std::max(int32_t(0), int32_t(map->width() - fb::game::map::MAX_SCREEN_WIDTH - 1));
        end.x   = std::max(int32_t(0), int32_t(map->width() - 1));
    }
    else
    {
        begin.x = std::max(int32_t(0), int32_t(me.x - fb::game::map::HALF_SCREEN_WIDTH - 1));
        end.x   = std::max(int32_t(0), int32_t(me.x + fb::game::map::HALF_SCREEN_WIDTH + 1));
    }


    if(me.y <= fb::game::map::HALF_SCREEN_HEIGHT) // 최상단
    {
        begin.y = 0;
        end.y   = fb::game::map::MAX_SCREEN_HEIGHT;
    }
    else if(me.y >= map->height() - fb::game::map::HALF_SCREEN_HEIGHT) // 최하단
    {
        begin.y = std::max(int32_t(0), int32_t(map->height() - fb::game::map::MAX_SCREEN_HEIGHT - 1));
        end.y   = std::max(int32_t(0), map->height() - 1);
    }
    else
    {
        begin.y = std::max(int32_t(0), int32_t(me.y - fb::game::map::HALF_SCREEN_HEIGHT - 1));
        end.y   = std::max(int32_t(0), int32_t(me.y + fb::game::map::HALF_SCREEN_HEIGHT + 1));
    }

    return begin.x <= you.x && end.x >= you.x &&
        begin.y <= you.y && end.y >= you.y;
}

void fb::game::object::map(fb::game::map* map, const point16_t& position)
{
    // 이전에 보이던 오브젝트들 전부 제거
    for(auto i : this->showings())
        this->_listener->on_hide(*this, *i);

    // 이전에 나를 보던 오브젝트들에게서 나를 제거
    for(auto i : this->showns())
        this->_listener->on_hide(*i, *this);

    this->_map = map;
    this->_position = position;
    if(this->_listener != nullptr && map != nullptr)
        this->_listener->on_warp(*this);

    // 지금 내가 볼 수 있는 오브젝트를 표시
    for(auto i : this->showings())
        this->_listener->on_show(*this, *i, false);

    // 지금 나를 볼 수 있는 오브젝트들에게 나를 표시
    for(auto i : this->showns())
        this->_listener->on_show(*i, *this, false);
}

void fb::game::object::map(fb::game::map* map)
{
    this->map(map, point16_t(0, 0));
}

fb::game::session* fb::game::object::near_session(fb::game::direction direction) const
{
    fb::game::map* map = this->_map;
    if(map == nullptr)
        return nullptr;


    point16_t front = this->position();
    switch(direction)
    {
    case fb::game::direction::TOP:
        front.y--;
        break;

    case fb::game::direction::BOTTOM:
        front.y++;
        break;

    case fb::game::direction::LEFT:
        front.x--;
        break;

    case fb::game::direction::RIGHT:
        front.x++;
        break;
    }

    if(map->existable(front) == false)
        return nullptr;

    for(auto session : map->objects.sessions())
    {
        if(session->position() == front)
            return session;
    }

    return nullptr;
}

std::vector<fb::game::session*> fb::game::object::near_sessions(fb::game::direction direction) const
{
    std::vector<fb::game::session*> list;

    fb::game::map* map = this->_map;
    if(map == nullptr)
        return list;


    point16_t front = this->position();
    switch(direction)
    {
    case fb::game::direction::TOP:
        front.y--;
        break;

    case fb::game::direction::BOTTOM:
        front.y++;
        break;

    case fb::game::direction::LEFT:
        front.x--;
        break;

    case fb::game::direction::RIGHT:
        front.x++;
        break;
    }

    if(map->existable(front) == false)
        return list;

    for(auto session : map->objects.sessions())
    {
        if(session->position() == front)
            list.push_back(session);
    }

    return list;
}

fb::game::session* fb::game::object::forward_session() const
{
    return this->near_session(this->_direction);
}

std::vector<fb::game::session*> fb::game::object::forward_sessions() const
{
    return this->near_sessions(this->_direction);
}

fb::game::object* fb::game::object::near_object(fb::game::direction direction, fb::game::object::types type) const
{
    fb::game::map* map = this->_map;
    if(map == nullptr)
        return nullptr;


    point16_t front = this->position();
    switch(direction)
    {
    case fb::game::direction::TOP:
        front.y--;
        break;

    case fb::game::direction::BOTTOM:
        front.y++;
        break;

    case fb::game::direction::LEFT:
        front.x--;
        break;

    case fb::game::direction::RIGHT:
        front.x++;
        break;
    }

    if(map->existable(front) == false)
        return nullptr;

    for(auto object : map->objects)
    {
        if(type != fb::game::object::types::UNKNOWN && object->is(type) == false)
            continue;

        if(object->position() == front)
            return object;
    }

    return nullptr;
}

std::vector<fb::game::object*> fb::game::object::near_objects(fb::game::direction direction, fb::game::object::types type) const
{
    std::vector<object*>    list;
    if(this->_map == nullptr)
        return list;


    point16_t front = this->position();
    switch(direction)
    {
    case fb::game::direction::TOP:
        front.y--;
        break;

    case fb::game::direction::BOTTOM:
        front.y++;
        break;

    case fb::game::direction::LEFT:
        front.x--;
        break;

    case fb::game::direction::RIGHT:
        front.x++;
        break;
    }

    if(this->_map->existable(front) == false)
        return list;

    for(auto object : this->_map->objects)
    {
        if(type != fb::game::object::types::UNKNOWN && object->is(type) == false)
            continue;

        if(object->position() == front)
            list.push_back(object);
    }

    return list;
}

fb::game::object* fb::game::object::forward_object(fb::game::object::types type) const
{
    return this->near_object(this->_direction, type);
}

std::vector<fb::game::object*> fb::game::object::forward_objects(fb::game::object::types type) const
{
    return this->near_objects(this->_direction, type);
}

std::vector<fb::game::object*> fb::game::object::showings(object::types type) const
{
    std::vector<object*>    list;
    if(this->_map == nullptr)
        return list;

    for(auto object : this->_map->objects)
    {
        if(object == this)
            continue;

        if(type != object::types::UNKNOWN && object->is(type) == false)
            continue;

        // 내가 object를 볼 수 있는가?
        if(this->sight(*object) == false)
            continue;

        list.push_back(object);
    }

    return list;
}

std::vector<object*> fb::game::object::showns(object::types type) const
{
    std::vector<object*> list;

    if(this->_map == nullptr)
        return list;

    for(auto object : this->_map->objects)
    {
        if(object == this)
            continue;

        if(type != object::types::UNKNOWN && object->is(type) == false)
            continue;

        // object가 나를 볼 수 있는가?
        if(object->sight(*this) == false)
            continue;

        list.push_back(object);
    }

    return list;
}

bool fb::game::object::visible() const
{
    return this->_visible;
}

void fb::game::object::visible(bool value)
{
    if(this->_visible == value)
        return;

    this->_visible = value;
}

double fb::game::object::distance(const object& right) const
{
    return std::sqrt(this->distance_sqrt(right));
}

uint32_t fb::game::object::distance_sqrt(const object& right) const
{
    return (uint32_t)std::pow(this->_position.x - right._position.x, 2) + 
        (uint32_t)std::pow(this->_position.y - right._position.y, 2);
}

bool fb::game::object::operator==(const object& right) const
{
    return this->_map == right._map &&
        this->sequence() == right.sequence();
}

bool fb::game::object::operator!=(const object& right) const
{
    return !((*this) == right);
}

int fb::game::object::builtin_core(lua_State* lua)
{
    auto object = *(fb::game::object**)lua_touserdata(lua, 1);
    auto master = object->based();
    master->to_lua(lua);
    return 1;
}

int fb::game::object::builtin_id(lua_State* lua)
{
    auto object = *(fb::game::object**)lua_touserdata(lua, 1);
    auto acceptor = lua::env<fb::game::acceptor>("acceptor");

    lua_pushinteger(lua, object->sequence());
    return 1;
}

int fb::game::object::builtin_eq(lua_State* lua)
{
    auto argc = lua_gettop(lua);
    auto me = *(fb::game::object**)lua_touserdata(lua, 1);
    auto you = *(fb::game::object**)lua_touserdata(lua, 2);
    
    lua_pushboolean(lua, me->sequence() == you->sequence());
    return 1;
}

int fb::game::object::builtin_tostring(lua_State* lua)
{
    auto me = *(fb::game::object**)lua_touserdata(lua, 1);

    lua_pushstring(lua, me->name().c_str());
    return 1;
}

int fb::game::object::builtin_name(lua_State* lua)
{
    auto object = *(fb::game::object**)lua_touserdata(lua, 1);
    auto acceptor = lua::env<fb::game::acceptor>("acceptor");

    lua_pushstring(lua, object->name().c_str());
    return 1;
}

int fb::game::object::builtin_dialog(lua_State* lua)
{
    return ::builtin_dialog<object>(lua);
}

int fb::game::object::builtin_sound(lua_State* lua)
{
    auto acceptor = lua::env<fb::game::acceptor>("acceptor");
    auto object = *(fb::game::object**)lua_touserdata(lua, 1);
    auto sound = lua_tointeger(lua, 2);

    acceptor->send(*object, response::game::object::sound(*object, sound::type(sound)), acceptor::scope::PIVOT);
    lua_pushinteger(lua, -1);
    return 1;
}

int fb::game::object::builtin_position(lua_State* lua)
{
    auto argc = lua_gettop(lua);
    auto object = *(fb::game::object**)lua_touserdata(lua, 1);

    if(argc == 1)
    {
        lua_pushinteger(lua, object->_position.x);
        lua_pushinteger(lua, object->_position.y);
        return 2;
    }
    

    uint16_t x, y;
    if(lua_istable(lua, 2))
    {
        lua_rawgeti(lua, 2, 1);
        x = (uint16_t)lua_tointeger(lua, -1);
        lua_remove(lua, -1);

        lua_rawgeti(lua, 2, 2);
        y = (uint16_t)lua_tointeger(lua, -1);
        lua_remove(lua, -1);
    }
    else
    {
        x = (uint16_t)lua_tointeger(lua, 2);
        y = (uint16_t)lua_tointeger(lua, 3);
    }

    std::vector<game::object*> shows, hides, showns, hiddens;
    object->position(x, y);

    auto acceptor = lua::env<fb::game::acceptor>("acceptor");
    if(object->is(fb::game::object::types::SESSION))
        acceptor->send(*object, response::game::session::show(static_cast<fb::game::session&>(*object)), acceptor::scope::PIVOT);
    else
        acceptor->send(*object, response::game::object::show(*object), acceptor::scope::PIVOT);

    if(object->is(object::types::SESSION))
    {
        auto session = static_cast<fb::game::session*>(object);
        acceptor->send(*object, response::game::session::position(*session), acceptor::scope::SELF);
    }

    return 0;
}

int fb::game::object::builtin_direction(lua_State* lua)
{
    auto argc = lua_gettop(lua);
    auto object = *(fb::game::object**)lua_touserdata(lua, 1);

    if(argc == 1)
    {
        lua_pushinteger(lua, object->_direction);
        return 1;
    }
    else
    {
        auto direction = game::direction(lua_tointeger(lua, 2));
        object->direction(direction);

        auto acceptor = lua::env<fb::game::acceptor>("acceptor");
        acceptor->send(*object, response::game::object::direction(*object), acceptor::scope::PIVOT);
        return 0;
    }
}

int fb::game::object::builtin_chat(lua_State* lua)
{
    auto argc = lua_gettop(lua);
    auto acceptor = lua::env<fb::game::acceptor>("acceptor");
    auto object = *(fb::game::object**)lua_touserdata(lua, 1);
    auto message = lua_tostring(lua, 2);
    auto type = argc < 3 ? chat::type::NORMAL : chat::type(lua_tointeger(lua, 3));
    auto decorate = argc < 4 ? true : lua_toboolean(lua, 4);

    std::stringstream sstream;
    if(decorate)
    {
        if(type == chat::type::SHOUT)
            sstream << object->name() << "! " << message;
        else
            sstream << object->name() << ": " << message;
    }
    else
    {
        sstream << message;
    }

    acceptor->send(*object, response::game::object::chat(*object, type, sstream.str()), acceptor::scope::PIVOT);
    return 0;
}

int fb::game::object::builtin_message(lua_State* lua)
{
    auto argc = lua_gettop(lua);
    auto acceptor = lua::env<fb::game::acceptor>("acceptor");
    auto object = *(fb::game::object**)lua_touserdata(lua, 1);
    auto message = lua_tostring(lua, 2);
    auto type = argc < 3 ? fb::game::message::STATE : lua_tointeger(lua, 3);

    if(object->type() == object::types::SESSION)
        acceptor->send(*object, response::game::message(message, fb::game::message::type(type)), acceptor::scope::SELF);

    return 0;
}

int fb::game::object::builtin_buff(lua_State* lua)
{
    auto acceptor = lua::env<fb::game::acceptor>("acceptor");
    auto object = *(fb::game::object**)lua_touserdata(lua, 1);
    auto spell = *(fb::game::spell**)lua_touserdata(lua, 2);
    auto time = (uint32_t)lua_tointeger(lua, 3);

    auto buff = object->buffs.push_back(spell, time);
    if(buff == nullptr)
        lua_pushnil(lua);
    else
        acceptor->send(*object, response::game::spell::buff(*buff), acceptor::scope::SELF);

    return 1;
}

int fb::game::object::builtin_unbuff(lua_State* lua)
{
    auto acceptor = lua::env<fb::game::acceptor>("acceptor");
    auto object = *(fb::game::object**)lua_touserdata(lua, 1);

    if(lua_isstring(lua, 2))
    {
        auto buff_name = lua_tostring(lua, 2);
        lua_pushboolean(lua, object->buffs.remove(buff_name));
    }
    else if(lua_isuserdata(lua, 2))
    {
        auto buff = *(fb::game::spell**)lua_touserdata(lua, 2);
        lua_pushboolean(lua, object->buffs.remove(buff->name()));
    }
    else
    {
        lua_pushboolean(lua, false);
    }

    return 1;
}

int fb::game::object::builtin_isbuff(lua_State* lua)
{
    auto object = *(fb::game::object**)lua_touserdata(lua, 1);
    if(lua_isstring(lua, 2))
    {
        auto buff_name = lua_tostring(lua, 2);
        lua_pushboolean(lua, object->buffs.contains(buff_name));
    }
    else if(lua_isuserdata(lua, 2))
    {
        auto buff = *(fb::game::spell**)lua_touserdata(lua, 2);
        lua_pushboolean(lua, object->buffs.contains(buff));
    }
    else
    {
        lua_pushboolean(lua, false);
    }
    return 1;
}

int fb::game::object::builtin_effect(lua_State* lua)
{
    auto acceptor = lua::env<fb::game::acceptor>("acceptor");
    auto object = *(fb::game::object**)lua_touserdata(lua, 1);
    auto effect = (uint8_t)lua_tointeger(lua, 2);

    if(object->type() != object::types::ITEM)
        acceptor->send(*object, response::game::object::effect(*object, effect), acceptor::scope::PIVOT);
    return 0;
}

int fb::game::object::builtin_map(lua_State* lua)
{
    try
    {
        auto argc = lua_gettop(lua);
        auto object = *(fb::game::object**)lua_touserdata(lua, 1);

        if(argc == 1)
        {
            auto map = object->map();
            if(map == nullptr)
                lua_pushnil(lua);
            else
                map->to_lua(lua);
            return 1;
        }

        fb::game::map* map = nullptr;
        if(lua_isuserdata(lua, 2))
        {
            map = *(fb::game::map**)lua_touserdata(lua, 2);
            if(map == nullptr)
                throw std::exception();
        }
        else if(lua_isstring(lua, 2))
        {
            game::master::get().name2map(lua_tostring(lua, 2));
            if(map == nullptr)
                throw std::exception();
        }
        else
        {
            throw std::exception();
        }

        point16_t position;
        if(lua_istable(lua, 3))
        {
            lua_rawgeti(lua, 3, 1);
            position.x = (uint16_t)lua_tointeger(lua, -1);
            lua_remove(lua, -1);

            lua_rawgeti(lua, 3, 2);
            position.y = (uint16_t)lua_tointeger(lua, -1);
            lua_remove(lua, -1);
        }
        else if(lua_isnumber(lua, 3) && lua_isnumber(lua, 4))
        {
            position.x = (uint16_t)lua_tointeger(lua, 3);
            position.y = (uint16_t)lua_tointeger(lua, 4);
        }
        else
        {
            throw std::exception();
        }

        map->objects.add(*object, position);
    }
    catch(...)
    { }

    return 0;
}

int fb::game::object::builtin_mkitem(lua_State* lua)
{
    auto object = *(fb::game::object**)lua_touserdata(lua, 1);
    auto name = lua_tostring(lua, 2);

    auto master = game::master::get().name2item(name);
    if(master == nullptr)
    {
        lua_pushnil(lua);
    }
    else
    {
        auto acceptor = lua::env<fb::game::acceptor>("acceptor");
        auto item = master->make(acceptor);
        object->_map->objects.add(*item, object->_position);
        item->to_lua(lua);
        
        acceptor->send(*item, response::game::object::show(*item), acceptor::scope::PIVOT);
    }

    return 1;
}

int fb::game::object::builtin_showings(lua_State* lua)
{
    auto argc = lua_gettop(lua);
    auto object = *(fb::game::object**)lua_touserdata(lua, 1);
    auto filter = argc < 2 ? object::types::UNKNOWN : object::types(lua_tointeger(lua, 2));

    lua_newtable(lua);
    const auto& objects = object->showings(filter);

    for(int i = 0; i < objects.size(); i++)
    {
        objects[i]->to_lua(lua);
        lua_rawseti(lua, -2, long long(i+1));
    }

    return 1;
}

int fb::game::object::builtin_showns(lua_State* lua)
{
    auto argc = lua_gettop(lua);
    auto object = *(fb::game::object**)lua_touserdata(lua, 1);
    auto filter = argc < 2 ? object::types::UNKNOWN : object::types(lua_tointeger(lua, 2));

    lua_newtable(lua);
    const auto& objects = object->showns(filter);

    for(int i = 0; i < objects.size(); i++)
    {
        objects[i]->to_lua(lua);
        lua_rawseti(lua, -2, long long(i+1));
    }

    return 1;
}

int fb::game::object::builtin_front(lua_State* lua)
{
    auto argc = lua_gettop(lua);
    auto object = *(fb::game::object**)lua_touserdata(lua, 1);
    auto filter = argc < 2 ? object::types::UNKNOWN : object::types(lua_tointeger(lua, 2));

    auto front = object->forward_object(filter);
    if(front == nullptr)
        lua_pushnil(lua);
    else
        front->to_lua(lua);

    return 1;
}

int fb::game::object::builtin_is(lua_State* lua)
{
    auto argc = lua_gettop(lua);
    auto object = *(fb::game::object**)lua_touserdata(lua, 1);
    auto type = lua_tointeger(lua, 2);

    lua_pushboolean(lua, object->is(object::types(type)));
    return 1;
}