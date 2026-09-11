#ifndef FB_GAME_MATCHMAKER_H
#define FB_GAME_MATCHMAKER_H

#include <async/task.h>
#include <fb/lua.h>
#include <fb/protocol/flatbuffer/protocol.h>
#include <optional>
#include <string>
#include <string_view>
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

    character& owner;

private:
    struct enrollment_state
    {
        uint32_t    match_type;
        std::string registry_id;
    };

    std::unordered_map<uint32_t, matchmaking_skill> _entries;
    std::optional<std::string>                      _pending_match_id;
    std::optional<enrollment_state>                 _enrollment;

    void enqueue_squad_unregister(uint32_t match_type, std::string_view registry_id);

public:
    explicit matchmaker(character& owner);

    void                                     load(const std::vector<matchmaking_skill::dto_type>& skills);
    std::vector<matchmaking_skill::dto_type> to_protocol() const;
    std::optional<matchmaking_skill>         get(uint32_t match_type) const;
    void                                     upsert(uint32_t match_type, double mu, double sigma);
    const std::optional<std::string>&        pending_match_id() const;
    void                                     set_pending_match_id(std::string match_id);
    void                                     clear_pending_match_id();
    bool                                     clear_pending_match_id_if(std::string_view match_id);
    bool                                     enrolled() const;
    std::optional<std::string_view>          registry_id() const;
    void                                     set_enrollment(uint32_t match_type, std::string registry_id);
    void                                     clear_enrollment();
    async::task<void>                        register_queue(uint32_t match_type);
    async::task<void>                        unregister_queue(bool quiet = false);
    async::task<void>                        discard_leftover_enrollment();
    async::task<void>                        confirm_queue(std::string match_id, bool quiet = false);
    async::task<void>                        decline_queue(std::string match_id, bool quiet = false);
};

} // namespace fb::game

#endif // FB_GAME_MATCHMAKER_H
