#ifndef __SPELL_H__
#define __SPELL_H__

#include <vector>
#include <chrono>
#include <fb/game/mmo.h>
#include <fb/game/container.h>
#include <fb/game/lua.h>

namespace fb { namespace game {

class object;
class life;

class spell : public lua::luable
{
public:
    LUA_PROTOTYPE

public:
    interface listener;

public:
    enum class types : uint8_t
    {
        INPUT   = 0x01, 
        TARGET  = 0x02,
        NORMAL  = 0x05, 
    };

public:
    const uint16_t               id     = 0;
    const fb::game::spell::types type   = fb::game::spell::types::NORMAL;
    const std::string            name;
    const std::string            cast, uncast, concast;
    const std::string            message;

public:
    spell(uint16_t id, fb::game::spell::types type, const std::string& name, const std::string& cast, const std::string& uncast, const std::string& concast, const std::string& message);
    ~spell();

public:
    static int                  builtin_type(lua_State* lua);
    static int                  builtin_name(lua_State* lua);
    static int                  builtin_message(lua_State* lua);
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
    std::chrono::milliseconds   _time;

public:
    const fb::game::spell&      spell;

public:
    buff(const fb::game::spell& spell, uint32_t seconds);
    ~buff();

public:
    std::chrono::milliseconds   time() const;
    template <class _Rep, class _Period>
    void                        time(const std::chrono::duration<_Rep, _Period>& value) { this->_time = std::chrono::duration_cast<std::chrono::milliseconds>(value); }
    void                        time_inc(uint32_t inc);
    void                        time_dec(uint32_t dec);

public:
    operator                    const fb::game::spell& () const;
    operator                    const fb::game::spell* () const;
};


class buffs : private std::vector<std::unique_ptr<buff>>
{
private:
    fb::game::object&           _owner;

public:
    using std::vector<std::unique_ptr<buff>>::size;
    using std::vector<std::unique_ptr<buff>>::begin;
    using std::vector<std::unique_ptr<buff>>::end;
    using std::vector<std::unique_ptr<buff>>::rbegin;
    using std::vector<std::unique_ptr<buff>>::rend;
    using std::vector<std::unique_ptr<buff>>::cbegin;
    using std::vector<std::unique_ptr<buff>>::cend;
    using std::vector<std::unique_ptr<buff>>::crbegin;
    using std::vector<std::unique_ptr<buff>>::crend;

public:
    buffs(fb::game::object& owner);
    ~buffs();

private:
    bool                        push_back(std::unique_ptr<buff>&& buff);

public:
    bool                        contains(const buff* buff) const;
    bool                        contains(const spell* spell) const;
    bool                        contains(const std::string& name) const;
    buff*                       push_back(const fb::game::spell* spell, uint32_t seconds);
    bool                        remove(const std::string& name);
    bool                        remove(const fb::game::spell* spell);
    void                        remove(buff* buff);

public:
    buff*                       operator [] (int index) const;
    buff*                       operator [] (const std::string& name) const;
};



interface spell::listener
{
    virtual void on_spell_update(life& me, uint8_t index) = 0;
    virtual void on_spell_remove(life& me, uint8_t index) = 0;
};

} }

#endif // !__SPELL_H__
