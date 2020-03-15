#include "object.h"
#include "map.h"
#include "session.h"
#include "mob.h"
#include "fb_game.h"
#include "builtin_function.h"

IMPLEMENT_LUA_EXTENSION(fb::game::object::core, "fb.game.object.core")
{"name",        fb::game::object::core::builtin_name},
{"look",        fb::game::object::core::builtin_look},
{"color",       fb::game::object::core::builtin_color},
{"dialog",      fb::game::object::core::builtin_dialog},
END_LUA_EXTENSION


IMPLEMENT_LUA_EXTENSION(fb::game::object, "fb.game.object")
{"core",        fb::game::object::builtin_core},
{"__eq",        fb::game::object::builtin_eq},
{"__tostring",  fb::game::object::builtin_tostring},
{"id",          fb::game::object::builtin_id},
{"name",        fb::game::object::builtin_name},
{"dialog",      fb::game::object::builtin_dialog},
{"sound",       fb::game::object::builtin_sound},
{"position",    fb::game::object::builtin_position},
{"chat",        fb::game::object::builtin_chat},
{"message",     fb::game::object::builtin_message},
{"buff",        fb::game::object::builtin_buff},
{"effect",      fb::game::object::builtin_effect},
{"map",         fb::game::object::builtin_map},
END_LUA_EXTENSION


IMPLEMENT_LUA_EXTENSION(fb::game::life::core, "fb.game.life.core")
{"hp",          fb::game::life::core::builtin_hp},
{"mp",          fb::game::life::core::builtin_mp},
END_LUA_EXTENSION

IMPLEMENT_LUA_EXTENSION(fb::game::life, "fb.game.life")
{"__eq",        fb::game::object::builtin_eq},
{"hp",          fb::game::life::builtin_hp},
{"mp",          fb::game::life::builtin_mp},
{"hp_inc",      fb::game::life::builtin_hp_inc},
{"mp_inc",      fb::game::life::builtin_mp_inc},
{"hp_dec",      fb::game::life::builtin_hp_dec},
{"mp_dec",      fb::game::life::builtin_mp_dec},
{"base_hp",     fb::game::life::builtin_base_hp},
{"base_mp",     fb::game::life::builtin_base_mp},
{"action",      fb::game::life::builtin_action},
{"spell",       fb::game::life::builtin_spell},
END_LUA_EXTENSION

fb::game::object::core::core(const std::string& name, uint16_t look, uint8_t color) : 
    _name(name),
    _look(look),
    _color(color)
{
}

fb::game::object::core::~core()
{
}

uint8_t fb::game::object::core::dialog_look_type() const
{
    return this->look() > 0xBFFF ? 0x02 : 0x01;
}

fb::game::object::types fb::game::object::core::type() const
{
    return types::UNKNOWN;
}

const std::string& fb::game::object::core::name() const
{
    return this->_name;
}

void fb::game::object::core::name(const std::string& value)
{
    this->_name = value;
}

uint16_t fb::game::object::core::look() const
{
    return this->_look;
}

void fb::game::object::core::look(uint16_t value)
{
    this->_look = value;
}

uint8_t fb::game::object::core::color() const
{
    return this->_color;
}

void fb::game::object::core::color(uint8_t value)
{
    this->_color = value;
}

fb::ostream fb::game::object::core::make_dialog_stream(const std::string& message, bool button_prev, bool button_next, fb::game::map* map, dialog::interaction interaction) const
{
    fb::ostream             ostream;

    ostream.write_u8(0x30)
        .write_u8(0x00)     // unknown
        .write_u8(interaction)     // interaction
        .write_u32(map != nullptr ? map->id() : 0x01)
        .write_u8(this->dialog_look_type())
        .write_u8(0x01)
        .write_u16(this->look())
        .write_u8(this->color())
        .write_u8(this->dialog_look_type())
        .write_u16(this->look())
        .write_u8(this->color())
        .write_u32(0x01)
        .write_u8(button_prev)
        .write_u8(button_next)
        .write(message, true);

    return ostream;
}

