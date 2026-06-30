#include <fb/game/marketplace.h>
#include <fb/game/character.h>
#include <fb/game/server.h>
#include <fb/game/storage.h>
#include <fb/game/item/weapon.h>
#include <fb/protocol/flatbuffer/protocol.h>
#include <fb/model/model.h>
#include <fb/encoding.h>
#include <json/json.h>
#include <macro.h>
#include <stdexcept>
#include <sstream>
#include <unordered_set>
#include <algorithm>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

using namespace fb::game;

namespace mp      = fb::protocol::marketplace;
namespace mp_reqs = mp::request;
namespace mp_resp = mp::response;

marketplace::marketplace(character& owner) :
    _owner(owner)
{ }

std::string marketplace::generate_uuid()
{
    static auto gen  = boost::uuids::random_generator{};
    auto        uuid = gen();
    return boost::uuids::to_string(uuid);
}

async::task<marketplace::listing> marketplace::list(uint8_t slot, uint16_t count, uint32_t price, uint16_t expire_hours)
{
    this->_owner.assert_thread();

    auto weak = this->_owner.weak_from_this_as<fb::game::character>();
    if (weak.expired())
        throw std::runtime_error(_TEXT(MESSAGE_MARKETPLACE_CHARACTER_EXPIRED));

    // Generate UUID for listing_id
    auto id = generate_uuid();
    if (weak.expired())
        throw std::runtime_error(_TEXT(MESSAGE_MARKETPLACE_CHARACTER_EXPIRED));

    // Get item from inventory
    auto item = this->_owner.items.at(slot);
    if (item == nullptr)
        throw std::runtime_error(_TEXT(MESSAGE_MARKETPLACE_ITEM_NOT_FOUND_AT_INDEX));

    auto& model = item->based<fb::model::item>();
    if (item->count() < count)
        throw std::runtime_error(_TEXT(MESSAGE_MARKETPLACE_INSUFFICIENT_ITEM_COUNT));

    if (model.trade == false)
        throw std::runtime_error(_TEXT(MESSAGE_EXCEPTION_CANNOT_REGISTER_ITEM));

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

    // Calculate listing fee based on total sale amount (count * price)
    auto total_sale_amount = static_cast<uint32_t>(count * price);
    auto listing_fee = static_cast<uint32_t>(total_sale_amount * fb::model::const_value::marketplace::listing_fee);

    // Validate listing fee BEFORE API call
    if (this->_owner.money() < listing_fee)
        throw std::runtime_error(_TEXT(MESSAGE_MARKETPLACE_INSUFFICIENT_MONEY_FOR_LISTING_FEE));

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
                                                         .purchase_id  = id, // For list, purchase_id == listing_id
                                                         .listing_id   = id,
                                                         .dsls         = std::move(dsls),
                                                         .character_id = this->_owner.id,
                                                         .expected_purchase_count = 0,
                                                         .expected_total_price    = 0});

    // Remove item from inventory and deduct listing fee BEFORE API call
    std::ignore = this->_owner.items.remove(slot, count, ITEM_DELETE_TYPE::REMOVED);
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
        auto   world = fb::config<uint32_t>("world");
        auto&& resp  = co_await this->_owner.server.http.post(
            "marketplace",
            "/marketplace/list",
            mp_reqs::List{
                world,
                this->_owner.id,
                id,
                mp::Item{this->_owner.id, model.id, count, durability, custom_name},
                price
        });

        co_await this->_owner.server.threads.switching(weak);

        if (resp.error != 0)
        {
            // API call returned an error - throw exception to handle in catch block
            throw std::runtime_error(std::format(_TEXT(MESSAGE_MARKETPLACE_FAILED_TO_LIST_ITEM),
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
            .id           = resp.listing_id,
            .seller_id    = this->_owner.id,
            .item_data    = {.owner       = this->_owner.id,
                             .model       = model.id,
                             .count       = count,
                             .durability  = durability,
                             .custom_name = custom_name},
            .price        = price,
            .listing_fee  = listing_fee,
            .state        = 0,
            .expire_date  = std::nullopt,
            .created_date = std::nullopt
        };
    }
    catch (const std::exception& e)
    {
        if (weak.expired())
            throw std::runtime_error(_TEXT(MESSAGE_MARKETPLACE_CHARACTER_EXPIRED));

        // Any failure (system error or logical error) - items/money already deducted, keep DSL for recovery
        // DSL remains in _pending_listings for recovery on next login
        auto log_data            = Json::Value();
        log_data["character_id"] = static_cast<Json::Int64>(this->_owner.id);
        log_data["listing_id"]   = id;
        log_data["error"]        = e.what();
        this->_owner.server.log.write("marketplace_list_failed", log_data);
        throw std::runtime_error(std::format(_TEXT(MESSAGE_MARKETPLACE_FAILED_TO_LIST_ITEM_WITH_ERROR), e.what()));
    }
}

