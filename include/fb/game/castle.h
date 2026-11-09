#ifndef __FB_GAME_CASTLE_H__
#define __FB_GAME_CASTLE_H__

#include <fb/game/lazy_container.h>
#include <fb/lua.h>
#include <fb/model/datetime.h>
#include <fb/model/model.h>
#include <fb/protocol/flatbuffer/protocol.h>
#include <fb/socket.h>

#include <chrono>
#include <cstdint>
#include <optional>
#include <utility>

namespace fb::protocol::internal::response {
class UpdatedCastle;
}

namespace fb::game {

class server;

class castle : public lua::luable
{
public:
    LUA_PROTOTYPE

public:
    class container;

    enum class siege_end_result : uint8_t
    {
        ENDED,
        NOT_ACTIVE,
        SAVE_FAILED,
    };

    enum class siege_phase : uint8_t
    {
        RUNNING,
        ENDING,
        END_FAILED,
        ENDED,
    };

    struct siege_start_result
    {
        bool     started   = false;
        bool     active    = false;
        uint32_t remaining = 0;
    };

    using castle_dto = fb::protocol::internal::Castle;

private:
    server&                             _server;
    fb::model::enum_value::DIVINE_BEAST _divine_beast;
    std::optional<uint32_t>             _owner_clan_id;
    std::optional<uint64_t>             _siege_id;
    fb::model::datetime                 _siege_end_at;
    fb::model::datetime                 _siege_active_until;
    uint64_t                            _revision = 0;
    siege_phase                         _phase    = siege_phase::RUNNING;
    std::optional<uint32_t>             _siege_winner;

public:
    // clang-format off
    castle(server& server, fb::model::enum_value::DIVINE_BEAST divine_beast, const std::optional<uint32_t>& owner_clan_id);
    castle(const castle&) = delete;
    castle(castle&&);
    ~castle() = default;
    // clang-format on

public:
    // clang-format off
    uint32_t                            id() const;
    fb::model::enum_value::DIVINE_BEAST divine_beast() const;
    const std::optional<uint32_t>&      owner_clan_id() const;
    void                                owner_clan_id(const std::optional<uint32_t>& value);
    bool                                siege_active() const;
    bool                                siege_pending() const;
    const std::optional<uint64_t>&      siege_id() const;
    std::optional<uint32_t>             siege_remaining_seconds() const;
    bool                                load(const castle_dto& row);
    siege_phase                         phase() const;
    void                                phase(siege_phase value);
    const std::optional<uint32_t>&      siege_winner() const;
    void                                siege_winner(const std::optional<uint32_t>& value);
    // clang-format on
};

class castle::container : public lazy_container<castle>
{
protected:
    async::task<entity_ptr> fetch(uint32_t id) override;

private:
    void on_owner_changed(fb::model::enum_value::DIVINE_BEAST divine_beast, const std::optional<uint32_t>& previous);
    async::task<void> reflect(const castle_dto& row);

public:
    explicit container(server& server);

public:
    // clang-format off
    async::task<void> load_all();
    async::task<void> refresh();
    async::task<void> on_updated(const fb::protocol::internal::response::UpdatedCastle& resp);
    async::task<castle::siege_start_result> start_siege(fb::model::enum_value::DIVINE_BEAST divine_beast, uint64_t siege_key);
    async::task<bool> finish_siege(fb::model::enum_value::DIVINE_BEAST divine_beast);
    async::task<castle::siege_end_result> end_siege(fb::model::enum_value::DIVINE_BEAST divine_beast, const std::optional<uint32_t>& winner_clan_id);
    // clang-format on
};

} // namespace fb::game

#endif // !__FB_GAME_CASTLE_H__
