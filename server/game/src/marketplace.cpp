#include <fb/game/marketplace.h>

#include <fb/encoding.h>
#include <fb/game/character.h>
#include <fb/game/item/weapon.h>
#include <fb/game/server.h>
#include <fb/game/storage.h>
#include <fb/model/model.h>
#include <fb/protocol/flatbuffer/protocol.h>
#include <macro.h>

#include <json/json.h>

#include <algorithm>
#include <cstdint>
#include <exception>
#include <format>
#include <limits>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>

using namespace fb::game;

namespace mp      = fb::protocol::marketplace;
namespace mp_reqs = mp::request;
namespace mp_resp = mp::response;

marketplace::marketplace(character& owner) :
    _owner(owner)
{ }

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
    auto uid         = item->uid() != 0 ? std::optional<uint64_t>(item->uid()) : std::nullopt;
    auto custom_name = std::optional<std::string>{};
    if (model.attr(ITEM_ATTRIBUTE::WEAPON))
    {
        auto weapon = static_cast<fb::game::weapon*>(item.get());
        if (weapon != nullptr && weapon->custom_name().has_value())
            custom_name = weapon->custom_name().value();
    }

    if (count > 0 && price > std::numeric_limits<uint64_t>::max() / count)
        throw std::runtime_error(std::format(_TEXT(MESSAGE_MARKETPLACE_FAILED_TO_LIST_ITEM_WITH_ERROR), "price"));

    auto total_sale_amount = static_cast<uint64_t>(count) * price;
    auto listing_fee = static_cast<uint64_t>(total_sale_amount * fb::model::const_value::marketplace::listing_fee);
    if (this->_owner.money() < listing_fee)
        throw std::runtime_error(_TEXT(MESSAGE_MARKETPLACE_INSUFFICIENT_MONEY_FOR_LISTING_FEE));

    auto id    = this->_owner.unsettled.add(UNSETTLED_TYPE::MARKET_LIST, 0);
    auto parts = std::vector<fb::game::items::escrow_part>{
        {slot, count}
    };
    if (this->_owner.items.hold(id, listing_fee, parts) == false)
    {
        this->_owner.unsettled.erase(id);
        throw std::runtime_error(std::format(_TEXT(MESSAGE_MARKETPLACE_FAILED_TO_LIST_ITEM_WITH_ERROR), "lock"));
    }

    auto log_data_before            = Json::Value();
    log_data_before["character_id"] = static_cast<Json::Int64>(this->_owner.id);
    log_data_before["listing_id"]   = static_cast<Json::UInt64>(id);
    log_data_before["item_model"]   = model.id;
    log_data_before["item_count"]   = count;
    log_data_before["price"]        = static_cast<Json::Int64>(price);
    log_data_before["listing_fee"]  = static_cast<Json::Int64>(listing_fee);
    if (uid.has_value())
        log_data_before["item_uid"] = static_cast<Json::UInt64>(uid.value());
    this->_owner.server.log.write("marketplace_list", log_data_before);

    auto error_what = std::string{};
    auto error_code = fb::model::enum_value::ERROR_CODE::NONE;
    auto unknown    = false;
    try
    {
        auto world    = this->_owner.world();
        auto deadline = std::chrono::duration_cast<std::chrono::milliseconds>(
                            (std::chrono::system_clock::now() + REQUEST_TTL).time_since_epoch())
                            .count();
        auto&& resp = co_await this->_owner.server.http.post(
            weak,
            "marketplace",
            "/marketplace/list",
            mp_reqs::List{
                world,
                this->_owner.id,
                id,
                mp::Item{this->_owner.id, model.id, count, durability, custom_name, uid},
                price,
                deadline
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
        case fb::model::enum_value::ERROR_CODE::MARKETPLACE_REQUEST_EXPIRED:
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

    if (unknown)
    {
        this->_owner.unsettled.find(id)->in_flight = false;

        auto log_data            = Json::Value();
        log_data["character_id"] = static_cast<Json::Int64>(this->_owner.id);
        log_data["listing_id"]   = static_cast<Json::UInt64>(id);
        log_data["error"]        = error_what;
        this->_owner.server.log.write("marketplace_list_pending", log_data);
        throw std::runtime_error(_TEXT(MESSAGE_MARKETPLACE_LISTING_PENDING));
    }
    else if (error_code != fb::model::enum_value::ERROR_CODE::NONE)
    {
        this->_owner.unsettled.erase(id);
        this->_owner.items.release(id);

        auto log_data            = Json::Value();
        log_data["character_id"] = static_cast<Json::Int64>(this->_owner.id);
        log_data["listing_id"]   = static_cast<Json::UInt64>(id);
        log_data["error"]        = enum_tostring(error_code);
        this->_owner.server.log.write("marketplace_list_failed", log_data);
        throw std::runtime_error(
            std::format(_TEXT(MESSAGE_MARKETPLACE_FAILED_TO_LIST_ITEM), enum_tostring(error_code)));
    }
    else
    {
        this->_owner.unsettled.erase(id);
        this->_owner.items.take(id);

        auto log_data_success            = Json::Value();
        log_data_success["character_id"] = static_cast<Json::Int64>(this->_owner.id);
        log_data_success["listing_id"]   = static_cast<Json::UInt64>(id);
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

async::task<bool> marketplace::cancel(uint64_t id)
{
    this->_owner.assert_thread();

    // Log before API call
    auto log_data_before            = Json::Value();
    log_data_before["character_id"] = static_cast<Json::Int64>(this->_owner.id);
    log_data_before["listing_id"]   = static_cast<Json::UInt64>(id);
    this->_owner.server.log.write("marketplace_cancel", log_data_before);

    auto   weak  = this->_owner.weak_from_this_as<fb::game::character>();
    auto   world = this->_owner.world();
    auto   req   = mp_reqs::Cancel{world, this->_owner.id, id};
    auto&& resp  = co_await this->_owner.server.http.post(weak, "marketplace", "/marketplace/cancel", req);

    if (resp.error != 0)
    {
        auto log_data            = Json::Value();
        log_data["character_id"] = static_cast<Json::Int64>(this->_owner.id);
        log_data["listing_id"]   = static_cast<Json::UInt64>(id);
        log_data["error"]        = static_cast<Json::Int64>(resp.error);
        this->_owner.server.log.write("marketplace_cancel_failed", log_data);
        throw std::runtime_error(std::format(_TEXT(MESSAGE_MARKETPLACE_FAILED_TO_CANCEL_LISTING), resp.error));
    }

    // Log successful cancellation
    auto log_data_success            = Json::Value();
    log_data_success["character_id"] = static_cast<Json::Int64>(this->_owner.id);
    log_data_success["listing_id"]   = static_cast<Json::UInt64>(id);
    this->_owner.server.log.write("marketplace_cancel_success", log_data_success);

    co_return true;
}

async::task<uint32_t> marketplace::clear()
{
    this->_owner.assert_thread();

    auto   weak = this->_owner.weak_from_this_as<fb::game::character>();
    auto   req  = mp_reqs::ClearListings{this->_owner.id};
    auto&& resp = co_await this->_owner.server.http.post(weak, "marketplace", "/marketplace/clear-listings", req);
    if (resp.error != 0)
        throw std::runtime_error(std::format("error code {}", resp.error));

    co_return resp.count;
}

async::task<marketplace::listing> marketplace::purchase(uint64_t listing_id, uint16_t purchase_count)
{
    this->_owner.assert_thread();

    // Get listing info first (needed for price validation and timeout recovery)
    auto weak        = this->_owner.weak_from_this_as<fb::game::character>();
    auto listing_ids = std::vector<uint64_t>{listing_id};
    auto listings    = co_await this->get_listings(listing_ids);

    if (listings.empty())
        throw std::runtime_error(_TEXT(MESSAGE_MARKETPLACE_LISTING_NOT_FOUND));

    auto& listing = listings[0];
    if (listing.state != static_cast<uint8_t>(mp::ListingState::ACTIVE))
        throw std::runtime_error(_TEXT(MESSAGE_MARKETPLACE_LISTING_NOT_AVAILABLE_FOR_PURCHASE));

    if (listing.price > 0 && purchase_count > std::numeric_limits<uint64_t>::max() / listing.price)
        throw std::runtime_error(_TEXT(MESSAGE_MARKETPLACE_INSUFFICIENT_MONEY));

    // Calculate expected price (per unit price * purchase_count)
    auto expected_price = listing.price * purchase_count;

    // Validate money BEFORE API call
    if (this->_owner.money() < expected_price)
        throw std::runtime_error(_TEXT(MESSAGE_MARKETPLACE_INSUFFICIENT_MONEY));

    auto detail     = Json::Value{Json::objectValue};
    detail["count"] = static_cast<Json::UInt>(purchase_count);
    detail["price"] = static_cast<Json::UInt64>(expected_price);

    this->_owner.money_reduce(expected_price);
    auto purchase_id = this->_owner.unsettled.add(UNSETTLED_TYPE::MARKET_PURCHASE, listing_id, detail);

    // Log before API call (after deduction)
    auto log_data_before              = Json::Value();
    log_data_before["character_id"]   = static_cast<Json::Int64>(this->_owner.id);
    log_data_before["listing_id"]     = static_cast<Json::UInt64>(listing_id);
    log_data_before["purchase_id"]    = static_cast<Json::UInt64>(purchase_id);
    log_data_before["seller_id"]      = static_cast<Json::Int64>(listing.seller_id);
    log_data_before["purchase_count"] = static_cast<Json::Int64>(purchase_count);
    log_data_before["expected_price"] = static_cast<Json::Int64>(expected_price);
    this->_owner.server.log.write("marketplace_purchase", log_data_before);

    auto error_what = std::string{};
    auto error_code = fb::model::enum_value::ERROR_CODE::NONE;
    auto unknown    = false;
    auto purchased  = std::optional<marketplace::listing>{};
    auto refund     = uint64_t{0};
    try
    {
        auto world    = this->_owner.world();
        auto deadline = std::chrono::duration_cast<std::chrono::milliseconds>(
                            (std::chrono::system_clock::now() + REQUEST_TTL).time_since_epoch())
                            .count();
        auto   req  = mp_reqs::Purchase{world, this->_owner.id, listing_id, purchase_count, purchase_id, deadline};
        auto&& resp = co_await this->_owner.server.http.post(weak, "marketplace", "/marketplace/purchase", req);
        error_code  = static_cast<fb::model::enum_value::ERROR_CODE>(resp.error);
        switch (error_code)
        {
        case fb::model::enum_value::ERROR_CODE::NONE:
            purchased = marketplace::listing{
                .id           = listing_id,
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
            refund = resp.refund_amount;
            break;

        case fb::model::enum_value::ERROR_CODE::MARKETPLACE_LISTING_NOT_FOUND:
        case fb::model::enum_value::ERROR_CODE::MARKETPLACE_LISTING_EXPIRED:
        case fb::model::enum_value::ERROR_CODE::MARKETPLACE_LISTING_ALREADY_SOLD:
        case fb::model::enum_value::ERROR_CODE::MARKETPLACE_LISTING_ALREADY_CANCELLED:
        case fb::model::enum_value::ERROR_CODE::MARKETPLACE_INSUFFICIENT_STOCK:
        case fb::model::enum_value::ERROR_CODE::MARKETPLACE_REQUEST_EXPIRED:
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

    if (unknown)
    {
        this->_owner.unsettled.find(purchase_id)->in_flight = false;

        auto log_data            = Json::Value();
        log_data["character_id"] = static_cast<Json::Int64>(this->_owner.id);
        log_data["listing_id"]   = static_cast<Json::UInt64>(listing_id);
        log_data["purchase_id"]  = static_cast<Json::UInt64>(purchase_id);
        log_data["error"]        = error_what;
        this->_owner.server.log.write("marketplace_purchase_pending", log_data);
        throw std::runtime_error(_TEXT(MESSAGE_MARKETPLACE_PURCHASE_PENDING));
    }
    else if (error_code != fb::model::enum_value::ERROR_CODE::NONE)
    {
        this->_owner.unsettled.erase(purchase_id);
        std::ignore = this->_owner.money_add(expected_price);

        auto log_data            = Json::Value();
        log_data["character_id"] = static_cast<Json::Int64>(this->_owner.id);
        log_data["listing_id"]   = static_cast<Json::UInt64>(listing_id);
        log_data["purchase_id"]  = static_cast<Json::UInt64>(purchase_id);
        log_data["error"]        = enum_tostring(error_code);
        this->_owner.server.log.write("marketplace_purchase_failed", log_data);
        throw std::runtime_error(
            std::format(_TEXT(MESSAGE_MARKETPLACE_FAILED_TO_PURCHASE_ITEM), enum_tostring(error_code)));
    }
    else
    {
        this->_owner.unsettled.erase(purchase_id);

        auto log_data_success             = Json::Value();
        log_data_success["character_id"]  = static_cast<Json::Int64>(this->_owner.id);
        log_data_success["listing_id"]    = static_cast<Json::UInt64>(listing_id);
        log_data_success["purchase_id"]   = static_cast<Json::UInt64>(purchase_id);
        log_data_success["actual_count"]  = static_cast<Json::Int64>(purchased->item_data.count);
        log_data_success["refund_amount"] = static_cast<Json::Int64>(refund);
        this->_owner.server.log.write("marketplace_purchase_success", log_data_success);

        co_return std::move(purchased.value());
    }
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

async::task<std::vector<marketplace::listing>> marketplace::get_listings(const std::vector<uint64_t>& listing_ids,
                                                                         uint32_t                     buyer_id)
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

async::task<void> marketplace::settle_list(uint64_t listing_id)
{
    this->_owner.assert_thread();

    auto request = this->_owner.unsettled.find(listing_id);
    if (request == nullptr)
        co_return;

    // An entry whose item row failed to restore still holds the fee, so the entry decides and not its parts.
    auto weak     = this->_owner.weak_from_this_as<fb::game::character>();
    auto world    = this->_owner.world();
    auto item     = mp::Item{this->_owner.id, 0, 0, std::nullopt, std::nullopt, std::nullopt};
    auto escrowed = this->_owner.items.escrow().contains(listing_id);
    for (const auto& part : this->_owner.items.escrow().parts(listing_id))
    {
        auto& source      = *part.value;
        auto  custom_name = std::optional<std::string>{};
        if (source.model().attr(ITEM_ATTRIBUTE::WEAPON))
            custom_name = static_cast<fb::game::weapon&>(source).custom_name();

        item = mp::Item{this->_owner.id,
                        source.model().id,
                        source.count(),
                        source.durability(),
                        custom_name,
                        source.uid() != 0 ? std::optional<uint64_t>(source.uid()) : std::nullopt};
        break;
    }

    // Listings created before item escrow removed the item and kept it in the unsettled request detail.
    auto returns = std::vector<fb::model::dsl>{};
    if (escrowed == false)
    {
        for (const auto& dsl_json : request->detail["attachments"])
        {
            auto dsl = fb::model::dsl(dsl_json);
            returns.push_back(dsl);
            if (dsl.header != fb::model::enum_value::DSL::item)
                continue;

            auto params      = fb::model::dsl::item(dsl.params);
            item.model       = params.id;
            item.count       = static_cast<uint16_t>(params.count);
            item.durability  = params.durability;
            item.custom_name = params.custom_name;
            item.uid         = params.uid;
        }

        if (returns.empty())
        {
            fb::logger::warn("marketplace.settle_list nothing held user={} listing={}", this->_owner.id, listing_id);
            this->_owner.unsettled.erase(listing_id);
            co_return;
        }
    }

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

    if (weak.expired() || created.has_value() == false)
        co_return;

    if (escrowed)
    {
        if (created.value())
            this->_owner.items.take(listing_id);
        else
            this->_owner.items.release(listing_id);
    }
    else if (created.value() == false)
    {
        auto returned =
            co_await this->_owner.server.system_storage.create(world,
                                                               this->_owner.id,
                                                               std::format("marketplace:list:{}", listing_id),
                                                               _TEXT(MESSAGE_MARKETPLACE_LISTING_RECOVERY_TITLE),
                                                               _TEXT(MESSAGE_MARKETPLACE_LISTING_RECOVERY_MESSAGE),
                                                               returns);
        co_await this->_owner.server.threads.switching(weak);
        if (weak.expired() || returned == false)
            co_return;
    }
    this->_owner.unsettled.erase(listing_id);

    auto log_data            = Json::Value();
    log_data["character_id"] = static_cast<Json::Int64>(this->_owner.id);
    log_data["listing_id"]   = static_cast<Json::UInt64>(listing_id);
    log_data["created"]      = created.value();
    log_data["legacy"]       = escrowed == false;
    this->_owner.server.log.write("marketplace_restore_success", log_data);
}

async::task<void> marketplace::settle_purchase(uint64_t purchase_id)
{
    this->_owner.assert_thread();

    auto request = this->_owner.unsettled.find(purchase_id);
    if (request == nullptr)
        co_return;

    auto weak       = this->_owner.weak_from_this_as<fb::game::character>();
    auto world      = this->_owner.world();
    auto listing_id = request->target;
    auto price      = request->detail["price"].asUInt64();

    auto purchased = std::optional<bool>{};
    try
    {
        auto&& resp = co_await this->_owner.server.http.post(
            weak,
            "marketplace",
            "/marketplace/abort-purchase",
            mp_reqs::AbortPurchase{world, this->_owner.id, listing_id, purchase_id});
        if (resp.error == 0)
            purchased = resp.purchased;
        else
            fb::logger::warn("abort-purchase {} failed: {}", purchase_id, resp.error);
    }
    catch (std::exception& e)
    {
        fb::logger::warn("abort-purchase {} failed: {}", purchase_id, e.what());
    }

    if (weak.expired() || purchased.has_value() == false)
        co_return;

    if (purchased.value() == false && price > 0)
    {
        auto refunded =
            co_await this->_owner.server.system_storage.create(world,
                                                               this->_owner.id,
                                                               std::format("marketplace:purchase:{}", purchase_id),
                                                               _TEXT(MESSAGE_MARKETPLACE_PURCHASE_RECOVERY_TITLE),
                                                               _TEXT(MESSAGE_MARKETPLACE_PURCHASE_RECOVERY_MESSAGE),
                                                               {fb::model::dsl::money(price).to_dsl()});
        co_await this->_owner.server.threads.switching(weak);
        if (weak.expired() || refunded == false)
            co_return;
    }
    this->_owner.unsettled.erase(purchase_id);

    auto log_data            = Json::Value();
    log_data["character_id"] = static_cast<Json::Int64>(this->_owner.id);
    log_data["purchase_id"]  = static_cast<Json::UInt64>(purchase_id);
    log_data["listing_id"]   = static_cast<Json::UInt64>(listing_id);
    log_data["purchased"]    = purchased.value();
    this->_owner.server.log.write("marketplace_restore_success", log_data);
}

void marketplace::listing::to_lua(fb::lua::context* lua) const
{
    lua->new_table();

    lua->pushstring("id");
    lua->pushinteger(this->id);
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