async::task<bool> marketplace::cancel(std::string_view id)
{
    this->_owner.assert_thread();

    // Copy id to avoid coroutine lifetime issues
    auto id_copy = std::string(id);

    // Log before API call
    auto log_data_before            = Json::Value();
    log_data_before["character_id"] = static_cast<Json::Int64>(this->_owner.id);
    log_data_before["listing_id"]   = id_copy;
    this->_owner.server.log.write("marketplace_cancel", log_data_before);

    auto   weak  = this->_owner.weak_from_this_as<fb::game::character>();
    auto   world = fb::config<uint32_t>("world");
    auto   req   = mp_reqs::Cancel{world, this->_owner.id, id_copy};
    auto&& resp  = co_await this->_owner.server.http.post("marketplace", "/marketplace/cancel", req);

    co_await this->_owner.server.threads.switching(weak);

    if (resp.error != 0)
    {
        auto log_data            = Json::Value();
        log_data["character_id"] = static_cast<Json::Int64>(this->_owner.id);
        log_data["listing_id"]   = id_copy;
        log_data["error"]        = static_cast<Json::Int64>(resp.error);
        this->_owner.server.log.write("marketplace_cancel_failed", log_data);
        throw std::runtime_error(std::format(_TEXT(MESSAGE_MARKETPLACE_FAILED_TO_CANCEL_LISTING), resp.error));
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

async::task<marketplace::listing> marketplace::purchase(std::string_view listing_id, uint16_t purchase_count)
{
    this->_owner.assert_thread();

    // Copy id to avoid coroutine lifetime issues
    auto listing_id_copy = std::string(listing_id);

    // Generate UUID for purchase_id
    auto purchase_id = generate_uuid();

    // Get listing info first (needed for price validation and timeout recovery)
    auto weak        = this->_owner.weak_from_this_as<fb::game::character>();
    auto listing_ids = std::vector<std::string>{listing_id_copy};
    auto listings    = co_await this->get_listings(listing_ids);

    if (listings.empty())
        throw std::runtime_error(_TEXT(MESSAGE_MARKETPLACE_LISTING_NOT_FOUND));

    auto& listing = listings[0];
    if (listing.state != static_cast<uint8_t>(mp::ListingState::ACTIVE))
        throw std::runtime_error(_TEXT(MESSAGE_MARKETPLACE_LISTING_NOT_AVAILABLE_FOR_PURCHASE));

    // Calculate expected price (per unit price * purchase_count)
    auto expected_price = listing.price * purchase_count;

    // Validate money BEFORE API call
    if (this->_owner.money() < expected_price)
        throw std::runtime_error(_TEXT(MESSAGE_MARKETPLACE_INSUFFICIENT_MONEY));

    // Prepare DSL for pending purchase recovery (money only) BEFORE API call
    auto dsls = std::vector<fb::model::dsl>{};
    if (expected_price > 0)
    {
        auto money_dsl = fb::model::dsl::money(expected_price);
        dsls.push_back(money_dsl.to_dsl());
    }

    // Store pending purchase for recovery in case of server failure (use purchase_id as key)
    this->_pending_listings.emplace(purchase_id,
                                    pending_listing_info{.type                    = pending_type::PURCHASE,
                                                         .purchase_id             = purchase_id,
                                                         .listing_id              = listing_id_copy,
                                                         .dsls                    = std::move(dsls),
                                                         .character_id            = this->_owner.id,
                                                         .expected_purchase_count = purchase_count,
                                                         .expected_total_price    = expected_price});

    // Deduct money BEFORE API call
    this->_owner.money_reduce(expected_price);

    // Log before API call (after deduction)
    auto log_data_before              = Json::Value();
    log_data_before["character_id"]   = static_cast<Json::Int64>(this->_owner.id);
    log_data_before["listing_id"]     = listing_id_copy;
    log_data_before["purchase_id"]    = purchase_id;
    log_data_before["seller_id"]      = static_cast<Json::Int64>(listing.seller_id);
    log_data_before["purchase_count"] = static_cast<Json::Int64>(purchase_count);
    log_data_before["expected_price"] = static_cast<Json::Int64>(expected_price);
    this->_owner.server.log.write("marketplace_purchase", log_data_before);

    // Send purchase request to marketplace server
    auto world           = fb::config<uint32_t>("world");
    auto unhandled_error = true;

    try
    {
        auto   req  = mp_reqs::Purchase{world, this->_owner.id, listing_id_copy, purchase_count, purchase_id};
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
            throw std::runtime_error(
                std::format(_TEXT(MESSAGE_MARKETPLACE_FAILED_TO_PURCHASE_ITEM), enum_tostring(ec)));
        }

        // Purchase succeeded - remove from pending_listings (use purchase_id as key)
        this->_pending_listings.erase(purchase_id);

        // Log successful purchase
        auto log_data_success             = Json::Value();
        log_data_success["character_id"]  = static_cast<Json::Int64>(this->_owner.id);
        log_data_success["listing_id"]    = listing_id_copy;
        log_data_success["purchase_id"]   = purchase_id;
        log_data_success["actual_count"]  = static_cast<Json::Int64>(resp.actual_purchase_count);
        log_data_success["refund_amount"] = static_cast<Json::Int64>(resp.refund_amount);
        this->_owner.server.log.write("marketplace_purchase_success", log_data_success);

        // Build and return listing (from response item data)
        co_return marketplace::listing{
            .id           = listing_id_copy,
            .seller_id    = 0, // Not provided in response
            .item_data    = {.owner       = resp.item.owner,
                             .model       = resp.item.model,
                             .count       = resp.item.count,
                             .durability  = resp.item.durability,
                             .custom_name = resp.item.custom_name},
            .price        = 0, // Not provided in response
            .listing_fee  = 0,
            .state        = 0,
            .expire_date  = std::nullopt,
            .created_date = std::nullopt,
            .purchase     = std::nullopt
        };
    }
    catch (const std::exception& e)
    {
        if (weak.expired())
            throw std::runtime_error(_TEXT(MESSAGE_MARKETPLACE_CHARACTER_EXPIRED));

        if (unhandled_error)
        {
            // HTTP exception (timeout, connection error, etc.) - system error
            // Money already deducted, DSL is already saved for recovery
            auto log_data            = Json::Value();
            log_data["character_id"] = static_cast<Json::Int64>(this->_owner.id);
            log_data["listing_id"]   = listing_id_copy;
            log_data["purchase_id"]  = purchase_id;
            log_data["error"]        = e.what();
            this->_owner.server.log.write("marketplace_purchase_failed", log_data);
        }
        else
        {
            // Logical error - restore money
            this->_owner.money_add(expected_price);
            this->_pending_listings.erase(purchase_id); // Remove DSL as purchase definitely failed
            auto log_data            = Json::Value();
            log_data["character_id"] = static_cast<Json::Int64>(this->_owner.id);
            log_data["listing_id"]   = listing_id_copy;
            log_data["purchase_id"]  = purchase_id;
            log_data["error"]        = e.what();
            this->_owner.server.log.write("marketplace_purchase_failed", log_data);
        }

        throw std::runtime_error(std::format(_TEXT(MESSAGE_MARKETPLACE_FAILED_TO_PURCHASE_ITEM_WITH_ERROR), e.what()));
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
        throw std::runtime_error(std::format(_TEXT(MESSAGE_MARKETPLACE_FAILED_TO_SEARCH_ITEMS),
                                             enum_tostring((fb::model::enum_value::ERROR_CODE)resp.error)));

    auto result = search_result{.listings = {}, .total_count = resp.result.total_count, .page = resp.result.page};
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
            .id          = listing.id,
            .seller_id   = listing.seller_id,
            .item_data   = {.owner       = listing.item.owner,
                            .model       = listing.item.model,
                            .count       = listing.item.count,
                            .durability  = listing.item.durability,
                            .custom_name = listing.item.custom_name},
            .price       = listing.price,
            .listing_fee = static_cast<uint32_t>(
                listing.item.count * listing.price *
                fb::model::const_value::marketplace::listing_fee), // Calculate listing fee based on total sale amount
            .state        = 0,
            .expire_date  = expire_date_opt,
            .created_date = created_date_opt,
            .purchase     = std::nullopt
        });
    }

    co_return result;
}

