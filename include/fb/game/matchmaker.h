#ifndef FB_GAME_MATCHMAKER_H
#define FB_GAME_MATCHMAKER_H

#include <async/task.h>
#include <fb/lua.h>
#include <fb/protocol/flatbuffer/protocol.h>

#include <cstdint>
#include <optional>
#include <unordered_map>
#include <vector>

namespace fb::game {

class character;

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
    std::optional<ticket_state>                     _ticket;
    bool                                            _enqueuing = false;

public:
    explicit matchmaker(character& owner);

    void                                     load(const std::vector<matchmaking_skill::dto_type>& skills);
    std::vector<matchmaking_skill::dto_type> to_protocol() const;
    std::optional<matchmaking_skill>         get(uint32_t match_type) const;
    void                                     upsert(uint32_t match_type, double mu, double sigma);
    const std::optional<uint64_t>&           pending_match_id() const;
    void                                     set_pending_match_id(uint64_t match_id);
    void                                     clear_pending_match_id();
    bool                                     clear_pending_match_id_if(uint64_t match_id);
    bool                                     queued() const;
    bool                                     enqueuing() const;
    std::optional<uint64_t>                  ticket_id() const;
    void                                     begin_enqueue();
    void                                     set_ticket(uint32_t match_type, uint64_t ticket_id);
    void                                     clear_ticket();
    async::task<void>                        enqueue(uint32_t match_type);
    async::task<void>                        dequeue(initiator by);
    async::task<void>                        discard_leftover_ticket();
    async::task<void>                        confirm(uint64_t match_id);
    async::task<void>                        decline(uint64_t match_id, initiator by);
};

} // namespace fb::game

#endif // FB_GAME_MATCHMAKER_H
