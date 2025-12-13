#include <fb/game/marketplace.h>
#include <fb/game/character.h>
#include <fb/game/server.h>
#include <fb/game/storage.h>
#include <fb/game/item/weapon.h>
#include <fb/protocol/flatbuffer/protocol.h>
#include <fb/model/model.h>
#include <fb/encoding.h>
#include <json/json.h>
#include <stdexcept>
#include <sstream>
#include <unordered_set>

using namespace fb::game;

namespace mp      = fb::protocol::marketplace;
namespace mp_reqs = mp::request;
namespace mp_resp = mp::response;

marketplace::marketplace(character& owner) :
    _owner(owner)
{ }

async::task<std::string> marketplace::allocate_id()
{
    this->_owner.assert_thread();

    auto   weak = this->_owner.weak_from_this_as<fb::game::character>();
    auto   req  = mp_reqs::AllocateListingId{this->_owner.id};
    auto&& resp = co_await this->_owner.server.http.post("marketplace", "/marketplace/allocate-listing-id", req);

    co_await this->_owner.server.threads.switching(weak);

    if (resp.error != 0)
        throw std::runtime_error(std::format("Failed to allocate listing ID: error={}", resp.error));

    co_return resp.listing_id;
}

async::task<marketplace::listing>
marketplace::list(uint8_t item_index, uint16_t count, uint32_t price, uint16_t expire_hours)
{
    this->_owner.assert_thread();

    auto weak = this->_owner.weak_from_this_as<fb::game::character>();
    if (weak.expired())
        throw std::runtime_error("Character has expired");

    auto id = co_await this->allocate_id();
    if (weak.expired())
        throw std::runtime_error("Character has expired");

    // Get item from inventory
    auto item = this->_owner.items.at(item_index);
    if (item == nullptr)
        throw std::runtime_error("Item not found at index");

    auto& model = item->based<fb::model::item>();
    if (item->count() < count)
        throw std::runtime_error("Insufficient item count");

    // Build request item data
    auto durability  = item->durability();
    auto custom_name = std::optional<std::string>{};

    // Check if item is a weapon with custom name
    if (model.attr(ITEM_ATTRIBUTE::WEAPON))
    {
        auto weapon = static_cast<fb::game::weapon*>(item.get());
        if (weapon != nullptr && weapon->custom_name().has_value())
            custom_name = weapon->custom_name().value();
    }

    // Calculate listing fee (10% of price)
    auto listing_fee = static_cast<uint32_t>(price * 0.1);

    // Validate listing fee BEFORE API call
    if (this->_owner.money() < listing_fee)
        throw std::runtime_error("Insufficient money for listing fee");

    // Prepare DSL for pending listing recovery (BEFORE API call)
    auto item_dsl = fb::model::dsl::item(model.id, count, durability, custom_name, 100.0);
    auto dsls     = std::vector<fb::model::dsl>{item_dsl.to_dsl()};

    // Add listing fee as money DSL for recovery
    if (listing_fee > 0)
    {
        auto money_dsl = fb::model::dsl::money(listing_fee);
        dsls.push_back(money_dsl.to_dsl());
    }

    // Store pending listing for recovery in case of server failure
    this->_pending_listings.emplace(id,
                                    pending_listing_info{.type         = pending_type::LIST,
                                                         .listing_id   = id,
                                                         .dsls         = std::move(dsls),
                                                         .character_id = this->_owner.id});

    // Remove item from inventory and deduct listing fee BEFORE API call
    std::ignore = this->_owner.items.remove(item_index, count, ITEM_DELETE_TYPE::REMOVED);
    this->_owner.money_reduce(listing_fee);

    // Log before API call (after deduction)
    auto log_data_before            = Json::Value();
    log_data_before["character_id"] = static_cast<Json::Int64>(this->_owner.id);
    log_data_before["listing_id"]   = id;
    log_data_before["item_model"]   = model.id;
    log_data_before["item_count"]   = count;
    log_data_before["price"]        = static_cast<Json::Int64>(price);
    log_data_before["listing_fee"]  = static_cast<Json::Int64>(listing_fee);
    this->_owner.server.log.write("marketplace_list", log_data_before);

    // Send request to marketplace server
    try
    {
        auto&& resp = co_await this->_owner.server.http.post(
            "marketplace",
            "/marketplace/list",
            mp_reqs::List{
                this->_owner.id,
                id,
                mp::Item{this->_owner.id, model.id, count, durability, custom_name},
                price,
                expire_hours
        });

        co_await this->_owner.server.threads.switching(weak);

        if (resp.error != 0)
        {
            // API call returned an error - throw exception to handle in catch block
            throw std::runtime_error(std::format("Failed to list item: error={}",
                                                 enum_tostring((fb::model::enum_value::ERROR_CODE)resp.error)));
        }

        // API call succeeded - remove from pending_listings
        this->_pending_listings.erase(id);
        if (resp.listing_id != id)
        {
            // If the server returned a different listing_id, also remove that entry if it exists
            this->_pending_listings.erase(resp.listing_id);
        }

        // Log successful listing
        auto log_data_success                 = Json::Value();
        log_data_success["character_id"]      = static_cast<Json::Int64>(this->_owner.id);
        log_data_success["listing_id"]        = id;
        log_data_success["server_listing_id"] = resp.listing_id;
        this->_owner.server.log.write("marketplace_list_success", log_data_success);

        // Build and return listing
        co_return marketplace::listing{
            .id              = resp.listing_id,
            .seller_id       = this->_owner.id,
            .buyer_id        = 0,
            .item_data       = {.owner       = this->_owner.id,
                                .model       = model.id,
                                .count       = count,
                                .durability  = durability,
                                .custom_name = custom_name},
            .price           = price,
            .listing_fee     = listing_fee,
            .transaction_fee = 0, // Will be calculated by server
            .state           = 0,
            .expire_date     = std::nullopt,
            .created_date    = std::nullopt
        };
    }
    catch (const std::exception& e)
    {
        if (weak.expired())
            throw std::runtime_error("Character has expired");

        // Any failure (system error or logical error) - items/money already deducted, keep DSL for recovery
        // DSL remains in _pending_listings for recovery on next login
        auto log_data            = Json::Value();
        log_data["character_id"] = static_cast<Json::Int64>(this->_owner.id);
        log_data["listing_id"]   = id;
        log_data["error"]        = e.what();
        this->_owner.server.log.write("marketplace_list_failed", log_data);
        throw std::runtime_error(std::format("Failed to list item: {}", e.what()));
    }
}

