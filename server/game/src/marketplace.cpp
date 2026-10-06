#include <fb/game/marketplace.h>

#include <fb/encoding.h>
#include <fb/game/character.h>
#include <fb/game/item/weapon.h>
#include <fb/game/server.h>
#include <fb/game/storage.h>
#include <fb/model/model.h>
#include <fb/protocol/flatbuffer/protocol.h>
#include <macro.h>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <json/json.h>

#include <algorithm>
#include <cstdint>
#include <exception>
#include <format>
#include <memory>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

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

async::task<marketplace::listing>
marketplace::list(uint8_t slot, uint32_t model_id, uint16_t count, uint64_t price, uint16_t expire_hours)
{
    this->_owner.assert_thread();

    auto weak = this->_owner.weak_from_this_as<fb::game::character>();
    if (weak.expired())
        throw std::runtime_error(_TEXT(MESSAGE_MARKETPLACE_CHARACTER_EXPIRED));

    if (this->_owner.items.reserved(slot))
        throw std::runtime_error(_TEXT(MESSAGE_MARKETPLACE_ITEM_LOCKED));

    // The slot can be swapped while the listing dialog is open, so it must still hold the item the seller chose.
    auto item = this->_owner.items.at(slot);
    if (item == nullptr || item->model().id != model_id)
        throw std::runtime_error(_TEXT(MESSAGE_MARKETPLACE_ITEM_NOT_FOUND_AT_INDEX));

    auto& model = item->model();
    if (item->count() - item->trade_count() < count)
        throw std::runtime_error(_TEXT(MESSAGE_MARKETPLACE_INSUFFICIENT_ITEM_COUNT));

    if (model.trade == false)
        throw std::runtime_error(_TEXT(MESSAGE_EXCEPTION_CANNOT_REGISTER_ITEM));

    auto durability  = item->durability();
    auto custom_name = std::optional<std::string>{};
    if (model.attr(ITEM_ATTRIBUTE::WEAPON))
    {
        auto weapon = static_cast<fb::game::weapon*>(item.get());
        if (weapon != nullptr && weapon->custom_name().has_value())
            custom_name = weapon->custom_name().value();
    }

    auto total_sale_amount = static_cast<uint64_t>(count) * price;
    auto listing_fee = static_cast<uint64_t>(total_sale_amount * fb::model::const_value::marketplace::listing_fee);
    if (this->_owner.money() < listing_fee)
        throw std::runtime_error(_TEXT(MESSAGE_MARKETPLACE_INSUFFICIENT_MONEY_FOR_LISTING_FEE));

    auto id = generate_uuid();
    if (this->_owner.items.lock(slot, count, listing_fee, id) == false)
        throw std::runtime_error(std::format(_TEXT(MESSAGE_MARKETPLACE_FAILED_TO_LIST_ITEM_WITH_ERROR), "lock"));
    this->_listing.insert(id);

    auto log_data_before            = Json::Value();
    log_data_before["character_id"] = static_cast<Json::Int64>(this->_owner.id);
    log_data_before["listing_id"]   = id;
    log_data_before["item_model"]   = model.id;
    log_data_before["item_count"]   = count;
    log_data_before["price"]        = static_cast<Json::Int64>(price);
    log_data_before["listing_fee"]  = static_cast<Json::Int64>(listing_fee);
    this->_owner.server.log.write("marketplace_list", log_data_before);

    // The escrow must be durable before the request leaves: a crash after it leaves is settled by abort-list on
    // the next login, which needs the escrow row to exist.
    auto saved      = false;
    auto error_what = std::string{};
    try
    {
        saved = co_await this->_owner.server.save(this->_owner);
        co_await this->_owner.server.threads.switching(weak);
    }
    catch (const std::exception& e)
    {
        error_what = e.what();
    }

    if (weak.expired())
        throw std::runtime_error(_TEXT(MESSAGE_MARKETPLACE_CHARACTER_EXPIRED));

    if (saved == false)
    {
        co_await this->_owner.server.threads.switching(weak);
        this->_listing.erase(id);
        this->_owner.items.unlock(id);

        auto log_data            = Json::Value();
        log_data["character_id"] = static_cast<Json::Int64>(this->_owner.id);
        log_data["listing_id"]   = id;
        log_data["error"]        = error_what.empty() ? std::string{"save failed"} : error_what;
        this->_owner.server.log.write("marketplace_list_failed", log_data);
        throw std::runtime_error(std::format(_TEXT(MESSAGE_MARKETPLACE_FAILED_TO_LIST_ITEM_WITH_ERROR), "save"));
    }

    auto error_code = fb::model::enum_value::ERROR_CODE::NONE;
    auto unknown    = false;
    try
    {
        auto   world = this->_owner.world();
        auto&& resp  = co_await this->_owner.server.http.post(
            weak,
            "marketplace",
            "/marketplace/list",
            mp_reqs::List{
                world,
                this->_owner.id,
                id,
                mp::Item{this->_owner.id, model.id, count, durability, custom_name},
                price
        });
        error_code = static_cast<fb::model::enum_value::ERROR_CODE>(resp.error);
        switch (error_code)
        {
        case fb::model::enum_value::ERROR_CODE::NONE:
        case fb::model::enum_value::ERROR_CODE::MARKETPLACE_ID_ALREADY_EXISTS:
        case fb::model::enum_value::ERROR_CODE::MARKETPLACE_ITEM_NOT_FOUND:
        case fb::model::enum_value::ERROR_CODE::MARKETPLACE_ITEM_NOT_TRADEABLE:
        case fb::model::enum_value::ERROR_CODE::MARKETPLACE_LISTING_LIMIT_EXCEEDED:
        case fb::model::enum_value::ERROR_CODE::MARKETPLACE_LISTING_NOT_FOUND:
            break;

        default:
            unknown    = true;
            error_what = enum_tostring(error_code);
            break;
        }
    }
    catch (const std::exception& e)
    {
        unknown    = true;
        error_what = e.what();
    }

    if (weak.expired())
        throw std::runtime_error(_TEXT(MESSAGE_MARKETPLACE_CHARACTER_EXPIRED));

    this->_listing.erase(id);

    if (unknown)
    {
        // The listing may or may not exist, so the escrow stays until abort-list settles it.
        auto log_data            = Json::Value();
        log_data["character_id"] = static_cast<Json::Int64>(this->_owner.id);
        log_data["listing_id"]   = id;
        log_data["error"]        = error_what;
        this->_owner.server.log.write("marketplace_list_pending", log_data);
        throw std::runtime_error(_TEXT(MESSAGE_MARKETPLACE_LISTING_PENDING));
    }
    else if (error_code != fb::model::enum_value::ERROR_CODE::NONE)
    {
        this->_owner.items.unlock(id);

        auto log_data            = Json::Value();
        log_data["character_id"] = static_cast<Json::Int64>(this->_owner.id);
        log_data["listing_id"]   = id;
        log_data["error"]        = enum_tostring(error_code);
        this->_owner.server.log.write("marketplace_list_failed", log_data);
        throw std::runtime_error(
            std::format(_TEXT(MESSAGE_MARKETPLACE_FAILED_TO_LIST_ITEM), enum_tostring(error_code)));
    }
    else
    {
        this->_owner.items.deduct(id);

        auto log_data_success            = Json::Value();
        log_data_success["character_id"] = static_cast<Json::Int64>(this->_owner.id);
        log_data_success["listing_id"]   = id;
        this->_owner.server.log.write("marketplace_list_success", log_data_success);

        co_return marketplace::listing{
            .id           = id,
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
    auto   world = this->_owner.world();
    auto   req   = mp_reqs::Cancel{world, this->_owner.id, id_copy};
    auto&& resp  = co_await this->_owner.server.http.post(weak, "marketplace", "/marketplace/cancel", req);

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

    auto dsls = std::vector<fb::model::dsl>{};
    if (expected_price > 0)
    {
        auto money_dsl = fb::model::dsl::money(expected_price);
        dsls.push_back(money_dsl.to_dsl());
    }

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
    auto world           = this->_owner.world();
    auto unhandled_error = true;
    auto restore_money   = false;
    auto character_gone  = false;
    auto error_what      = std::string{};

    try
    {
        auto   req  = mp_reqs::Purchase{world, this->_owner.id, listing_id_copy, purchase_count, purchase_id};
        auto&& resp = co_await this->_owner.server.http.post(weak, "marketplace", "/marketplace/purchase", req);

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
        {
            character_gone = true;
        }
        else
        {
            error_what = e.what();
            if (unhandled_error)
            {
                this->_pending_listings.emplace(purchase_id,
                                                pending_listing_info{.type                    = pending_type::PURCHASE,
                                                                     .purchase_id             = purchase_id,
                                                                     .listing_id              = listing_id_copy,
                                                                     .dsls                    = std::move(dsls),
                                                                     .character_id            = this->_owner.id,
                                                                     .expected_purchase_count = purchase_count,
                                                                     .expected_total_price    = expected_price});
                auto log_data            = Json::Value();
                log_data["character_id"] = static_cast<Json::Int64>(this->_owner.id);
                log_data["listing_id"]   = listing_id_copy;
                log_data["purchase_id"]  = purchase_id;
                log_data["error"]        = error_what;
                this->_owner.server.log.write("marketplace_purchase_failed", log_data);
            }
            else
            {
                restore_money            = true;
                auto log_data            = Json::Value();
                log_data["character_id"] = static_cast<Json::Int64>(this->_owner.id);
                log_data["listing_id"]   = listing_id_copy;
                log_data["purchase_id"]  = purchase_id;
                log_data["error"]        = error_what;
                this->_owner.server.log.write("marketplace_purchase_failed", log_data);
            }
        }
    }

    if (character_gone)
        throw std::runtime_error(_TEXT(MESSAGE_MARKETPLACE_CHARACTER_EXPIRED));

    if (restore_money)
        std::ignore = this->_owner.money_add(expected_price);

    throw std::runtime_error(std::format(_TEXT(MESSAGE_MARKETPLACE_FAILED_TO_PURCHASE_ITEM_WITH_ERROR), error_what));
}

async::task<marketplace::search_result> marketplace::search(const search_option& option)
{
    this->_owner.assert_thread();

    auto   weak = this->_owner.weak_from_this_as<fb::game::character>();
    auto&& resp = co_await this->_owner.server.http.post(weak,
                                                         "marketplace",
                                                         "/marketplace/search",
                                                         mp_reqs::Search{option.item_name,
                                                                         option.min_price,
                                                                         option.max_price,
                                                                         option.seller_id,
                                                                         option.sort_by,
                                                                         option.page});

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
            .id        = listing.id,
            .seller_id = listing.seller_id,
            .item_data = {.owner       = listing.item.owner,
                          .model       = listing.item.model,
                          .count       = listing.item.count,
                          .durability  = listing.item.durability,
                          .custom_name = listing.item.custom_name},
            .price     = listing.price,
            .listing_fee =
                static_cast<uint64_t>(static_cast<double>(static_cast<uint64_t>(listing.item.count) * listing.price) *
                                      fb::model::const_value::marketplace::listing_fee),
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
        weak,
        "marketplace",
        "/marketplace/get-listings",
        mp_reqs::GetListings{listing_ids, buyer_id > 0 ? std::optional<uint32_t>(buyer_id) : std::nullopt});

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
            .id        = listing.id,
            .seller_id = listing.seller_id,
            .item_data = {.owner       = listing.item.owner,
                          .model       = listing.item.model,
                          .count       = listing.item.count,
                          .durability  = listing.item.durability,
                          .custom_name = listing.item.custom_name},
            .price     = listing.price,
            .listing_fee =
                static_cast<uint64_t>(static_cast<double>(static_cast<uint64_t>(listing.item.count) * listing.price) *
                                      fb::model::const_value::marketplace::listing_fee),
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
    auto&& resp = co_await this->_owner.server.http.post(weak,
                                                         "marketplace",
                                                         "/marketplace/get-purchases",
                                                         mp_reqs::GetPurchases{purchase_ids});

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

    if (this->_restoring)
        co_return;

    auto weak = this->_owner.weak_from_this_as<fb::game::character>();
    if (weak.expired())
        co_return;

    auto world = this->_owner.world();

    // Escrows and legacy LIST pendings are settled by abort-list, which also finds archived listings.
    auto abort_targets = std::vector<std::tuple<std::string, mp::Item, bool>>{}; // listing_id, item, legacy
    for (auto i : this->_owner.items.escrow_indices())
    {
        auto escrow = this->_owner.items.escrow(i);
        if (this->_listing.contains(escrow->listing_id))
            continue;

        auto& item        = *escrow->item;
        auto  custom_name = std::optional<std::string>{};
        if (item.model().attr(ITEM_ATTRIBUTE::WEAPON))
            custom_name = static_cast<fb::game::weapon&>(item).custom_name();

        abort_targets.emplace_back(
            escrow->listing_id,
            mp::Item{this->_owner.id, item.model().id, item.count(), item.durability(), custom_name},
            false);
    }

    auto purchase_pending = std::vector<std::pair<std::string, pending_listing_info>>{};
    auto purchase_ids     = std::vector<std::string>{};
    for (const auto& [key, pending_info] : this->_pending_listings)
    {
        if (pending_info.type == pending_type::PURCHASE)
        {
            purchase_pending.emplace_back(key, pending_info);
            purchase_ids.push_back(pending_info.purchase_id);
        }
        else
        {
            auto item = mp::Item{this->_owner.id, 0, 0, std::nullopt, std::nullopt};
            for (const auto& dsl : pending_info.dsls)
            {
                if (dsl.header != fb::model::enum_value::DSL::item)
                    continue;

                auto params      = fb::model::dsl::item(dsl.params);
                item.model       = params.id;
                item.count       = static_cast<uint16_t>(params.count);
                item.durability  = params.durability;
                item.custom_name = params.custom_name;
            }
            abort_targets.emplace_back(key, item, true);
        }
    }

    if (abort_targets.empty() && purchase_pending.empty())
        co_return;

    this->_restoring = true;

    for (auto& [listing_id, item, legacy] : abort_targets)
    {
        auto created = std::optional<bool>{};
        try
        {
            auto&& resp =
                co_await this->_owner.server.http.post(weak,
                                                       "marketplace",
                                                       "/marketplace/abort-list",
                                                       mp_reqs::AbortList{world, this->_owner.id, listing_id, item, 0});
            if (resp.error == 0)
                created = resp.created;
            else
                fb::logger::warn("abort-list {} failed: {}", listing_id, resp.error);
        }
        catch (std::exception& e)
        {
            fb::logger::warn("abort-list {} failed: {}", listing_id, e.what());
        }

        if (weak.expired())
            throw std::runtime_error(_TEXT(MESSAGE_MARKETPLACE_CHARACTER_EXPIRED));

        if (created.has_value() == false)
            continue;

        if (legacy && created.value() == false)
        {
            std::ignore =
                co_await this->_owner.server.system_storage.create(this->_owner.world(),
                                                                   this->_owner.id,
                                                                   std::format("marketplace:list:{}", listing_id),
                                                                   _TEXT(MESSAGE_MARKETPLACE_LISTING_RECOVERY_TITLE),
                                                                   _TEXT(MESSAGE_MARKETPLACE_LISTING_RECOVERY_MESSAGE),
                                                                   this->_pending_listings[listing_id].dsls);
            co_await this->_owner.server.threads.switching(weak);
        }

        if (legacy)
            this->_pending_listings.erase(listing_id);
        else if (created.value())
            this->_owner.items.deduct(listing_id);
        else
            this->_owner.items.unlock(listing_id);

        auto log_data            = Json::Value();
        log_data["character_id"] = static_cast<Json::Int64>(this->_owner.id);
        log_data["listing_id"]   = listing_id;
        log_data["created"]      = created.value();
        log_data["legacy"]       = legacy;
        this->_owner.server.log.write("marketplace_restore_success", log_data);
    }

    if (purchase_pending.empty())
    {
        this->_restoring = false;
        co_return;
    }

    auto purchases = std::unordered_map<std::string, marketplace::purchase_info>{};
    auto fetched   = false;
    try
    {
        purchases = co_await this->get_purchases(purchase_ids);
        fetched   = true;
    }
    catch (std::exception& e)
    {
        fb::logger::warn("Failed to get purchase records during restore: {}", e.what());
    }

    co_await this->_owner.server.threads.switching(weak);
    this->_restoring = false;

    // A missing record only means not purchased when the lookup itself succeeded.
    if (fetched == false)
        co_return;

    for (const auto& [purchase_id, pending_info] : purchase_pending)
    {
        auto purchase_it = purchases.find(pending_info.purchase_id);
        if (purchase_it != purchases.end())
        {
            // The marketplace server already delivered the item and any partial-purchase refund
            // (marketplace:buy:{id}), so only the pending entry is cleared here.
            this->_pending_listings.erase(purchase_id);
        }
        else
        {
            // Purchase record does not exist - restore money
            std::ignore =
                co_await this->_owner.server.system_storage.create(this->_owner.world(),
                                                                   this->_owner.id,
                                                                   std::format("marketplace:purchase:{}", purchase_id),
                                                                   _TEXT(MESSAGE_MARKETPLACE_PURCHASE_RECOVERY_TITLE),
                                                                   _TEXT(MESSAGE_MARKETPLACE_PURCHASE_RECOVERY_MESSAGE),
                                                                   pending_info.dsls);
            co_await this->_owner.server.threads.switching(weak);

            auto log_data            = Json::Value();
            log_data["character_id"] = static_cast<Json::Int64>(this->_owner.id);
            log_data["purchase_id"]  = purchase_id;
            log_data["listing_id"]   = pending_info.listing_id;
            log_data["type"]         = static_cast<Json::Int64>(pending_info.type);
            this->_owner.server.log.write("marketplace_restore_success", log_data);

            this->_pending_listings.erase(purchase_id);
        }
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