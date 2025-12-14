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

    struct purchase_info
    {
        uint16_t                           purchase_count = 0;
        uint32_t                           purchase_price = 0;
        std::optional<fb::model::datetime> created_date   = std::nullopt;
    };

    struct listing
    {
        std::string                        id;
        uint32_t                           seller_id = 0;
        item                               item_data;
        uint32_t                           price         = 0;
        uint32_t                           listing_fee   = 0;
        uint8_t                            state         = 0; // fb::protocol::marketplace::ListingState
        std::optional<fb::model::datetime> expire_date   = std::nullopt;
        std::optional<fb::model::datetime> created_date  = std::nullopt;
        std::optional<purchase_info>       purchase_info = std::nullopt;

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
        pending_type type;        // LIST or PURCHASE
        std::string  purchase_id; // For purchase: unique purchase ID (used as key), for list: same as listing_id
        std::string  listing_id;  // Listing ID reference
        std::vector<fb::model::dsl> dsls;
        uint32_t                    character_id;                // For purchase: buyer_id, for list: seller_id
        uint16_t                    expected_purchase_count = 0; // For purchase: expected count, for list: 0
        uint32_t                    expected_total_price    = 0; // For purchase: expected total price, for list: 0
    };

    // Type aliases for commonly used types
    using purchase_map_t     = std::unordered_map<std::string, purchase_info>;
    using pending_listings_t = std::unordered_map<std::string, pending_listing_info>;
    using string_vector_t    = std::vector<std::string>;

private:
    character&         _owner;
    pending_listings_t _pending_listings; // Key: purchase_id (for purchase) or listing_id (for list)

public:
    explicit marketplace(character& owner);

private:
    static std::string generate_uuid();

public:
    async::task<listing>              list(uint8_t slot, uint16_t count, uint32_t price, uint16_t expire_hours = 72);
    async::task<bool>                 cancel(const std::string& id);
    async::task<listing>              purchase(const std::string& listing_id, uint16_t purchase_count);
    async::task<search_result>        search(const search_option& option);
    async::task<std::vector<listing>> get_listings(const string_vector_t& listing_ids, uint32_t buyer_id = 0);
    async::task<purchase_map_t>       get_purchases(const string_vector_t& purchase_ids);
    void                              set_pending_listings(pending_listings_t pending_listings);
    async::task<void>                 restore();
    const pending_listings_t&         pending_listings() const;
};

} // namespace fb::game

#endif // FB_GAME_MARKETPLACE_H
