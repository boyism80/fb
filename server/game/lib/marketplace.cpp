#include <fb/game/marketplace.h>
#include <fb/game/character.h>
#include <fb/game/server.h>
#include <fb/game/item/weapon.h>
#include <fb/protocol/flatbuffer/protocol.h>
#include <fb/model/model.h>
#include <fb/encoding.h>
#include <stdexcept>
#include <sstream>

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

    auto req  = mp_reqs::AllocateListingId{this->_owner.id};
    auto resp = co_await this->_owner.server.http.post("marketplace", "/marketplace/allocate-listing-id", req);

    if (resp.error != 0)
        throw std::runtime_error(std::format("Failed to allocate listing ID: error={}", resp.error));

    co_return resp.listing_id;
}

async::task<marketplace::listing>
marketplace::list(const std::string& id, uint8_t item_index, uint16_t count, uint32_t price, uint16_t expire_hours)
{
    this->_owner.assert_thread();

    // Get item from inventory
    auto item = this->_owner.items.at(item_index);
    if (item == nullptr)
        throw std::runtime_error("Item not found at index");

    auto& model = item->based<fb::model::item>();
    if (item->count() < count)
        throw std::runtime_error("Insufficient item count");

    // Build request item data
    mp::Item req_item{};
    req_item.owner  = this->_owner.id;
    req_item.model  = model.id;
    req_item.count  = count;
    auto durability = item->durability();
    if (durability.has_value())
        req_item.durability = durability.value();

    // Check if item is a weapon with custom name
    if (model.attr(ITEM_ATTRIBUTE::WEAPON))
    {
        auto weapon = static_cast<fb::game::weapon*>(item.get());
        if (weapon != nullptr && weapon->custom_name().has_value())
            req_item.custom_name = weapon->custom_name().value();
    }

    // Send request
    auto req  = mp_reqs::List{this->_owner.id, id, req_item, price, expire_hours};
    auto resp = co_await this->_owner.server.http.post("marketplace", "/marketplace/list", req);

    if (resp.error != 0)
        throw std::runtime_error(std::format("Failed to list item: error={}", resp.error));

    // Remove item from inventory and deduct listing fee
    std::ignore = this->_owner.items.remove(item_index, count, ITEM_DELETE_TYPE::REMOVED);
    this->_owner.money_reduce(resp.listing_fee);

    // Build and return listing
    marketplace::listing result{};
    result.id              = resp.listing_id;
    result.seller_id       = this->_owner.id;
    result.item_data.owner = this->_owner.id;
    result.item_data.model = model.id;
    result.item_data.count = count;
    if (durability.has_value())
        result.item_data.durability = durability.value();

    if (model.attr(ITEM_ATTRIBUTE::WEAPON))
    {
        auto weapon = static_cast<fb::game::weapon*>(item.get());
        if (weapon != nullptr && weapon->custom_name().has_value())
            result.item_data.custom_name = weapon->custom_name().value();
    }

    result.price           = price;
    result.listing_fee     = resp.listing_fee;
    result.transaction_fee = 0; // Will be calculated by server

    co_return result;
}

async::task<bool> marketplace::cancel(const std::string& id)
{
    this->_owner.assert_thread();

    auto req  = mp_reqs::Cancel{this->_owner.id, id};
    auto resp = co_await this->_owner.server.http.post("marketplace", "/marketplace/cancel", req);

    if (resp.error != 0)
        throw std::runtime_error(std::format("Failed to cancel listing: error={}", resp.error));

    co_return true;
}

async::task<marketplace::listing> marketplace::purchase(const std::string& id)
{
    this->_owner.assert_thread();

    auto req  = mp_reqs::Purchase{this->_owner.id, id};
    auto resp = co_await this->_owner.server.http.post("marketplace", "/marketplace/purchase", req);

    if (resp.error != 0)
        throw std::runtime_error(std::format("Failed to purchase item: error={}", resp.error));

    // Note: Item and money will be handled via storage_box
    // The marketplace server will create a pending transaction in storage_box

    // Build and return listing (from response item data)
    marketplace::listing result{};
    result.id              = id;
    result.seller_id       = 0; // Not provided in response
    result.item_data.owner = resp.item.owner;
    result.item_data.model = resp.item.model;
    result.item_data.count = resp.item.count;
    if (resp.item.durability.has_value())
        result.item_data.durability = resp.item.durability.value();

    if (resp.item.custom_name.has_value())
        result.item_data.custom_name = resp.item.custom_name.value();
    result.price           = 0; // Not provided in response
    result.listing_fee     = 0;
    result.transaction_fee = 0;

    co_return result;
}

async::task<marketplace::search_result> marketplace::search(const search_option& option)
{
    this->_owner.assert_thread();

    mp_reqs::Search req{};
    req.item_name = option.item_name;
    req.min_price = option.min_price;
    req.max_price = option.max_price;
    req.seller_id = option.seller_id;
    req.sort_by   = option.sort_by;
    req.page      = option.page;

    auto resp = co_await this->_owner.server.http.post("marketplace", "/marketplace/search", req);

    if (resp.error != 0)
        throw std::runtime_error(std::format("Failed to search items: error={}", resp.error));

    search_result result{};
    result.page        = resp.result.page;
    result.total_count = resp.result.total_count;
    result.listings.reserve(resp.result.listings.size());

    for (const auto& listing : resp.result.listings)
    {
        marketplace::listing l{};
        l.id              = listing.id;
        l.seller_id       = listing.seller_id;
        l.item_data.owner = listing.item.owner;
        l.item_data.model = listing.item.model;
        l.item_data.count = listing.item.count;
        if (listing.item.durability.has_value())
            l.item_data.durability = listing.item.durability.value();
        if (listing.item.custom_name.has_value())
            l.item_data.custom_name = listing.item.custom_name.value();
        l.price           = listing.price;
        l.listing_fee     = listing.listing_fee;
        l.transaction_fee = listing.transaction_fee;
        if (!listing.expire_date.empty())
            l.expire_date = fb::model::datetime(listing.expire_date);
        if (!listing.created_date.empty())
            l.created_date = fb::model::datetime(listing.created_date);
        result.listings.push_back(l);
    }

    co_return result;
}

async::task<std::optional<marketplace::listing>> marketplace::check_status(const std::string& id)
{
    this->_owner.assert_thread();

    auto req  = mp_reqs::CheckListingStatus{id};
    auto resp = co_await this->_owner.server.http.post("marketplace", "/marketplace/check-listing-status", req);

    if (resp.error != 0)
        co_return std::nullopt;

    // Note: The response only contains listing_id and listing_fee
    // Full listing details are not provided in this endpoint
    marketplace::listing result{};
    result.id          = resp.listing_id;
    result.listing_fee = resp.listing_fee;

    co_return result;
}