async::task<std::vector<marketplace::listing>>
marketplace::get_listings(const marketplace::string_vector_t& listing_ids, uint32_t buyer_id)
{
    this->_owner.assert_thread();

    if (listing_ids.empty())
        co_return std::vector<marketplace::listing>{};

    auto weak = this->_owner.weak_from_this_as<fb::game::character>();
    // Call get_listings API to get all listing data (with optional buyer_id for purchase_info)
    auto&& resp = co_await this->_owner.server.http.post(
        "marketplace",
        "/marketplace/get-listings",
        mp_reqs::GetListings{listing_ids, buyer_id > 0 ? std::optional<uint32_t>(buyer_id) : std::nullopt});

    co_await this->_owner.server.threads.switching(weak);

    if (resp.error != 0)
        throw std::runtime_error(std::format(_TEXT(MESSAGE_MARKETPLACE_FAILED_TO_GET_LISTINGS), resp.error));

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

        auto purchase_info_opt = std::optional<marketplace::purchase_info>{};
        if (listing.purchase_info.has_value())
        {
            const auto& pi                        = listing.purchase_info.value();
            auto        purchase_created_date_opt = std::optional<fb::model::datetime>{};
            if (!pi.created_date.empty())
                purchase_created_date_opt = fb::model::datetime(pi.created_date);

            purchase_info_opt = marketplace::purchase_info{.purchase_count = pi.purchase_count,
                                                           .purchase_price = pi.purchase_price,
                                                           .created_date   = purchase_created_date_opt};
        }

        result.push_back(marketplace::listing{
            .id          = listing.id,
            .seller_id   = listing.seller_id,
            .item_data   = {.owner       = listing.item.owner,
                            .model       = listing.item.model,
                            .count       = listing.item.count,
                            .durability  = listing.item.durability,
                            .custom_name = listing.item.custom_name},
            .price       = listing.price,
            .listing_fee = static_cast<uint32_t>(
                listing.item.count * listing.price *
                fb::model::const_value::marketplace::listing_fee), // Calculate listing fee based on total sale amount
            .state        = static_cast<uint8_t>(listing.state),
            .expire_date  = expire_date_opt,
            .created_date = created_date_opt,
            .purchase     = purchase_info_opt
        });
    }

    co_return result;
}

