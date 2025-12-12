#ifndef FB_GAME_MARKETPLACE_H
#define FB_GAME_MARKETPLACE_H

#include <fb/model/datetime.h>
#include <fb/model/model.h>
#include <optional>
#include <string>
#include <map>
#include <vector>
#include <cstdint>
#include <async/task.h>

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

    struct listing
    {
        std::string                        id;
        uint32_t                           seller_id = 0;
        item                               item_data;
        uint32_t                           price           = 0;
        uint32_t                           listing_fee     = 0;
        uint32_t                           transaction_fee = 0;
        std::optional<fb::model::datetime> expire_date     = std::nullopt;
        std::optional<fb::model::datetime> created_date    = std::nullopt;
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

private:
    character& _owner;

    async::task<std::string> allocate_id();

public:
    explicit marketplace(character& owner);

    async::task<listing>
    list(const std::string& id, uint8_t item_index, uint16_t count, uint32_t price, uint16_t expire_hours = 72);
    async::task<bool>                   cancel(const std::string& id);
    async::task<listing>                purchase(const std::string& id);
    async::task<search_result>          search(const search_option& option);
    async::task<std::optional<listing>> check_status(const std::string& id);
};

} // namespace fb::game

#endif // FB_GAME_MARKETPLACE_H