async::task<bool> marketplace::cancel(const std::string& id)
{
    this->_owner.assert_thread();

    // Copy id to local variable to ensure it survives across coroutine suspension
    auto id_copy = id;

    // Log before API call
    auto log_data_before            = Json::Value();
    log_data_before["character_id"] = static_cast<Json::Int64>(this->_owner.id);
    log_data_before["listing_id"]   = id_copy;
    this->_owner.server.log.write("marketplace_cancel", log_data_before);

    auto   weak = this->_owner.weak_from_this_as<fb::game::character>();
    auto   req  = mp_reqs::Cancel{this->_owner.id, id_copy};
    auto&& resp = co_await this->_owner.server.http.post("marketplace", "/marketplace/cancel", req);

    co_await this->_owner.server.threads.switching(weak);

    if (resp.error != 0)
    {
        auto log_data            = Json::Value();
        log_data["character_id"] = static_cast<Json::Int64>(this->_owner.id);
        log_data["listing_id"]   = id_copy;
        log_data["error"]        = static_cast<Json::Int64>(resp.error);
        this->_owner.server.log.write("marketplace_cancel_failed", log_data);
        throw std::runtime_error(std::format("Failed to cancel listing: error={}", resp.error));
    }

    // Remove from pending_listings if it exists (in case listing was created but response was lost)
    this->_pending_listings.erase(id_copy);

    // Log successful cancellation
    auto log_data_success            = Json::Value();
    log_data_success["character_id"] = static_cast<Json::Int64>(this->_owner.id);
    log_data_success["listing_id"]   = id_copy;
    this->_owner.server.log.write("marketplace_cancel_success", log_data_success);

    co_return true;
}