async::task<marketplace::purchase_map_t> marketplace::get_purchases(const marketplace::string_vector_t& purchase_ids)
{
    this->_owner.assert_thread();

    if (purchase_ids.empty())
        co_return std::unordered_map<std::string, marketplace::purchase_info>{};

    auto weak = this->_owner.weak_from_this_as<fb::game::character>();
    // Call get_purchases API to get purchase records
    auto&& resp = co_await this->_owner.server.http.post("marketplace",
                                                         "/marketplace/get-purchases",
                                                         mp_reqs::GetPurchases{purchase_ids});

    co_await this->_owner.server.threads.switching(weak);

    if (resp.error != 0)
        throw std::runtime_error(std::format(_TEXT(MESSAGE_MARKETPLACE_FAILED_TO_GET_LISTINGS), resp.error));

    // Convert response purchases to marketplace::purchase_info map
    std::unordered_map<std::string, marketplace::purchase_info> result;

    for (const auto& purchase : resp.purchases)
    {
        auto created_date_opt = std::optional<fb::model::datetime>{};
        if (!purchase.created_date.empty())
            created_date_opt = fb::model::datetime(purchase.created_date);

        result.emplace(purchase.id,
                       marketplace::purchase_info{.purchase_count = purchase.purchase_count,
                                                  .purchase_price = purchase.purchase_price,
                                                  .created_date   = created_date_opt});
    }

    co_return result;
}

