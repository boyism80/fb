#include "model/object/object.h"
#include "model/map/map.h"
#include "model/session/session.h"
#include "model/mob/mob.h"
#include "model/acceptor/acceptor.game.h"
#include "model/listener/listener.h"
#include "builtin/builtin_function.h"

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

    lua_push_utf8(lua, object->_name.c_str());
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
    _visible(true),
    _sector(nullptr)
{
}

fb::game::object::object(const object& right) :
    object(right._master, right._listener, right.sequence(), right._position, right._direction, right._map)
{
}

fb::game::object::~object()
{
    this->map(nullptr);
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

bool fb::game::object::position(uint16_t x, uint16_t y, bool force)
{
    if(this->_map == nullptr)
        return false;

    if(this->_position.x == x && this->_position.y == y)
        return true;

    this->_before.x = this->_position.x;
    this->_before.y = this->_position.y;

    this->_position.x = std::max(0, std::min(this->_map->width() - 1, int32_t(x)));
    this->_position.y = std::max(0, std::min(this->_map->height() - 1, int32_t(y)));

    if(force && this->is(fb::game::object::types::SESSION))
        this->_listener->on_hold(static_cast<fb::game::session&>(*this));

    this->_map->update(*this);
    {
        auto befores = fb::game::object::showings(this->_map->nears(this->_before), *this, fb::game::object::UNKNOWN, true);
        std::sort(befores.begin(), befores.end());

        auto afters = fb::game::object::showings(this->_map->nears(this->_position), *this, fb::game::object::UNKNOWN, false);
        std::sort(afters.begin(), afters.end());


        auto hides = std::vector<fb::game::object*>();
        std::set_difference(befores.begin(), befores.end(), afters.begin(), afters.end(), std::inserter(hides, hides.begin()));
        for(auto x : hides)
            this->_listener->on_hide(*this, *x);

        auto shows = std::vector<fb::game::object*>();
        std::set_difference(afters.begin(), afters.end(), befores.begin(), befores.end(), std::inserter(shows, shows.begin()));
        for(auto x : shows)
            this->_listener->on_show(*this, *x, false);
    }

    {
        auto befores = fb::game::object::showns(this->_map->nears(this->_before), *this, fb::game::object::UNKNOWN, true);
        std::sort(befores.begin(), befores.end());

        auto afters = fb::game::object::showns(this->_map->nears(this->_position), *this, fb::game::object::UNKNOWN, false);
        std::sort(afters.begin(), afters.end());


        auto hides = std::vector<fb::game::object*>();
        std::set_difference(befores.begin(), befores.end(), afters.begin(), afters.end(), std::inserter(hides, hides.begin()));
        for(auto x : hides)
            this->_listener->on_hide(*x, *this);

        auto shows = std::vector<fb::game::object*>();
        std::set_difference(afters.begin(), afters.end(), befores.begin(), befores.end(), std::inserter(shows, shows.begin()));
        for(auto x : shows)
            this->_listener->on_show(*x, *this, false);
    }

    return true;
}

bool fb::game::object::position(const point16_t position, bool force)
{
    return this->position(position.x, position.y, force);
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

const fb::game::point16_t fb::game::object::position_forward() const
{
    auto forward = point16_t(this->_position);
    forward.forward(this->_direction);
    if(this->_map->movable(forward))
        return forward;


    auto left = point16_t(this->_position);
    left.left(this->_direction);
    if(this->_map->movable(left))
        return left;


    auto right = point16_t(this->_position);
    right.right(this->_direction);
    if(this->_map->movable(right))
        return right;

    auto backward = point16_t(this->_position);
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

bool fb::game::object::sector(fb::game::sector* sector)
{
    if(this->_sector == sector)
        return false;

    if(this->_sector != nullptr)
        this->_sector->erase(*this);

    this->_sector = sector;
    if(sector != nullptr)
        sector->push(*this);
    return true;
}

fb::game::sector* fb::game::object::sector()
{
    return this->_sector;
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
    if(map != nullptr && this->_map == map)
    {
        this->position(position, true);
    }
    //else if(this->_map != nullptr && map != nullptr && this->_map->host_id() != map->host_id())
    //{
    //    auto nears = this->_map->nears(this->_position);
    //    for(auto x : nears)
    //    {
    //        if(x == this)
    //            continue;

    //        this->_listener->on_hide(*x, *this);
    //    }

    //    this->_map->objects.remove(*this);

    //    this->_map = map;
    //    this->_position = position;
    //    this->_listener->on_warp(static_cast<fb::game::session&>(*this), *map, position);
    //}
    else
    {
        auto sector = this->_sector;

        if(this->_map != nullptr)
        {
            auto nears = this->_map->nears(this->_position);
            for(auto x : nears)
            {
                if(x == this)
                    continue;

                this->_listener->on_hide(*this, *x);
                this->_listener->on_hide(*x, *this);
            }

            this->_map->objects.remove(*this);
        }

        this->_map = map;
        this->_position = position;
        if(this->_map == nullptr)
            this->sector(nullptr);
        else
            map->update(*this);

        if(map != nullptr)
        {
            map->objects.add(*this);
            
            if(this->is(fb::game::object::types::SESSION))
                this->_listener->on_warp(static_cast<fb::game::session&>(*this));

            auto nears = this->_map->nears(this->_position);
            for(auto x : nears)
            {
                if(x == this)
                    continue;

                this->_listener->on_show(*this, *x, false);
                this->_listener->on_show(*x, *this, false);
            }
        }
    }
}

void fb::game::object::map(fb::game::map* map)
{
    this->map(map, point16_t(0, 0));
}

fb::game::object* fb::game::object::side(fb::game::direction direction, fb::game::object::types type) const
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

    auto nears = map->nears(this->_position, type);
    auto found = std::find_if
    (
        nears.begin(), nears.end(), 
        [&front] (fb::game::object* x)
        {
            return x->position() == front;
        }
    );

    return found != nears.end() ?
        *found : nullptr;
}

std::vector<fb::game::object*> fb::game::object::sides(fb::game::direction direction, fb::game::object::types type) const
{
    auto result = std::vector<fb::game::object*>();
    try
    {
        auto map = this->_map;
        if(map == nullptr)
            throw std::exception();

        auto front = this->position();
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
            throw std::exception();

        auto nears = map->nears(this->_position, type);
        std::copy_if
        (
            nears.begin(), nears.end(), std::back_inserter(result),
            [&front] (fb::game::object* x)
            {
                return x->position() == front;
            }
        );
    }
    catch(std::exception&)
    {}

    return result;
}

fb::game::object* fb::game::object::forward(fb::game::object::types type) const
{
    return this->side(this->_direction, type);
}

std::vector<fb::game::object*> fb::game::object::forwards(fb::game::object::types type) const
{
    return this->sides(this->_direction, type);
}

std::vector<fb::game::object*> fb::game::object::showings(object::types type) const
{
    if(this->_map == nullptr)
        return std::vector<fb::game::object*>();
    else
        return fb::game::object::showings(this->_map->nears(this->_position), *this, type);
}

std::vector<fb::game::object*> fb::game::object::showings(const std::vector<object*>& source, const fb::game::object& pivot, object::types type, bool before)
{
    auto                    objects = std::vector<fb::game::object*>();
    std::copy_if
    (
        source.begin(), source.end(), std::back_inserter(objects), 
        [&pivot, type, before](fb::game::object* x)
        {
            return
                *x != pivot &&
                (type == object::types::UNKNOWN || x->is(type)) &&
                pivot.sight(*x, before);
        }
    );

    return objects;
}

std::vector<object*> fb::game::object::showns(object::types type) const
{
    if(this->_map == nullptr)
        return std::vector<object*>();
    else
        return fb::game::object::showns(this->_map->nears(this->_position), *this, type);
}

std::vector<object*> fb::game::object::showns(const std::vector<object*>& source, const fb::game::object& pivot, object::types type, bool before)
{
    auto                    objects = std::vector<fb::game::object*>();
    
    std::copy_if
    (
        source.begin(), source.end(), std::back_inserter(objects), 
        [&pivot, type, before](fb::game::object* x)
        {
            return
                *x != pivot &&
                (type == object::types::UNKNOWN || x->is(type)) &&
                x->sight(pivot, before);
        }
    );

    return objects;
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

    lua_push_utf8(lua, me->name().c_str());
    return 1;
}

int fb::game::object::builtin_name(lua_State* lua)
{
    auto object = *(fb::game::object**)lua_touserdata(lua, 1);
    auto acceptor = lua::env<fb::game::acceptor>("acceptor");

    lua_push_utf8(lua, object->name().c_str());
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

    acceptor->send(*object, fb::protocol::game::response::object::sound(*object, sound::type(sound)), acceptor::scope::PIVOT);
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

    std::vector<fb::game::object*> shows, hides, showns, hiddens;
    object->position(x, y);

    auto acceptor = lua::env<fb::game::acceptor>("acceptor");
    if(object->is(fb::game::object::types::SESSION))
        acceptor->send(*object, fb::protocol::game::response::session::show(static_cast<fb::game::session&>(*object)), acceptor::scope::PIVOT);
    else
        acceptor->send(*object, fb::protocol::game::response::object::show(*object), acceptor::scope::PIVOT);

    if(object->is(object::types::SESSION))
    {
        auto session = static_cast<fb::game::session*>(object);
        acceptor->send(*object, fb::protocol::game::response::session::position(*session), acceptor::scope::SELF);
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
        auto direction = fb::game::direction(lua_tointeger(lua, 2));
        object->direction(direction);

        auto acceptor = lua::env<fb::game::acceptor>("acceptor");
        acceptor->send(*object, fb::protocol::game::response::object::direction(*object), acceptor::scope::PIVOT);
        return 0;
    }
}

int fb::game::object::builtin_chat(lua_State* lua)
{
    auto argc = lua_gettop(lua);
    auto acceptor = lua::env<fb::game::acceptor>("acceptor");
    auto object = *(fb::game::object**)lua_touserdata(lua, 1);
    auto message = lua_cp949(lua, 2);
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

    acceptor->send(*object, fb::protocol::game::response::object::chat(*object, type, sstream.str()), acceptor::scope::PIVOT);
    return 0;
}

int fb::game::object::builtin_message(lua_State* lua)
{
    auto argc = lua_gettop(lua);
    auto acceptor = lua::env<fb::game::acceptor>("acceptor");
    auto object = *(fb::game::object**)lua_touserdata(lua, 1);
    auto message = lua_cp949(lua, 2);
    auto type = argc < 3 ? fb::game::message::STATE : lua_tointeger(lua, 3);

    if(object->type() == object::types::SESSION)
        acceptor->send(*object, fb::protocol::game::response::message(message, fb::game::message::type(type)), acceptor::scope::SELF);

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
        acceptor->send(*object, fb::protocol::game::response::spell::buff(*buff), acceptor::scope::SELF);

    return 1;
}

int fb::game::object::builtin_unbuff(lua_State* lua)
{
    auto acceptor = lua::env<fb::game::acceptor>("acceptor");
    auto object = *(fb::game::object**)lua_touserdata(lua, 1);

    if(lua_isstring(lua, 2))
    {
        auto buff_name = lua_cp949(lua, 2);
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
        auto buff_name = lua_cp949(lua, 2);
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
        acceptor->send(*object, fb::protocol::game::response::object::effect(*object, effect), acceptor::scope::PIVOT);
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
            fb::game::table::maps.name2map(lua_cp949(lua, 2));
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

        object->map(map, position);
    }
    catch(...)
    { }

    return 0;
}

int fb::game::object::builtin_mkitem(lua_State* lua)
{
    auto object = *(fb::game::object**)lua_touserdata(lua, 1);
    auto name = lua_cp949(lua, 2);

    auto master = fb::game::table::items.name2item(name);
    if(master == nullptr)
    {
        lua_pushnil(lua);
    }
    else
    {
        auto acceptor = lua::env<fb::game::acceptor>("acceptor");
        auto item = master->make(acceptor);
        item->map(object->_map, object->_position);
        item->to_lua(lua);
        
        acceptor->send(*item, fb::protocol::game::response::object::show(*item), acceptor::scope::PIVOT);
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
        lua_rawseti(lua, -2, uint64_t(i+1));
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
        lua_rawseti(lua, -2, uint64_t(i+1));
    }

    return 1;
}

int fb::game::object::builtin_front(lua_State* lua)
{
    auto argc = lua_gettop(lua);
    auto object = *(fb::game::object**)lua_touserdata(lua, 1);
    auto filter = argc < 2 ? object::types::UNKNOWN : object::types(lua_tointeger(lua, 2));

    auto front = object->forward(filter);
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