#ifndef FB_GAME_QUEST_H
#define FB_GAME_QUEST_H

#include <cstdint>
#include <fb/model/model.h>
#include <fb/lua.h>

namespace fb::game {

class character;

class quest : public fb::lua::luable
{
public:
    LUA_PROTOTYPE

private:
    uint32_t    _step      = 0;
    uint32_t    _progress  = 0;
    bool        _completed = false;
    std::string _param     = "";

public:
    uint32_t                           id;
    std::weak_ptr<fb::game::character> owner;

public:
    quest(uint32_t id, std::weak_ptr<fb::game::character> owner);
    quest(uint32_t                           id,
          std::weak_ptr<fb::game::character> owner,
          uint32_t                           step,
          uint32_t                           progress,
          bool                               completed,
          std::string                        param);
    ~quest() = default;

public:
    uint32_t    step() const;
    uint32_t    progress() const;
    std::string param() const;
    void        param(const std::string& param);
    bool        inc_progress(uint32_t value = 1);
    bool        inc_step(uint32_t value = 1);
    bool        complete();
    bool        completed() const;

public:
    static int builtin_model(lua_State* L);
    static int builtin_step(lua_State* L);
    static int builtin_progress(lua_State* L);
    static int builtin_param(lua_State* L);
    static int builtin_inc_progress(lua_State* L);
    static int builtin_inc_step(lua_State* L);
    static int builtin_complete(lua_State* L);
    static int builtin_completed(lua_State* L);
    static int builtin_completed_progress(lua_State* L);
    static int builtin_completed_step(lua_State* L);

public:
};

class quests : private std::unordered_map<uint32_t, std::shared_ptr<quest>>
{
public:
    using super = std::unordered_map<uint32_t, std::shared_ptr<quest>>;

    using super::operator[];
    using super::at;
    using super::begin;
    using super::cbegin;
    using super::cend;
    using super::contains;
    using super::empty;
    using super::end;
    using super::erase;
    using super::size;

private:
    std::weak_ptr<fb::game::character> _owner;

public:
    quests()                          = default;
    quests(const quests&)             = delete;
    quests(quests&&)                  = default;
    quests& operator= (const quests&) = delete;
    ~quests()                         = default;

public:
    void owner(std::weak_ptr<fb::game::character> owner);
    void add(uint32_t id, uint32_t step, uint32_t progress, bool completed, std::string param);
    void remove(uint32_t id);
};

} // namespace fb::game

#endif // FB_GAME_QUEST_H