async::task<marketplace::listing> marketplace::purchase(const std::string& id)
{
    this->_owner.assert_thread();

    // Copy id to local variable to ensure it survives across coroutine suspension
    auto id_copy = id;

    // Get listing info first (needed for price validation and timeout recovery)
    auto weak        = this->_owner.weak_from_this_as<fb::game::character>();
    auto listing_ids = std::vector<std::string>{id_copy};
    auto listings    = co_await this->get_listings(listing_ids);

    if (listings.empty())
        throw std::runtime_error("Listing not found");

    auto& listing = listings[0];
    if (listing.state != static_cast<uint8_t>(mp::ListingState::ACTIVE))
        throw std::runtime_error("Listing is not available for purchase");

    auto price = listing.price; // Buyer only pays price, not transaction_fee

    // Validate money BEFORE API call
    if (this->_owner.money() < price)
        throw std::runtime_error("Insufficient money");

    // Prepare DSL for pending purchase recovery (money only) BEFORE API call
    auto dsls = std::vector<fb::model::dsl>{};
    if (price > 0)
    {
        auto money_dsl = fb::model::dsl::money(price);
        dsls.push_back(money_dsl.to_dsl());
    }

    // Store pending purchase for recovery in case of server failure
    this->_pending_listings.emplace(id_copy,
                                    pending_listing_info{.type         = pending_type::PURCHASE,
                                                         .listing_id   = id_copy,
                                                         .dsls         = std::move(dsls),
                                                         .character_id = this->_owner.id});

    // Deduct money BEFORE API call
    this->_owner.money_reduce(price);

    // Log before API call (after deduction)
    auto log_data_before            = Json::Value();
    log_data_before["character_id"] = static_cast<Json::Int64>(this->_owner.id);
    log_data_before["listing_id"]   = id_copy;
    log_data_before["seller_id"]    = static_cast<Json::Int64>(listing.seller_id);
    log_data_before["price"]        = static_cast<Json::Int64>(price);
    this->_owner.server.log.write("marketplace_purchase", log_data_before);

    // Send purchase request to marketplace server
    auto req             = mp_reqs::Purchase{this->_owner.id, id_copy};
    auto unhandled_error = true;

    try
    {
        auto&& resp = co_await this->_owner.server.http.post("marketplace", "/marketplace/purchase", req);

        co_await this->_owner.server.threads.switching(weak);

        auto ec = (fb::model::enum_value::ERROR_CODE)resp.error;
        if (ec != fb::model::enum_value::ERROR_CODE::NONE)
        {
            // Check if this is a definite failure (logical error) or system error
            switch (ec)
            {
            case fb::model::enum_value::ERROR_CODE::MARKETPLACE_LISTING_NOT_FOUND:
            case fb::model::enum_value::ERROR_CODE::MARKETPLACE_LISTING_EXPIRED:
            case fb::model::enum_value::ERROR_CODE::MARKETPLACE_LISTING_ALREADY_SOLD:
            case fb::model::enum_value::ERROR_CODE::MARKETPLACE_LISTING_ALREADY_CANCELLED:
                unhandled_error = false;
                break;

            default:
                unhandled_error = true;
                break;
            }
            throw std::runtime_error(std::format("Failed to purchase item: error={}", enum_tostring(ec)));
        }

        // Purchase succeeded - remove from pending_listings
        this->_pending_listings.erase(id_copy);

        // Log successful purchase
        auto log_data_success            = Json::Value();
        log_data_success["character_id"] = static_cast<Json::Int64>(this->_owner.id);
        log_data_success["listing_id"]   = id_copy;
        this->_owner.server.log.write("marketplace_purchase_success", log_data_success);

        // Build and return listing (from response item data)
        co_return marketplace::listing{
            .id              = id_copy,
            .seller_id       = 0, // Not provided in response
            .buyer_id        = 0,
            .item_data       = {.owner       = resp.item.owner,
                                .model       = resp.item.model,
                                .count       = resp.item.count,
                                .durability  = resp.item.durability,
                                .custom_name = resp.item.custom_name},
            .price           = 0, // Not provided in response
            .listing_fee     = 0,
            .transaction_fee = 0,
            .state           = 0,
            .expire_date     = std::nullopt,
            .created_date    = std::nullopt
        };
    }
    catch (const std::exception& e)
    {
        if (weak.expired())
            throw std::runtime_error("Character has expired");

        if (unhandled_error)
        {
            // HTTP exception (timeout, connection error, etc.) - system error
            // Money already deducted, DSL is already saved for recovery
            auto log_data            = Json::Value();
            log_data["character_id"] = static_cast<Json::Int64>(this->_owner.id);
            log_data["listing_id"]   = id_copy;
            log_data["error"]        = e.what();
            this->_owner.server.log.write("marketplace_purchase_failed", log_data);
        }
        else
        {
            // Logical error - restore money
            this->_owner.money_add(price);
            this->_pending_listings.erase(id); // Remove DSL as purchase definitely failed
            auto log_data            = Json::Value();
            log_data["character_id"] = static_cast<Json::Int64>(this->_owner.id);
            log_data["listing_id"]   = id;
            log_data["error"]        = e.what();
            this->_owner.server.log.write("marketplace_purchase_failed", log_data);
        }

        throw std::runtime_error(std::format("Failed to purchase item: {}", e.what()));
    }
}

