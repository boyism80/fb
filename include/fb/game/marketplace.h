#ifndef FB_GAME_MARKETPLACE_H
#define FB_GAME_MARKETPLACE_H

#include <fb/model/datetime.h>
#include <fb/model/model.h>
#include <fb/lua.h>
#include <optional>
#include <string>
#include <map>
#include <unordered_map>
#include <vector>
#include <cstdint>
#include <async/task.h>

namespace fb::game {
class character;
class server;

class marketplace
{
    friend class character;

public:
    struct item
    {
        uint32_t                   owner       = 0;
        uint32_t                   model       = 0;
        uint16_t                   count       = 0;
        std::optional<uint32_t>    durability  = std::nullopt;
        std::optional<std::string> custom_name = std::nullopt;
    };

    struct listing
    {
        std::string                        id;
        uint32_t                           seller_id = 0;
        uint32_t                           buyer_id  = 0;
        item                               item_data;
        uint32_t                           price           = 0;
        uint32_t                           listing_fee     = 0;
        uint32_t                           transaction_fee = 0;
        uint8_t                            state           = 0; // fb::protocol::marketplace::ListingState
        std::optional<fb::model::datetime> expire_date     = std::nullopt;
        std::optional<fb::model::datetime> created_date    = std::nullopt;

        void to_lua(fb::lua::context* lua) const;
    };

    struct search_option
    {
        std::optional<std::string> item_name = std::nullopt;
        std::optional<uint32_t>    min_price = std::nullopt;
        std::optional<uint32_t>    max_price = std::nullopt;
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

    enum class pending_type : uint8_t
    {
        LIST     = 0, // Listing creation (seller)
        PURCHASE = 1  // Purchase transaction (buyer)
    };

    struct pending_listing_info
    {
        pending_type                type; // LIST or PURCHASE
        std::string                 listing_id;
        std::vector<fb::model::dsl> dsls;
        uint32_t                    character_id; // For purchase: buyer_id, for list: seller_id
    };

private:
    character&                                            _owner;
    std::unordered_map<std::string, pending_listing_info> _pending_listings;

public:
    explicit marketplace(character& owner);

private:
    async::task<std::string> allocate_id();

public:
    async::task<listing>       list(uint8_t item_index, uint16_t count, uint32_t price, uint16_t expire_hours = 72);
    async::task<bool>          cancel(const std::string& id);
    async::task<listing>       purchase(const std::string& id);
    async::task<search_result> search(const search_option& option);
    async::task<std::vector<listing>> get_listings(const std::vector<std::string>& listing_ids);
    void              set_pending_listings(std::unordered_map<std::string, pending_listing_info> pending_listings);
    async::task<void> restore();
    const std::unordered_map<std::string, pending_listing_info>& pending_listings() const;
};

} // namespace fb::game

#endif // FB_GAME_MARKETPLACE_H