void marketplace::set_pending_listings(marketplace::pending_listings_t pending_listings)
{
    this->_owner.assert_thread();

    // Move pending listings to internal storage
    this->_pending_listings = std::move(pending_listings);
}

std::string marketplace::attachments_to_json(const std::vector<fb::model::dsl>& attachments)
{
    if (attachments.empty())
        return "[]";

    auto json_array = Json::Value{Json::arrayValue};
    for (const auto& dsl : attachments)
    {
        json_array.append(dsl.to_json());
    }

    auto builder           = Json::StreamWriterBuilder{};
    builder["emitUTF8"]    = true;
    builder["indentation"] = "";
    auto writer            = std::unique_ptr<Json::StreamWriter>(builder.newStreamWriter());
    auto stream            = std::ostringstream{};
    writer->write(json_array, &stream);
    return stream.str();
}

std::vector<fb::protocol::internal::MarketplacePending> marketplace::to_save_dtos() const
{
    this->_owner.assert_thread();

    auto dtos = std::vector<fb::protocol::internal::MarketplacePending>{};
    dtos.reserve(this->_pending_listings.size());

    for (const auto& [key, info] : this->_pending_listings)
    {
        if (info.dsls.empty())
            continue;

        dtos.push_back(fb::protocol::internal::MarketplacePending{this->_owner.id,
                                                                  key,
                                                                  static_cast<uint8_t>(info.type),
                                                                  info.purchase_id,
                                                                  info.listing_id,
                                                                  attachments_to_json(info.dsls),
                                                                  info.expected_purchase_count,
                                                                  info.expected_total_price,
                                                                  info.character_id});
    }

    return dtos;
}

