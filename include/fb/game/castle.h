#ifndef __FB_GAME_CASTLE_H__
#define __FB_GAME_CASTLE_H__

#include <fb/socket.h>
#include <fb/game/lazy_container.h>
#include <fb/lua.h>
#include <fb/model/model.h>
#include <chrono>
#include <cstdint>
#include <optional>

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

private:
    server&                               _server;
    fb::model::enum_value::DIVINE_BEAST   _divine_beast;
    std::optional<uint32_t>               _owner_clan_id;
    bool                                  _siege_active = false;
    std::chrono::steady_clock::time_point _siege_end_at{};

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
    std::optional<uint32_t>             siege_remaining_seconds() const;
    void                                start_siege(uint32_t duration_seconds);
    void                                clear_siege();
    // clang-format on
};

class castle::container : public lazy_container<castle>
{
protected:
    async::task<entity_ptr> fetch(uint32_t id) override;

public:
    explicit container(server& server);

public:
    // clang-format off
    async::task<void> load_all();
    async::task<void> apply_updated(const fb::protocol::internal::response::UpdatedCastle& resp);
    async::task<void> end_siege(fb::model::enum_value::DIVINE_BEAST divine_beast, const std::optional<uint32_t>& winner_clan_id);
    // clang-format on
};

} // namespace fb::game

#endif // !__FB_GAME_CASTLE_H__
