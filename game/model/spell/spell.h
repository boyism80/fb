#ifndef __SPELL_H__
#define __SPELL_H__

#include <vector>
#include "model/mmo/mmo.h"
#include "model/container/container.h"
#include "model/lua/lua.h"
#include "module/stream/stream.h"

namespace fb { namespace game {

#pragma region forward declaration
class object;
class life;
#pragma endregion

class spell : public lua::luable
{
#pragma region lua
public:
    LUA_PROTOTYPE
#pragma endregion

#pragma region forward nested declaration
public:
    interface listener;
#pragma endregion

#pragma region static const field
public:
    static const uint32_t       MAX_SLOT = 52;
#pragma endregion

#pragma region enum
public:
    enum class types : uint8_t
    {
        INPUT   = 0x01, 
        TARGET  = 0x02,
        NORMAL  = 0x05, 
    };
#pragma endregion

#pragma region private field
private:
    uint16_t                    _id     = 0;
    fb::game::spell::types      _type   = fb::game::spell::types::NORMAL;
    std::string                 _name;
    std::string                 _cast, _uncast, _concast;
    std::string                 _message;
#pragma endregion

#pragma region constructor / destructor
public:
    spell(uint16_t id, fb::game::spell::types type, const std::string& name, const std::string& cast, const std::string& uncast, const std::string& concast, const std::string& message);
    ~spell();
#pragma endregion

#pragma region public method
public:
    uint16_t                    id() const;
    fb::game::spell::types      type() const;
    const std::string&          name() const;
    const std::string&          cast() const;
    const std::string&          uncast() const;
    const std::string&          concast() const;
    const std::string&          message() const;
#pragma endregion

#pragma region built-in method
public:
    static int                  builtin_type(lua_State* lua);
    static int                  builtin_name(lua_State* lua);
    static int                  builtin_message(lua_State* lua);
#pragma endregion
};


class spells : public fb::game::base_container<fb::game::spell>
{
public:
    spells(life& owner);
    ~spells();

    // override
public:
    uint8_t                     add(spell& element);
    uint8_t                     add(spell& element, uint8_t index);
    uint8_t                     add(spell* element);
    uint8_t                     add(spell* element, uint8_t index);
    bool                        remove(uint8_t index);
    bool                        swap(uint8_t src, uint8_t dst);
};


class buff
{
private:
    const fb::game::spell*      _spell  = nullptr;
    uint32_t                    _time   = 0;

public:
    buff(const spell* spell, uint32_t time);
    ~buff();

public:
    const fb::game::spell&      spell() const;
    uint32_t                    time() const;
    void                        time(uint32_t value);
    void                        time_inc(uint32_t inc);
    void                        time_dec(uint32_t dec);

public:
    operator                    const fb::game::spell& () const;
    operator                    const fb::game::spell* () const;
};


class buffs : private std::vector<buff*>
{
private:
    fb::game::object&           _owner;

public:
    using std::vector<buff*>::size;
    using std::vector<buff*>::begin;
    using std::vector<buff*>::end;
    using std::vector<buff*>::rbegin;
    using std::vector<buff*>::rend;
    using std::vector<buff*>::cbegin;
    using std::vector<buff*>::cend;
    using std::vector<buff*>::crbegin;
    using std::vector<buff*>::crend;
    using std::vector<buff*>::operator[];

public:
    buffs(fb::game::object& owner);
    ~buffs();

public:
    bool                        contains(const buff* buff) const;
    bool                        contains(const spell* spell) const;
    bool                        contains(const std::string& name) const;
    bool                        push_back(buff* buff);
    buff*                       push_back(const fb::game::spell* spell, uint32_t time);
    bool                        remove(const std::string& name);
    bool                        remove(const fb::game::spell* spell);
    void                        remove(buff* buff);

public:
    buff*                       operator [] (const std::string& name) const;
};



interface spell::listener
{
    virtual void on_spell_update(life& me, uint8_t index) = 0;
    virtual void on_spell_remove(life& me, uint8_t index) = 0;
};

} }

#endif // !__SPELL_H__