int fb::game::object::core::builtin_name(lua_State* lua)
{
    auto object = *(fb::game::object::core**)lua_touserdata(lua, 1);

    lua_pushstring(lua, object->_name.c_str());
    return 1;
}

int fb::game::object::core::builtin_look(lua_State* lua)
{
    auto object = *(fb::game::object::core**)lua_touserdata(lua, 1);

    lua_pushinteger(lua, object->_look);
    return 1;
}

int fb::game::object::core::builtin_color(lua_State* lua)
{
    auto object = *(fb::game::object::core**)lua_touserdata(lua, 1);

    lua_pushinteger(lua, object->_color);
    return 1;
}

int fb::game::object::core::builtin_dialog(lua_State* lua)
{
    // Ex) npc:dialog(session, "hello", true, true);
    return ::builtin_dialog<object::core>(lua);
}



// fb::game::object
fb::game::object::object(const fb::game::object::core* core, uint32_t id, const point16_t position, fb::game::direction direction, fb::game::map* map) : 
    luable(id),
    _core(core),
    _position(position),
    _direction(direction),
    _map(map)
{
}

fb::game::object::object(const object& right) :
    object(right._core, right.id(), right._position, right._direction, right._map)
{
}

fb::game::object::~object()
{}

const fb::game::object::core* fb::game::object::based() const
{
    return this->_core;
}

const std::string& fb::game::object::name() const
{
    return this->_core->name();
}

uint16_t fb::game::object::look() const
{
    return this->_core->look();
}

uint8_t fb::game::object::color() const
{
    return this->_core->color();
}

fb::game::object::types fb::game::object::type() const
{
    return this->_core->type();
}

fb::game::point16_t fb::game::object::position() const
{
    return this->_position;
}

bool fb::game::object::position(uint16_t x, uint16_t y)
{
    if(this->_map == nullptr)
        return false;

    this->_before.x = this->_position.x;
    this->_before.y = this->_position.y;

    this->_position.x = std::max(0, std::min(this->_map->width() - 1, int32_t(x)));
    this->_position.y = std::max(0, std::min(this->_map->height() - 1, int32_t(y)));
    return true;
}

bool fb::game::object::position(const point16_t position)
{
    if(this->_map == nullptr)
        return false;

    this->_before.x = this->_position.x;
    this->_before.y = this->_position.y;

    this->_position.x = std::max(0, std::min(this->_map->width() - 1, int32_t(position.x)));
    this->_position.y = std::max(0, std::min(this->_map->height() - 1, int32_t(position.y)));
    return true;
}

fb::game::point16_t fb::game::object::position_forward() const
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
    if(this->_map == nullptr)
        return false;

    this->_position.x = std::max(uint16_t(0), std::min(uint16_t(this->_map->width() - 1), value));
    return true;
}

uint16_t fb::game::object::y() const
{
    return this->_position.y;
}

bool fb::game::object::y(uint16_t value)
{
    if(this->_map == nullptr)
        return false;

    this->_position.y = std::max(uint16_t(0), std::min(uint16_t(this->_map->height() - 1), value));
    return true;
}

fb::game::direction fb::game::object::direction() const
{
    return this->_direction;
}

bool fb::game::object::direction(fb::game::direction value)
{
    if(value != fb::game::direction::LEFT && value != fb::game::direction::TOP && value != fb::game::direction::RIGHT && value != fb::game::direction::BOTTOM)
        return false;

    this->_direction = value;
    return true;
}

fb::game::map* fb::game::object::map() const
{
    return this->_map;
}

uint16_t fb::game::object::map(fb::game::map* map)
{
    if(this->_map != nullptr)
        this->_map->object_delete(this);

    this->_map = map;
    return this->_map->object_add(this);
}

