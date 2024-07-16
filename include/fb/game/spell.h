#ifndef __SPELL_H__
#define __SPELL_H__

#include <vector>
#include <chrono>
#include <fb/game/mmo.h>
#include <fb/game/inventory.h>
#include <fb/game/model.h>

namespace fb { namespace game {

class object;
class life;

class spells : public fb::game::inventory<const fb::model::spell>
{
public:
    interface listener;

public:
    spells(life& owner);
    ~spells();

    // override
public:
    fb::task<uint8_t>           add(const fb::model::spell& element);
    fb::task<uint8_t>           add(const fb::model::spell& element, uint8_t index);
    bool                        remove(uint8_t index);
    bool                        swap(uint8_t src, uint8_t dst);
};

interface spells::listener
{
    virtual void on_spell_update(life& me, uint8_t index) = 0;
    virtual void on_spell_remove(life& me, uint8_t index) = 0;
};


class buff
{
private:
    std::chrono::milliseconds   _time;

public:
    const fb::model::spell&      model;
    const fb::game::context&     context;

public:
    buff(const fb::game::context& context, const fb::model::spell& model, uint32_t seconds);
    ~buff();

public:
    std::chrono::milliseconds   time() const;
    template <class _Rep, class _Period>
    void                        time(const std::chrono::duration<_Rep, _Period>& value) { this->_time = std::chrono::duration_cast<std::chrono::milliseconds>(value); }
    void                        time_inc(uint32_t inc);
    void                        time_dec(uint32_t dec);
};


class buffs : private std::unordered_map<uint32_t, buff*>
{
private:
    fb::game::object&           _owner;

public:
    using std::unordered_map<uint32_t, buff*>::size;
    using std::unordered_map<uint32_t, buff*>::begin;
    using std::unordered_map<uint32_t, buff*>::end;
    using std::unordered_map<uint32_t, buff*>::cbegin;
    using std::unordered_map<uint32_t, buff*>::cend;
    using std::unordered_map<uint32_t, buff*>::contains;
    using std::unordered_map<uint32_t, buff*>::insert;
    using std::unordered_map<uint32_t, buff*>::erase;

public:
    buffs(fb::game::object& owner);
    ~buffs();

private:
    bool                        push_back(buff& buff);

public:
    bool                        contains(const fb::model::spell& model) const;
    fb::task<buff*>             push_back(const fb::model::spell& spell, uint32_t seconds);
    bool                        remove(uint32_t id);
    bool                        remove(const fb::model::spell& spell);

public:
    buff*                       operator [] (uint32_t id) const;
};

} }

#endif // !__SPELL_H__