async::task<void> marketplace::restore()
{
    this->_owner.assert_thread();

    if (this->_pending_listings.empty())
        co_return;

    auto weak = this->_owner.weak_from_this_as<fb::game::character>();
    if (weak.expired())
        co_return;

    // Separate pending items by type
    auto purchase_pending = std::vector<std::pair<std::string, pending_listing_info>>{}; // purchase_id -> pending_info
    auto list_pending     = std::vector<std::pair<std::string, pending_listing_info>>{}; // listing_id -> pending_info
    auto listing_ids_for_purchase = std::vector<std::string>{};
    auto purchase_ids             = std::vector<std::string>{};

    for (const auto& [key, pending_info] : this->_pending_listings)
    {
        if (pending_info.type == pending_type::PURCHASE)
        {
            purchase_pending.emplace_back(key, pending_info);
            purchase_ids.push_back(pending_info.purchase_id);
            listing_ids_for_purchase.push_back(pending_info.listing_id);
        }
        else // LIST
        {
            list_pending.emplace_back(key, pending_info);
            listing_ids_for_purchase.push_back(pending_info.listing_id);
        }
    }

    // Remove duplicates from listing_ids_for_purchase
    std::sort(listing_ids_for_purchase.begin(), listing_ids_for_purchase.end());
    listing_ids_for_purchase.erase(std::unique(listing_ids_for_purchase.begin(), listing_ids_for_purchase.end()),
                                   listing_ids_for_purchase.end());

    // Check status of all pending listings in batch (with buyer_id for purchase_info)
    auto listings = std::vector<marketplace::listing>{};
    try
    {
        listings = co_await this->get_listings(listing_ids_for_purchase, this->_owner.id);
    }
    catch (std::exception& e)
    {
        fb::logger::warn("Failed to check marketplace listing status during restore: {}", e.what());
        co_return; // Exit immediately on HTTP failure
    }

    // Get purchase records for purchase pending items
    auto purchases = std::unordered_map<std::string, marketplace::purchase_info>{};
    if (!purchase_ids.empty())
    {
        try
        {
            purchases = co_await this->get_purchases(purchase_ids);
        }
        catch (std::exception& e)
        {
            fb::logger::warn("Failed to get purchase records during restore: {}", e.what());
            // Continue with listing-based recovery
        }
    }

    // Thread switching after HTTP calls
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

    // Process LIST pending items
    auto to_remove = std::vector<std::string>{};
    for (const auto& [listing_id, pending_info] : list_pending)
    {
        auto it = listing_map.find(pending_info.listing_id);
        if (it != listing_map.end() && it->second.seller_id == pending_info.character_id)
        {
            // Listing exists and seller_id matches - remove from pending (success)
            to_remove.push_back(listing_id);
        }
        else
        {
            // Listing does not exist or seller_id doesn't match - restore
            std::string title   = _TEXT(MESSAGE_MARKETPLACE_LISTING_RECOVERY_TITLE);
            std::string message = _TEXT(MESSAGE_MARKETPLACE_LISTING_RECOVERY_MESSAGE);

            co_await this->_owner.server.system_storage.create(this->_owner.id,
                                                               std::format("marketplace:list:{}", listing_id),
                                                               title,
                                                               message,
                                                               pending_info.dsls);

            auto log_data            = Json::Value();
            log_data["character_id"] = static_cast<Json::Int64>(this->_owner.id);
            log_data["listing_id"]   = listing_id;
            log_data["type"]         = static_cast<Json::Int64>(pending_info.type);
            this->_owner.server.log.write("marketplace_restore_success", log_data);

            to_remove.push_back(listing_id);
        }
    }

    // Process PURCHASE pending items
    for (const auto& [purchase_id, pending_info] : purchase_pending)
    {
        auto purchase_it = purchases.find(pending_info.purchase_id);
        if (purchase_it != purchases.end())
        {
            // Purchase record exists - check for refund if needed
            const auto& purchase      = purchase_it->second;
            auto        refund_amount = 0u;

            if (purchase.purchase_count < pending_info.expected_purchase_count)
            {
                // Partial purchase - calculate refund
                auto actual_price = purchase.purchase_price;
                refund_amount     = pending_info.expected_total_price - actual_price;

                if (refund_amount > 0)
                {
                    // Add refund to DSLs
                    auto refund_dsl = fb::model::dsl::money(refund_amount);
                    auto dsls_copy  = pending_info.dsls;
                    dsls_copy.push_back(refund_dsl.to_dsl());

                    co_await this->_owner.server.system_storage.create(
                        this->_owner.id,
                        std::format("marketplace:purchase:refund:{}", purchase_id),
                        _TEXT(MESSAGE_MARKETPLACE_PURCHASE_REFUND_TITLE),
                        _TEXT(MESSAGE_MARKETPLACE_PURCHASE_REFUND_MESSAGE),
                        dsls_copy);

                    auto log_data             = Json::Value();
                    log_data["character_id"]  = static_cast<Json::Int64>(this->_owner.id);
                    log_data["purchase_id"]   = purchase_id;
                    log_data["listing_id"]    = pending_info.listing_id;
                    log_data["refund_amount"] = static_cast<Json::Int64>(refund_amount);
                    this->_owner.server.log.write("marketplace_purchase_refund", log_data);
                }
            }

            // Purchase succeeded - remove from pending (item already in storage_box from marketplace server)
            to_remove.push_back(purchase_id);
        }
        else
        {
            // Purchase record does not exist - restore money
            co_await this->_owner.server.system_storage.create(this->_owner.id,
                                                               std::format("marketplace:purchase:{}", purchase_id),
                                                               _TEXT(MESSAGE_MARKETPLACE_PURCHASE_RECOVERY_TITLE),
                                                               _TEXT(MESSAGE_MARKETPLACE_PURCHASE_RECOVERY_MESSAGE),
                                                               pending_info.dsls);

            auto log_data            = Json::Value();
            log_data["character_id"] = static_cast<Json::Int64>(this->_owner.id);
            log_data["purchase_id"]  = purchase_id;
            log_data["listing_id"]   = pending_info.listing_id;
            log_data["type"]         = static_cast<Json::Int64>(pending_info.type);
            this->_owner.server.log.write("marketplace_restore_success", log_data);

            to_remove.push_back(purchase_id);
        }
    }

    // Remove processed listings
    for (const auto& key : to_remove)
    {
        this->_pending_listings.erase(key);
    }
}

const marketplace::pending_listings_t& marketplace::pending_listings() const
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