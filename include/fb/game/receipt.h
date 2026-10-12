#ifndef FB_GAME_RECEIPT_H
#define FB_GAME_RECEIPT_H

#include <async/task.h>
#include <json/json.h>

#include <cstdint>
#include <map>
#include <optional>
#include <vector>

namespace fb::protocol::internal {
class Receipt;
}

namespace fb::game {

class character;

enum class RECEIPT_TYPE : uint8_t
{
    CLAN_MONEY      = 0,
    STORAGE_CLAIM   = 1,
    MARKET_LIST     = 2,
    MARKET_PURCHASE = 3,
};

class receipts
{
public:
    struct entry
    {
        RECEIPT_TYPE type      = RECEIPT_TYPE::CLAN_MONEY;
        uint64_t     target    = 0;
        Json::Value  detail    = Json::Value{Json::objectValue};
        bool         in_flight = false;
    };
    using entry_map = std::map<uint64_t, entry>;
    using dto       = fb::protocol::internal::Receipt;

private:
    character& _owner;
    entry_map  _entries;
    bool       _settling = false;

public:
    explicit receipts(character& owner);

public:
    uint64_t issue(RECEIPT_TYPE type, uint64_t target, Json::Value detail = Json::Value{Json::objectValue});
    entry*   find(uint64_t id);
    std::optional<uint64_t> find(RECEIPT_TYPE type, uint64_t target) const;
    void                    erase(uint64_t id);
    const entry_map&        entries() const;
    void                    load(const std::vector<dto>& rows);
    std::vector<dto>        to_protocol() const;
    async::task<void>       settle();
};

} // namespace fb::game

#endif // FB_GAME_RECEIPT_H