async::task<marketplace::search_result> marketplace::search(const search_option& option)
{
    this->_owner.assert_thread();

    auto   weak = this->_owner.weak_from_this_as<fb::game::character>();
    auto&& resp = co_await this->_owner.server.http.post("marketplace",
                                                         "/marketplace/search",
                                                         mp_reqs::Search{option.item_name,
                                                                         option.min_price,
                                                                         option.max_price,
                                                                         option.seller_id,
                                                                         option.sort_by,
                                                                         option.page});

    co_await this->_owner.server.threads.switching(weak);

    if (resp.error != 0)
        throw std::runtime_error(std::format("Failed to search items: error={}",
                                             enum_tostring((fb::model::enum_value::ERROR_CODE)resp.error)));

    search_result result{.listings = {}, .total_count = resp.result.total_count, .page = resp.result.page};
    result.listings.reserve(resp.result.listings.size());

    for (const auto& listing : resp.result.listings)
    {
        auto expire_date_opt = std::optional<fb::model::datetime>{};
        if (!listing.expire_date.empty())
            expire_date_opt = fb::model::datetime(listing.expire_date);

        auto created_date_opt = std::optional<fb::model::datetime>{};
        if (!listing.created_date.empty())
            created_date_opt = fb::model::datetime(listing.created_date);

        result.listings.push_back(marketplace::listing{
            .id              = listing.id,
            .seller_id       = listing.seller_id,
            .buyer_id        = 0,
            .item_data       = {.owner       = listing.item.owner,
                                .model       = listing.item.model,
                                .count       = listing.item.count,
                                .durability  = listing.item.durability,
                                .custom_name = listing.item.custom_name},
            .price           = listing.price,
            .listing_fee     = static_cast<uint32_t>(listing.price * 0.1), // Calculate listing fee (10% of price)
            .transaction_fee = listing.transaction_fee,
            .state           = 0,
            .expire_date     = expire_date_opt,
            .created_date    = created_date_opt
        });
    }

    co_return result;
}

