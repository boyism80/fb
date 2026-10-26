#ifndef FB_GAME_MARKETPLACE_H
#define FB_GAME_MARKETPLACE_H

#include <async/task.h>
#include <fb/lua.h>
#include <fb/model/datetime.h>
#include <fb/model/model.h>

#include <chrono>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace fb::game {
class character;
class server;

class marketplace
{
public:
    struct item
    {
        uint32_t                   owner       = 0;
        uint32_t                   model       = 0;
        uint16_t                   count       = 0;
        std::optional<uint32_t>    durability  = std::nullopt;
        std::optional<std::string> custom_name = std::nullopt;
    };

    struct purchase_info
    {
        uint16_t                           purchase_count = 0;
        uint64_t                           purchase_price = 0;
        std::optional<fb::model::datetime> created_date   = std::nullopt;
    };

    struct listing
    {
        uint64_t                           id        = 0;
        uint32_t                           seller_id = 0;
        item                               item_data;
        uint64_t                           price        = 0;
        uint64_t                           listing_fee  = 0;
        uint8_t                            state        = 0; // fb::protocol::marketplace::ListingState
        std::optional<fb::model::datetime> expire_date  = std::nullopt;
        std::optional<fb::model::datetime> created_date = std::nullopt;
        std::optional<purchase_info>       purchase     = std::nullopt;

        void to_lua(fb::lua::context* lua) const;
    };

    struct search_option
    {
        std::optional<std::string> item_name = std::nullopt;
        std::optional<uint64_t>    min_price = std::nullopt;
        std::optional<uint64_t>    max_price = std::nullopt;
        std::optional<uint32_t>    seller_id = std::nullopt;
        std::optional<std::string> sort_by   = std::nullopt;
        uint32_t                   page      = 1;
    };

    struct search_result
    {
        std::vector<listing> listings;
        uint32_t             total_count = 0;
        uint32_t             page        = 1;
    };

private:
    // The marketplace rejects a request that arrives after sent + REQUEST_TTL.
    inline static constexpr auto REQUEST_TTL = std::chrono::seconds(10);

    character& _owner;

public:
    explicit marketplace(character& owner);

public:
    async::task<listing>
                      list(uint8_t slot, uint32_t model_id, uint16_t count, uint64_t price, uint16_t expire_hours = 72);
    async::task<bool> cancel(uint64_t id);
    async::task<uint32_t>             clear();
    async::task<listing>              purchase(uint64_t listing_id, uint16_t purchase_count);
    async::task<search_result>        search(const search_option& option);
    async::task<std::vector<listing>> get_listings(const std::vector<uint64_t>& listing_ids, uint32_t buyer_id = 0);
    async::task<void>                 settle_list(uint64_t listing_id);
    async::task<void>                 settle_purchase(uint64_t purchase_id);

private:
    static std::string rejection_message(fb::model::enum_value::ERROR_CODE error, std::string fallback);
};

} // namespace fb::game

#endif // FB_GAME_MARKETPLACE_H
