#ifndef FB_GAME_MATCHMAKER_H
#define FB_GAME_MATCHMAKER_H

#include <async/task.h>
#include <fb/lua.h>
#include <fb/model/model.h>
#include <fb/protocol/flatbuffer/protocol.h>

#include <chrono>
#include <cstdint>
#include <memory>
#include <optional>
#include <unordered_map>
#include <vector>

namespace fb::game {

class character;
class match;

struct matchmaking_skill
{
public:
    using dto_type = fb::protocol::internal::MatchmakingSkill;

public:
    uint32_t match_type = 0;
    double   mu         = 0.0;
    double   sigma      = 0.0;

public:
    dto_type to_protocol(uint32_t user) const;
};

class matchmaker : public fb::lua::luable
{
public:
    LUA_PROTOTYPE

    static constexpr double DEFAULT_MU    = 25.0;
    static constexpr double DEFAULT_SIGMA = 25.0 / 3.0;

    static constexpr auto RECONCILE_AFTER = std::chrono::seconds(60);

    // USER failures are thrown back to the script; SERVER cleanups only log them and skip script callbacks.
    enum class initiator : uint8_t
    {
        USER,
        SERVER,
    };

    character& owner;

private:
    struct ticket_state
    {
        uint32_t match_type;
        uint64_t ticket_id;
    };

    std::unordered_map<uint32_t, matchmaking_skill> _skills;
    std::optional<uint64_t>                         _pending_match_id;
    std::optional<uint64_t>                         _ready_match_id;
    std::optional<ticket_state>                     _ticket;
    bool                                            _enqueuing   = false;
    bool                                            _reconciling = false;
    std::chrono::steady_clock::time_point           _last_activity;
    std::weak_ptr<fb::game::match>                  _session;
    std::optional<uint32_t>                         _return_map;
    std::optional<fb::model::point16_t>             _return_position;

    void touch();
    void clear_pending_match_id();
    bool clear_pending_match_id_if(uint64_t match_id);
    bool mark_ready(uint64_t match_id);

public:
    explicit matchmaker(character& owner);

    void                                     load(const std::vector<matchmaking_skill::dto_type>& skills);
    std::vector<matchmaking_skill::dto_type> to_protocol() const;
    std::optional<matchmaking_skill>         get(uint32_t match_type) const;
    void                                     upsert(uint32_t match_type, double mu, double sigma);
    const std::optional<uint64_t>&           pending_match_id() const;
    void                                     set_pending_match_id(uint64_t match_id);
    bool                                     on_ready(uint64_t match_id);
    bool                                     on_dissolved(uint64_t match_id, bool ticket_closed);
    bool                                     on_ticket_removed(uint64_t ticket_id);
    bool                                     queued() const;
    bool                                     enqueuing() const;
    std::optional<uint64_t>                  ticket_id() const;
    void                                     begin_enqueue();
    void                                     set_ticket(uint32_t match_type, uint64_t ticket_id);
    void                                     clear_ticket();
    async::task<void>                        enqueue(uint32_t match_type);
    async::task<void>                        dequeue(initiator by);
    async::task<void>                        discard_leftover_ticket();
    bool                                     reconcile_due(std::chrono::steady_clock::time_point now) const;
    async::task<void>                        reconcile();
    async::task<void>                        confirm(uint64_t match_id);
    async::task<void>                        decline(uint64_t match_id, initiator by);
    std::shared_ptr<fb::game::match>         session() const;
    void                                     session(std::shared_ptr<fb::game::match> value);
    void                                     save_return_point();
    void                                     restore_return_point(uint32_t map, const fb::model::point16_t& position);
    bool                                     has_return_point() const;
    uint32_t                                 return_map() const;
    fb::model::point16_t                     return_position() const;
    [[nodiscard]] async::task<bool>          transfer_home();
    void                                     disconnect_after_failed_return();
};

} // namespace fb::game

#endif // FB_GAME_MATCHMAKER_H
