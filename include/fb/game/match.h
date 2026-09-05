#ifndef FB_GAME_MATCH_H
#define FB_GAME_MATCH_H

#include <fb/lua.h>
#include <fb/timer.h>
#include <async/task.h>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace fb::game {

class server;
class character;
class map;
class object;

enum class MATCH_STATE : uint8_t
{
    waiting = 0,
    playing = 1,
    ending  = 2,
    closed  = 3,
};

class match : public fb::lua::luable
{
public:
    LUA_PROTOTYPE

public:
    class container;

private:
    server&                               _server;
    const std::string                     _id;
    const uint32_t                        _type;
    const uint32_t                        _slot;
    const uint32_t                        _expected;
    MATCH_STATE                           _state = MATCH_STATE::waiting;
    mutable std::mutex                    _mutex;
    std::vector<std::weak_ptr<character>> _members;
    std::shared_ptr<fb::timer>            _timer;

public:
    match(server& server, std::string id, uint32_t type, uint32_t slot, uint32_t expected);
    match(const match&) = delete;
    match(match&&)      = delete;
    ~match()            = default;

public:
    const std::string&                      id() const;
    uint32_t                                type() const;
    uint32_t                                slot() const;
    uint32_t                                expected() const;
    MATCH_STATE                             state() const;
    uint32_t                                member_count();
    std::vector<std::shared_ptr<character>> members();
    std::shared_ptr<fb::game::map>          map(uint32_t model_id);

    async::task<void> join(character& ch);
    void              leave(character& ch);
    void              wait(uint32_t seconds);
    void              duration(uint32_t seconds, uint32_t grace_seconds = 0);
    void              finish(uint32_t grace_seconds);
    void              on_death(character& ch, std::shared_ptr<object> killer);
    async::task<void> close();

private:
    void cancel_timer();
    void enqueue_play();
    void enqueue_close();
    async::task<void>
    invoke(std::string_view func, character& ch, std::shared_ptr<object> extra = nullptr, bool wait = true);
    std::vector<std::shared_ptr<character>> snapshot();
    uint32_t                                live_count_locked();
    async::task<void>                       play();
};

class match::container
{
private:
    server&                                                 _server;
    std::mutex                                              _mutex;
    std::unordered_map<std::string, std::shared_ptr<match>> _sessions;
    uint32_t                                                _next_slot = 1;

public:
    explicit container(server& server);

public:
    std::shared_ptr<match> ensure(std::string_view match_id, uint32_t match_type);
    async::task<void>      join(character& ch, std::string_view match_id, uint32_t match_type);
    void                   leave(character& ch);
    void                   remove(std::string_view match_id);
};

} // namespace fb::game

#endif
