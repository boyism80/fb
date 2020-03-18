#ifndef __SPELL_H__
#define __SPELL_H__

#include "mmo.h"
#include "stream.h"
#include "container.h"
#include "lua.h"

namespace fb { namespace game {

class spell : public lua::luable
{
#pragma region lua
public:
    LUA_PROTOTYPE
#pragma endregion


#pragma region static const field
public:
    static const uint32_t   MAX_SLOT = 52;
#pragma endregion


#pragma region enum
public:
    enum types : uint8_t
    {
        INPUT   = 0x01, 
        TARGET  = 0x02,
        NORMAL  = 0x05, 
    };
#pragma endregion


#pragma region private field
private:
    types                       _type;
    std::string                 _name;
    std::string                 _cast, _uncast, _concast;
    std::string                 _message;
#pragma endregion


#pragma region constructor / destructor
public:
    spell(types type, const std::string& name, const std::string& cast, const std::string& uncast, const std::string& concast, const std::string& message);
    ~spell();
#pragma endregion


#pragma region public method
public:
    types                       type() const;
    const std::string&          name() const;
    const std::string&          cast() const;
    const std::string&          uncast() const;
    const std::string&          concast() const;
    const std::string&          message() const;
#pragma endregion


#pragma region make stream method
public:
    static fb::ostream          make_buff_stream(const std::string& message, uint32_t time);
#pragma endregion

#pragma region built-in method
public:
    static int                  builtin_type(lua_State* lua);
    static int                  builtin_name(lua_State* lua);
    static int                  builtin_message(lua_State* lua);
#pragma endregion
};


class spells : public container<fb::game::spell>
{
public:
	spells(life& owner);
	~spells();

    fb::ostream                 make_update_stream(uint8_t index) const;
    fb::ostream                 make_delete_stream(uint8_t index) const;
};

} }

#endif // !__SPELL_H__