uint16_t fb::game::object::map(fb::game::map* map, const point16_t& position)
{
    uint16_t seq = this->map(map);
    this->position(position);

    return seq;
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

    for(auto session : map->sessions())
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

    for(auto session : map->sessions())
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

    for(auto object : map->objects())
    {
        if(type != fb::game::object::types::UNKNOWN && object->type() != type)
            continue;

        if(object->alive() == false)
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

    for(auto object : this->_map->objects())
    {
        if(type != fb::game::object::types::UNKNOWN && (object->type() & type) != object->type())
            continue;

        if(object->alive() == false)
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

    for(auto object : this->_map->objects())
    {
        if(object == this)
            continue;

        if(type != object::types::UNKNOWN && (object->type() & type) != object->type())
            continue;

        if(object->alive() == false)
            continue;

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

    for(auto object : this->_map->objects())
    {
        if(object == this)
            continue;

        if(type != object::types::UNKNOWN && (object->type() & type) != object->type())
            continue;

        if(object->sight(*this) == false)
            continue;

        list.push_back(object);
    }

    return list;
}

bool fb::game::object::alive() const
{
    return true;
}

double fb::game::object::distance(const object& right) const
{
    return std::sqrt(this->distance_sqrt(right));
}

uint32_t fb::game::object::distance_sqrt(const object& right) const
{
    return std::pow(this->_position.x - right._position.x, 2) + 
        std::pow(this->_position.y - right._position.y, 2);
}

fb::ostream fb::game::object::make_show_stream() const
{
    fb::ostream             ostream;

    ostream.write_u8(0x07)
        .write_u16(0x0001) // count
        .write_u16(this->x()) // object x
        .write_u16(this->y()) // object y
        .write_u32(this->id()) // object id
        .write_u16(this->look()) // npc icon code
        .write_u8(this->color())  // color
        .write_u8(this->direction()); // side

    std::cout << "show mob : " << this->id() << std::endl;

    return ostream;
}

fb::ostream fb::game::object::make_hide_stream() const
{
    fb::ostream             ostream;
    ostream.write_u8(0x0E)
        .write_u32(this->id())
        .write_u8(0x00);

    std::cout << "hide mob : " << this->id() << std::endl;

    return ostream;
}

fb::ostream fb::game::object::make_show_stream(const std::vector<fb::game::object*>& objects)
{
    fb::ostream             ostream;

    if(objects.size() == 0)
        return ostream;

    ostream.write_u8(0x07)
        .write_u16(objects.size());

    for(const auto object : objects)
    {
        ostream.write_u16(object->x()) // object x
            .write_u16(object->y()) // object y
            .write_u32(object->id()) // object id
            .write_u16(object->look()) // npc icon code
            .write_u8(object->color())  // color
            .write_u8(object->direction()); // side

        std::cout << "show mob : " << object->id() << std::endl;
    }

    return ostream;
}

fb::ostream fb::game::object::make_chat_stream(const std::string& message, chat::type type) const
{
    fb::ostream             ostream;

    ostream.write_u8(0x0D)
        .write_u8(type)
        .write_u32(this->id())
        .write(message);

    return ostream;
}

fb::ostream fb::game::object::make_sound_stream(fb::game::sound::type sound) const
{
    // 0x019A : 장비 해제 사운드
    // 0x019B : 장비 장착 사운드
    // 0x0006 : 소비아이템 사용 사운드

    fb::ostream             ostream;
    ostream.write_u8(0x19)
        .write_u8(0x00)
        .write_u8(0x03)
        .write_u16(sound) // sound
        .write_u8(100)
        .write_u16(0x0004)
        .write_u32(this->id())
        .write_u16(0x0100)
        .write_u16(0x0202)
        .write_u16(0x0004)
        .write_u16(0xCCCC);

    return ostream;
}

fb::ostream fb::game::object::make_effet_stream(uint8_t effect) const
{
    fb::ostream             ostream;

    ostream.write_u8(0x29)
        .write_u32(this->id())
        .write_u8(effect)
        .write_u8(0x00);

    return ostream;
}

fb::ostream fb::game::object::make_dialog_stream(const std::string& message, bool button_prev, bool button_next) const
{
    return this->_core->make_dialog_stream(message, button_prev, button_next, this->_map);
}

int fb::game::object::builtin_id(lua_State* lua)
{
    auto object = *(fb::game::object**)lua_touserdata(lua, 1);
    auto acceptor = lua::env<fb::game::acceptor>("acceptor");

    lua_pushinteger(lua, object->id());
    return 1;
}

int fb::game::object::builtin_eq(lua_State* lua)
{
    auto argc = lua_gettop(lua);
    auto me = *(fb::game::object**)lua_touserdata(lua, 1);
    auto you = *(fb::game::object**)lua_touserdata(lua, 2);
    
    lua_pushboolean(lua, me->id() == you->id());
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

    acceptor->send_stream(*object, object->make_sound_stream(sound::type(sound)), acceptor::scope::PIVOT);

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
    else if(lua_istable(lua, 2))
    {
        lua_rawgeti(lua, 2, 1);
        object->_position.x = lua_tointeger(lua, -1);
        lua_remove(lua, -1);

        lua_rawgeti(lua, 2, 2);
        object->_position.y = lua_tointeger(lua, -1);
        lua_remove(lua, -1);
        return 0;
    }
    else
    {
        object->_position.x = lua_tointeger(lua, 2);
        object->_position.y = lua_tointeger(lua, 3);
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

    acceptor->send_stream(*object, object->make_chat_stream(sstream.str(), type), acceptor::scope::PIVOT);
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
        acceptor->send_stream(*object, fb::game::message::make_stream(message, fb::game::message::type(type)), acceptor::scope::SELF);

    return 0;
}

int fb::game::object::builtin_buff(lua_State* lua)
{
    auto acceptor = lua::env<fb::game::acceptor>("acceptor");
    auto object = *(fb::game::object**)lua_touserdata(lua, 1);
    auto message = lua_tostring(lua, 2);
    auto time = lua_tointeger(lua, 3);

    if(object->type() == object::types::SESSION)
        acceptor->send_stream(*object, spell::make_buff_stream(message, time), acceptor::scope::SELF);

    return 0;
}

int fb::game::object::builtin_effect(lua_State* lua)
{
    auto acceptor = lua::env<fb::game::acceptor>("acceptor");
    auto object = *(fb::game::object**)lua_touserdata(lua, 1);
    auto effect = lua_tointeger(lua, 2);

    if(object->type() != object::types::ITEM)
        acceptor->send_stream(*object, object->make_effet_stream(effect), acceptor::scope::PIVOT);
    return 0;
}

int fb::game::object::builtin_map(lua_State* lua)
{
    auto argc = lua_gettop(lua);
    auto acceptor = lua::env<fb::game::acceptor>("acceptor");
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
    else
    {
        auto map = *(fb::game::map**)lua_touserdata(lua, 2);
        object->map(map);
        return 0;
    }
}

fb::game::life::core::core(const std::string& name, uint16_t look, uint8_t color, uint32_t hp, uint32_t mp) : 
    object::core(name, look, color),
    _hp(hp), _mp(mp),
    _experience(0)
{}


fb::game::life::core::core(const core& core, uint32_t hp, uint32_t mp) : 
    fb::game::object::core(core),
    _hp(hp), _mp(mp),
    _experience(0)
{
}

fb::game::life::core::core(const core& right) : 
    fb::game::object::core(right),
    _hp(right._hp), _mp(right._mp),
    _experience(right._experience)
{
}

fb::game::life::core::~core()
{
}

uint32_t fb::game::life::core::hp() const
{
    return this->_hp;
}

void fb::game::life::core::hp(uint32_t value)
{
    this->_hp = value;
}

uint32_t fb::game::life::core::mp() const
{
    return this->_mp;
}

void fb::game::life::core::mp(uint32_t value)
{
    this->_mp = value;
}

uint32_t fb::game::life::core::experience() const
{
    return this->_experience;
}

void fb::game::life::core::experience(uint32_t value)
{
    this->_experience = value;
}

uint32_t fb::game::life::core::defensive_physical() const
{
    return this->_defensive.physical;
}

void fb::game::life::core::defensive_physical(uint8_t value)
{
    this->_defensive.physical = value;
}

uint32_t fb::game::life::core::defensive_magical() const
{
    return this->_defensive.magical;
}

void fb::game::life::core::defensive_magical(uint8_t value)
{
    this->_defensive.magical = value;
}

fb::game::object* fb::game::life::core::make() const
{
    return new life(this);
}

int fb::game::life::core::builtin_hp(lua_State* lua)
{
    auto object = *(fb::game::life::core**)lua_touserdata(lua, 1);

    lua_pushinteger(lua, object->_hp);
    return 1;
}

int fb::game::life::core::builtin_mp(lua_State* lua)
{
    auto object = *(fb::game::life::core**)lua_touserdata(lua, 1);

    lua_pushinteger(lua, object->_mp);
    return 1;
}


fb::game::life::life(const core* core) : 
    object(core),
    _hp(0),
    _mp(0),
    _condition(fb::game::condition::NONE),
    spells(*this)
{
}

fb::game::life::life(core* core, uint32_t id, uint32_t hp, uint32_t mp, uint32_t exp) : 
    object(core, id),
    _hp(hp),
    _mp(mp),
    _condition(fb::game::condition::NONE),
    spells(*this)
{
}

fb::game::life::life(const fb::game::object& object, uint32_t hp, uint32_t mp, uint32_t exp) : 
    object(object),
    _hp(hp),
    _mp(mp),
    _condition(fb::game::condition::NONE),
    spells(*this)
{
}


fb::game::life::~life()
{
}

uint32_t fb::game::life::random_damage(uint32_t value, const fb::game::life& life) const
{
    uint32_t Xrate = life.direction() == this->direction() ? 2 : 1;
    uint32_t n = (100 - life.defensive_physical()) / 10;
    float defensive_percent = -125 + (n * (2*14.75f - (n-1)/2.0f))/2.0f;
    uint32_t random_damage = value - (defensive_percent * (value/100.0f));

    return random_damage * Xrate;
}

bool fb::game::life::movable(fb::game::direction direction) const
{
    if(this->_map == nullptr)
        return false;

    return this->_map->movable(*this, direction);
}

bool fb::game::life::movable_forward() const
{
    if(this->_map == nullptr)
        return false;

    return this->_map->movable_forward(*this);
}

bool fb::game::life::move(fb::game::direction direction, std::vector<object*>* shows, std::vector<object*>* hides, std::vector<object*>* showns, std::vector<object*>* hiddens)
{
    if(this->movable(direction) == false)
        return false;

    const auto before = this->_position;

    switch(direction)
    {
    case fb::game::direction::TOP:
        this->_position.y--;
        break;

    case fb::game::direction::BOTTOM:
        this->_position.y++;
        break;

    case fb::game::direction::LEFT:
        this->_position.x--;
        break;

    case fb::game::direction::RIGHT:
        this->_position.x++;
        break;
    }

    const auto& after = this->_position;

    // 내 기준으로 한 오브젝트
    if(shows != nullptr || hides != nullptr)
    {
        for(auto object : this->_map->objects())
        {
            if(object == this)
                continue;

            bool                before = this->sight(*object, true);
            bool                after  = this->sight(*object);

            bool                show = (!before && after);
            bool                hide = (before && !after);

            if(show && shows != nullptr)
                shows->push_back(object);
            else if(hide && hides != nullptr)
                hides->push_back(object);
        }
    }

    // 상대 기준으로 한 오브젝트
    if(showns != nullptr || hiddens)
    {
        for(auto object : this->_map->objects())
        {
            if(object == this)
                continue;

            bool                before_sight = object->sight(*this, false, true);
            bool                after_sight  = object->sight(*this);

            bool                show = (!before_sight && after_sight);
            bool                hide = (before_sight && !after_sight);

            if(show && showns != nullptr)
                showns->push_back(object);
            else if(hide && hiddens != nullptr)
                hiddens->push_back(object);
        }
    }

    this->_before = before;
    return true;
}

bool fb::game::life::move_forward(std::vector<object*>* shows, std::vector<object*>* hides, std::vector<object*>* showns, std::vector<object*>* hiddens)
{
    return this->move(this->_direction, shows, hides, showns, hiddens);
}

void fb::game::life::hp_up(uint32_t value)
{
    uint32_t capacity = 0xFFFFFFFF - this->base_hp();
    this->_hp += std::min(capacity, value);
}

void fb::game::life::hp_down(uint32_t value)
{
    this->_hp -= std::min(value, this->_hp);
}

void fb::game::life::mp_up(uint32_t value)
{
    uint32_t capacity = 0xFFFFFFFF - this->base_mp();
    this->_mp += std::min(capacity, value);
}

void fb::game::life::mp_down(uint32_t value)
{
    this->_mp -= std::min(value, this->_mp);
}

uint32_t fb::game::life::hp() const
{
    return this->_hp;
}

void fb::game::life::hp(uint32_t value)
{
    this->_hp = value;
}

void fb::game::life::heal(uint32_t value)
{
    uint32_t space = this->base_hp() - this->_hp;
    this->_hp += std::min(value, space);
}

uint32_t fb::game::life::mp() const
{
    return this->_mp;
}

void fb::game::life::mp(uint32_t value)
{
    this->_mp = value;
}

uint32_t fb::game::life::base_hp() const
{
    return static_cast<const core*>(this->_core)->_hp;
}

uint32_t fb::game::life::base_mp() const
{
    return static_cast<const core*>(this->_core)->_mp;
}

uint32_t fb::game::life::experience() const
{
    return static_cast<const core*>(this->_core)->_experience;
}

uint32_t fb::game::life::defensive_physical() const
{
    return static_cast<const core*>(this->_core)->_defensive.physical;
}

uint32_t fb::game::life::defensive_magical() const
{
    return static_cast<const core*>(this->_core)->_defensive.magical;
}

fb::game::condition fb::game::life::condition() const
{
    return this->_condition;
}

fb::game::condition fb::game::life::condition_add(fb::game::condition value)
{
    this->_condition = fb::game::condition(this->_condition | value);
    return this->_condition;
}

fb::game::condition fb::game::life::condition_remove(fb::game::condition value)
{
    this->_condition = fb::game::condition(this->_condition & ~value);
    return this->_condition;
}

bool fb::game::life::condition_contains(fb::game::condition value) const
{
    return uint32_t(this->_condition) & uint32_t(value);
}

bool fb::game::life::alive() const
{
    return this->_hp != 0;
}

void fb::game::life::kill()
{
    this->_hp = 0;
}

fb::ostream fb::game::life::make_move_stream(bool from_before) const
{
    return this->make_move_stream(this->_direction, from_before);
}

fb::ostream fb::game::life::make_move_stream(fb::game::direction direction, bool from_before) const
{
    fb::ostream             ostream;

    ostream.write_u8(0x0C)
        .write_u32(this->id())
        .write_u16(from_before ? this->_before.x : this->_position.x)
        .write_u16(from_before ? this->_before.y : this->_position.y)
        .write_u8(direction)
        .write_u8(0x00);

    return ostream;
}

fb::ostream fb::game::life::make_direction_stream() const
{
    fb::ostream             ostream;
    ostream.write_u8(0x11)
        .write_u32(this->id())
        .write_u8(this->_direction)
        .write_u8(0x00);

    return ostream;
}

fb::ostream fb::game::life::make_action_stream(fb::game::action action, fb::game::duration duration) const
{
    uint8_t sound = 0x00;

    fb::ostream             ostream;
    ostream.write_u8(0x1A)
        .write_u32(this->id())
        .write_u8(action) // type
        .write_u16(duration) // duration
        .write_u8(sound); // sound

    return ostream;
}

fb::ostream fb::game::life::make_show_hp_stream(uint32_t random_damage, bool critical) const
{
    fb::ostream             ostream;
    uint8_t                 percentage = uint8_t((this->_hp / float(this->base_hp())) * 100);

    ostream.write_u8(0x13)
        .write_u32(this->id())
        .write_u8(critical)
        .write_u8(percentage)
        .write_u32(random_damage)
        .write_u8(0x00);

    return ostream;
}

fb::ostream fb::game::life::make_die_stream() const
{
    fb::ostream             ostream;

    ostream.write_u8(0x5F)
        .write_u32(this->id())
        .write_u8(0x00);

    return ostream;
}

int fb::game::life::builtin_hp(lua_State* lua)
{
    auto argc = lua_gettop(lua);
    auto object = *(fb::game::life**)lua_touserdata(lua, 1);

    if(argc == 1)
    {
        lua_pushinteger(lua, object->hp());
        return 1;
    }
    else
    {
        auto value = lua_tointeger(lua, 2);
        object->hp(value);
        return 0;
    }
}

int fb::game::life::builtin_mp(lua_State* lua)
{
    auto argc = lua_gettop(lua);
    auto object = *(fb::game::life**)lua_touserdata(lua, 1);

    if(argc == 1)
    {
        lua_pushinteger(lua, object->mp());
        return 1;
    }
    else
    {
        auto value = lua_tointeger(lua, 2);
        object->mp(value);
        return 0;
    }
}

int fb::game::life::builtin_base_hp(lua_State* lua)
{
    auto object = *(fb::game::life**)lua_touserdata(lua, 1);
    auto core = object->based<life::core>();

    lua_pushinteger(lua, core->_hp);
    return 1;
}

int fb::game::life::builtin_base_mp(lua_State* lua)
{
    auto object = *(fb::game::life**)lua_touserdata(lua, 1);
    auto core = object->based<life::core>();

    lua_pushinteger(lua, core->_mp);
    return 1;
}

int fb::game::life::builtin_hp_inc(lua_State* lua)
{
    auto argc = lua_gettop(lua);
    auto object = *(fb::game::life**)lua_touserdata(lua, 1);
    auto value = lua_tointeger(lua, 2);

    object->hp_up(value);
    return 0;
}

int fb::game::life::builtin_hp_dec(lua_State* lua)
{
    auto argc = lua_gettop(lua);
    auto object = *(fb::game::life**)lua_touserdata(lua, 1);
    auto value = lua_tointeger(lua, 2);

    object->hp_down(value);
    return 0;
}

int fb::game::life::builtin_mp_inc(lua_State* lua)
{
    auto argc = lua_gettop(lua);
    auto object = *(fb::game::life**)lua_touserdata(lua, 1);
    auto value = lua_tointeger(lua, 2);

    object->mp_up(value);
    return 0;
}

int fb::game::life::builtin_mp_dec(lua_State* lua)
{
    auto argc = lua_gettop(lua);
    auto object = *(fb::game::life**)lua_touserdata(lua, 1);
    auto value = lua_tointeger(lua, 2);

    object->mp_down(value);
    return 0;
}

int fb::game::life::builtin_action(lua_State* lua)
{
    auto argc = lua_gettop(lua);
    auto acceptor = lua::env<fb::game::acceptor>("acceptor");
    auto life = *(fb::game::life**)lua_touserdata(lua, 1);
    auto action = lua_tointeger(lua, 2);
    auto duration = argc < 3 ? fb::game::duration::DURATION_SPELL : lua_tointeger(lua, 3);

    acceptor->send_stream(*life, life->make_action_stream(fb::game::action(action), fb::game::duration(duration)), acceptor::scope::SELF);
    return 0;
}

int fb::game::life::builtin_spell(lua_State* lua)
{
    auto argc = lua_gettop(lua);
    auto acceptor = lua::env<fb::game::acceptor>("acceptor");
    auto life = *(fb::game::life**)lua_touserdata(lua, 1);
    auto index = lua_tointeger(lua, 2);

    life->spells[index]->to_lua(lua);
    return 1;
}