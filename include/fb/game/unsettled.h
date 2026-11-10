#ifndef FB_GAME_UNSETTLED_H
#define FB_GAME_UNSETTLED_H

#include <async/task.h>
#include <fb/game/item/items.h>
#include <json/json.h>

#include <cstdint>
#include <map>
#include <optional>
#include <vector>

namespace fb::protocol::internal {
class UnsettledRequest;
}

namespace fb::game {

class character;

enum class UNSETTLED_TYPE : uint8_t
{
    CLAN_MONEY      = 0,
    MARKET_LIST     = 2,
    MARKET_PURCHASE = 3,
    CLAN_FEE        = 4,
};

class unsettled_requests
{
public:
    struct entry
    {
        UNSETTLED_TYPE type      = UNSETTLED_TYPE::CLAN_MONEY;
        uint64_t       target    = 0;
        Json::Value    detail    = Json::Value{Json::objectValue};
        bool           in_flight = false;
    };
    using entry_map = std::map<uint64_t, entry>;
    using dto       = fb::protocol::internal::UnsettledRequest;

private:
    character& _owner;
    entry_map  _entries;
    bool       _settling = false;

public:
    explicit unsettled_requests(character& owner);

public:
    // clang-format off
    std::optional<uint64_t> add(UNSETTLED_TYPE type, uint64_t target, uint64_t money, const std::vector<items::escrow_part>& parts, Json::Value detail = Json::Value{Json::objectValue});
    // clang-format on
    const entry*            find(uint64_t id) const;
    std::optional<uint64_t> find(UNSETTLED_TYPE type, uint64_t target) const;
    bool                    held(uint64_t id) const;
    void                    confirm(uint64_t id);
    void                    refuse(uint64_t id);
    void                    leave(uint64_t id);
    void                    retry(uint64_t id);
    bool                    charge(uint64_t id);
    const entry_map&        entries() const;
    void                    load(const std::vector<dto>& rows);
    std::vector<dto>        to_protocol() const;
    async::task<void>       settle();
};

} // namespace fb::game

#endif // FB_GAME_UNSETTLED_H