async::task<std::vector<marketplace::listing>> marketplace::get_listings(const std::vector<std::string>& listing_ids)
{
    this->_owner.assert_thread();

    if (listing_ids.empty())
        co_return std::vector<marketplace::listing>{};

    auto weak = this->_owner.weak_from_this_as<fb::game::character>();
    // Call get_listings API to get all listing data
    auto&& resp = co_await this->_owner.server.http.post("marketplace",
                                                         "/marketplace/get-listings",
                                                         mp_reqs::GetListings{listing_ids});

    co_await this->_owner.server.threads.switching(weak);

    if (resp.error != 0)
        throw std::runtime_error(std::format("Failed to get listings: error={}", resp.error));

    // Convert response listings to marketplace::listing
    std::vector<marketplace::listing> result;
    result.reserve(resp.listings.size());

    for (const auto& listing : resp.listings)
    {
        auto expire_date_opt = std::optional<fb::model::datetime>{};
        if (!listing.expire_date.empty())
            expire_date_opt = fb::model::datetime(listing.expire_date);

        auto created_date_opt = std::optional<fb::model::datetime>{};
        if (!listing.created_date.empty())
            created_date_opt = fb::model::datetime(listing.created_date);

        result.push_back(marketplace::listing{
            .id              = listing.id,
            .seller_id       = listing.seller_id,
            .buyer_id        = listing.buyer_id,
            .item_data       = {.owner       = listing.item.owner,
                                .model       = listing.item.model,
                                .count       = listing.item.count,
                                .durability  = listing.item.durability,
                                .custom_name = listing.item.custom_name},
            .price           = listing.price,
            .listing_fee     = static_cast<uint32_t>(listing.price * 0.1),
            .transaction_fee = listing.transaction_fee,
            .state           = static_cast<uint8_t>(listing.state),
            .expire_date     = expire_date_opt,
            .created_date    = created_date_opt
        });
    }

    co_return result;
}

void marketplace::set_pending_listings(std::unordered_map<std::string, pending_listing_info> pending_listings)
{
    this->_owner.assert_thread();

    // Move pending listings to internal storage
    this->_pending_listings = std::move(pending_listings);
}

async::task<void> marketplace::restore()
{
    this->_owner.assert_thread();

    if (this->_pending_listings.empty())
        co_return;

    auto weak = this->_owner.weak_from_this_as<fb::game::character>();
    if (weak.expired())
        co_return;

    // Collect all listing IDs
    auto listing_ids = std::vector<std::string>{};
    listing_ids.reserve(this->_pending_listings.size());
    for (const auto& [listing_id, pending_info] : this->_pending_listings)
    {
        listing_ids.push_back(listing_id);
    }

    // Check status of all pending listings in batch to get full listing data
    auto listings = std::vector<marketplace::listing>{};
    try
    {
        listings = co_await this->get_listings(listing_ids);
    }
    catch (std::exception& e)
    {
        fb::logger::warn("Failed to check marketplace listing status during restore: {}", e.what());
        co_return; // Exit immediately on HTTP failure
    }

    // Thread switching after HTTP call
    co_await this->_owner.server.threads.switching(weak);

    // Verify character is still valid after thread switching
    auto ch = weak.lock();
    if (ch == nullptr)
        co_return;

    // Create map of listing_id -> listing for quick lookup
    auto listing_map = std::unordered_map<std::string, marketplace::listing>{};
    for (const auto& listing : listings)
    {
        listing_map[listing.id] = listing;
    }

    // Process each pending listing
    auto to_remove = std::vector<std::string>{};
    for (const auto& [listing_id, pending_info] : this->_pending_listings)
    {
        auto it             = listing_map.find(listing_id);
        bool should_restore = false;

        if (it == listing_map.end())
        {
            // Listing does not exist on marketplace server - restore
            should_restore = true;
        }
        else
        {
            // Listing exists - check based on type
            const auto& listing = it->second;

            if (pending_info.type == pending_type::LIST)
            {
                // For LIST: Check if listing exists and seller_id matches
                if (listing.seller_id == pending_info.character_id)
                {
                    // Listing exists and seller_id matches - remove from pending (success)
                    to_remove.push_back(listing_id);
                    continue;
                }
                else
                {
                    // Listing exists but seller_id doesn't match - restore
                    should_restore = true;
                }
            }
            else if (pending_info.type == pending_type::PURCHASE)
            {
                // For PURCHASE: Check if listing is SOLD and buyer_id matches
                if (listing.buyer_id == pending_info.character_id &&
                    listing.state == static_cast<uint8_t>(mp::ListingState::SOLD))
                {
                    // Purchase succeeded - remove from pending (item will come via storage_box)
                    to_remove.push_back(listing_id);
                    continue;
                }
                else
                {
                    // Purchase failed (listing still active, buyer_id doesn't match, or not sold) - restore money
                    should_restore = true;
                }
            }
        }

        if (should_restore)
        {
            // Restore items/money to storage_box
            std::string title;
            std::string message;

            if (pending_info.type == pending_type::LIST)
            {
                title   = "Marketplace Listing Recovery";
                message = "Your marketplace listing failed. Items have been returned to your storage box.";
            }
            else
            {
                title   = "Marketplace Purchase Recovery";
                message = "Your marketplace purchase failed. Money has been returned to your storage box.";
            }

            // Create storage pending box with DSL attachments
            auto box = fb::game::storage_box::pending_box{
                .id          = listing_id, // Use listing_id as pending box ID
                .user        = this->_owner.id,
                .title       = title,
                .message     = message,
                .attachments = pending_info.dsls, // Copy DSLs (will be moved in apply_pending)
                .expire_date = std::nullopt       // No expiration date - keep until received
            };

            // Apply pending box to character's storage_box
            auto pending_boxes = std::vector<storage_box::pending_box>{};
            pending_boxes.push_back(std::move(box));
            this->_owner.storage_box.apply_pending(pending_boxes);

            // Log successful restore
            auto log_data            = Json::Value();
            log_data["character_id"] = static_cast<Json::Int64>(this->_owner.id);
            log_data["listing_id"]   = listing_id;
            log_data["type"]         = static_cast<Json::Int64>(pending_info.type);
            this->_owner.server.log.write("marketplace_restore_success", log_data);

            // Remove from pending listings
            to_remove.push_back(listing_id);
        }
        else
        {
            // Listing exists on server, removing pending (no log needed - normal case)
            to_remove.push_back(listing_id);
        }
    }

    // Remove processed listings
    for (const auto& listing_id : to_remove)
    {
        this->_pending_listings.erase(listing_id);
    }
}

