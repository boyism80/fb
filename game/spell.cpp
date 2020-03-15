#include "spell.h"
#include "fb_game.h"

IMPLEMENT_LUA_EXTENSION(fb::game::spell, "fb.game.spell")
{"type",        fb::game::spell::builtin_type},
{"name",        fb::game::spell::builtin_name},
{"message",     fb::game::spell::builtin_message},
END_LUA_EXTENSION

fb::game::spell::spell(types type, const std::string& name, const std::string& cast, const std::string& uncast, const std::string& concast, const std::string& message) : 
    _type(type),
    _name(name),
    _cast(cast),
    _uncast(uncast),
    _concast(concast),
    _message(message)
{
}

fb::game::spell::~spell()
{
}

fb::game::spell::types fb::game::spell::type() const
{
    return this->_type;
}

const std::string& fb::game::spell::name() const
{
    return this->_name;
}

const std::string& fb::game::spell::cast() const
{
    return this->_cast;
}

const std::string& fb::game::spell::uncast() const
{
    return this->_uncast;
}

const std::string& fb::game::spell::concast() const
{
    return this->_concast;
}

const std::string& fb::game::spell::message() const
{
    return this->_message;
}

fb::ostream fb::game::spell::make_show_stream(uint8_t slot) const
{
    ostream                 ostream;

    ostream.write_u8(0x17)
        .write_u8(slot) // one-based
        .write_u8(this->_type)
        .write(this->_name);

    if(this->_type < 3)
        ostream.write(this->_message);

    return ostream;
}

fb::ostream fb::game::spell::make_delete_stream(uint8_t slot)
{
    ostream                 ostream;
    
    ostream.write_u8(0x18)
        .write_u8(slot) // one-based
        .write_u8(0x00);

    return ostream;
}

fb::ostream fb::game::spell::make_buff_stream(const std::string& message, uint32_t time)
{
    fb::ostream             ostream;

    ostream.write_u8(0x3A)
        .write(message)
        .write_u32(time);

    return ostream;
}

int fb::game::spell::builtin_type(lua_State* lua)
{
    auto acceptor = lua::env<fb::game::acceptor>("acceptor");
    auto spell = *(fb::game::spell**)lua_touserdata(lua, 1);

    lua_pushinteger(lua, spell->type());
    return 1;
}

int fb::game::spell::builtin_name(lua_State* lua)
{
    auto acceptor = lua::env<fb::game::acceptor>("acceptor");
    auto spell = *(fb::game::spell**)lua_touserdata(lua, 1);

    lua_pushstring(lua, spell->_name.c_str());
    return 1;
}

int fb::game::spell::builtin_message(lua_State* lua)
{
    auto acceptor = lua::env<fb::game::acceptor>("acceptor");
    auto spell = *(fb::game::spell**)lua_touserdata(lua, 1);

    lua_pushstring(lua, spell->_message.c_str());
    return 1;
}

fb::game::spells::spells(session& owner) : 
	container(owner, spell::MAX_SLOT, false)
{
}

fb::game::spells::~spells()
{
}
