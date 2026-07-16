#ifndef __SPELL_H__
#define __SPELL_H__

#include <chrono>
#include <fb/game/inventory.h>
#include <fb/model/model.h>
#include <fb/lua.h>
#include <async/task.h>

namespace fb::game {

class server;
class object;
class life;

class spell : public lua::luable
{
public:
    LUA_PROTOTYPE

private:
    fb::model::datetime _next;

public:
    const fb::game::server& server;
    const fb::game::life&   owner;
    const fb::model::spell& model;

public:
    // clang-format off
    spell(const fb::game::server& server, const fb::game::life& owner, const fb::model::spell& model, uint16_t delay = 0);
   ~spell();
    // clang-format on

public:
    // clang-format off
    void                       delay(uint16_t value);
    uint16_t                   delay() const;
    const fb::model::datetime& next() const;
    // clang-format on
};

class spells : public fb::game::inventory<fb::game::spell>
{
public:
    using super = fb::game::inventory<fb::game::spell>;

    struct listener_t;

private:
    std::weak_ptr<fb::game::life> _owner;

public:
    // clang-format off
   spells();
   ~spells();
    // clang-format on

public:
    // clang-format off
    void                             owner(std::shared_ptr<fb::game::life> owner);
    std::shared_ptr<fb::game::life>  owner() const;
    std::shared_ptr<fb::game::spell> find(std::string_view name) const;
    std::shared_ptr<fb::game::spell> find(const fb::model::spell& model) const;
    // clang-format on

public:
    // clang-format off
    uint8_t add(std::shared_ptr<fb::game::spell> element);
    uint8_t add(std::shared_ptr<fb::game::spell> element, uint8_t index) override;
    uint8_t add(const fb::model::spell& model, uint8_t slot, uint16_t delay);
    uint8_t add(const fb::model::spell& model);
    bool    remove(uint8_t index) override;
    bool    swap(uint8_t src, uint8_t dst) override;
    // clang-format on
};

struct spells::listener_t
{
    // clang-format off
    virtual void on_spell_update(life& me, uint8_t index)                     = 0;
    virtual void on_spell_remove(life& me, uint8_t index)                     = 0;
    virtual void on_spell_delay(life& me, const spell& spell, uint32_t delay) = 0;
    // clang-format on
};

class buff : public ::fb::lua::luable
{
public:
    LUA_PROTOTYPE

private:
    fb::model::timespan _duration;

public:
    const fb::game::server&   server;
    const fb::model::spell&   model;
    const fb::game::object*   caster;
    const fb::model::datetime start;

public:
    // clang-format off
    buff(const fb::game::server& server, const fb::model::spell& model, const fb::game::object* caster, uint32_t seconds);
   ~buff();
    // clang-format on

public:
    // clang-format off
    const fb::model::timespan& duration() const;
    void                       duration(const fb::model::timespan& value);
    fb::model::timespan        remaining() const;
    void                       remaining(const fb::model::timespan& value);
    // clang-format on
};

class buffs : private std::unordered_map<uint32_t, std::shared_ptr<buff>>
{
private:
    using super = std::unordered_map<uint32_t, std::shared_ptr<buff>>;

private:
    fb::game::object& _owner;

public:
    using super::begin;
    using super::cbegin;
    using super::cend;
    using super::contains;
    using super::end;
    using super::erase;
    using super::insert;
    using super::size;

public:
    // clang-format off
    buffs(fb::game::object& owner);
    buffs(const buffs& other);
   ~buffs();
    // clang-format on

private:
    bool push_back(const std::shared_ptr<buff>& buff);

public:
    // clang-format off
    bool                                             contains(const fb::model::spell& model) const;
    [[nodiscard]] async::task<std::shared_ptr<buff>> push_back(const fb::model::spell& spell, uint32_t seconds, const std::shared_ptr<fb::game::object>& caster = nullptr);
    [[nodiscard]] async::task<bool>                  remove(uint32_t id);
    [[nodiscard]] async::task<bool>                  remove(const fb::model::spell& spell);
    // clang-format on

public:
    std::shared_ptr<buff> operator[] (uint32_t id) const;
};

} // namespace fb::game

#endif // !__SPELL_H__