const std::unordered_map<std::string, marketplace::pending_listing_info>& marketplace::pending_listings() const
{
    this->_owner.assert_thread();
    return this->_pending_listings;
}

void marketplace::listing::to_lua(fb::lua::context* lua) const
{
    lua->new_table();

    lua->pushstring("id");
    lua->pushstring(this->id);
    lua_settable(*lua, -3);

    lua->pushstring("seller_id");
    lua->pushinteger(this->seller_id);
    lua_settable(*lua, -3);

    lua->pushstring("buyer_id");
    lua->pushinteger(this->buyer_id);
    lua_settable(*lua, -3);

    lua->pushstring("item_data");
    lua->new_table();
    lua->pushstring("owner");
    lua->pushinteger(this->item_data.owner);
    lua_settable(*lua, -3);
    lua->pushstring("model");
    lua->pushinteger(this->item_data.model);
    lua_settable(*lua, -3);
    lua->pushstring("count");
    lua->pushinteger(this->item_data.count);
    lua_settable(*lua, -3);
    lua->pushstring("durability");
    if (this->item_data.durability.has_value())
    {
        lua->pushinteger(this->item_data.durability.value());
    }
    else
    {
        lua->pushnil();
    }
    lua_settable(*lua, -3);
    lua->pushstring("custom_name");
    if (this->item_data.custom_name.has_value())
    {
        lua->pushstring(this->item_data.custom_name.value());
    }
    else
    {
        lua->pushnil();
    }
    lua_settable(*lua, -3);
    lua_settable(*lua, -3);

    lua->pushstring("price");
    lua->pushinteger(this->price);
    lua_settable(*lua, -3);

    lua->pushstring("listing_fee");
    lua->pushinteger(this->listing_fee);
    lua_settable(*lua, -3);

    lua->pushstring("transaction_fee");
    lua->pushinteger(this->transaction_fee);
    lua_settable(*lua, -3);

    lua->pushstring("state");
    lua->pushinteger(this->state);
    lua_settable(*lua, -3);

    lua->pushstring("expire_date");
    if (this->expire_date.has_value())
    {
        lua->pushstring(this->expire_date.value().to_string());
    }
    else
    {
        lua->pushnil();
    }
    lua_settable(*lua, -3);

    lua->pushstring("created_date");
    if (this->created_date.has_value())
    {
        lua->pushstring(this->created_date.value().to_string());
    }
    else
    {
        lua->pushnil();
    }
    lua_settable(*lua, -3);
}