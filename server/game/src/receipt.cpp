#include <fb/game/receipt.h>

#include <fb/game/character.h>
#include <fb/game/server.h>
#include <fb/logger.h>
#include <fb/protocol/flatbuffer/protocol.h>

#include <exception>
#include <tuple>
#include <utility>

using namespace fb::game;

receipts::receipts(character& owner) :
    _owner(owner)
{ }

uint64_t receipts::issue(RECEIPT_TYPE type, uint64_t target, Json::Value detail)
{
    this->_owner.assert_thread();

    auto id            = this->_owner.server.ids.next();
    this->_entries[id] = entry{type, target, std::move(detail), true};
    return id;
}

receipts::entry* receipts::find(uint64_t id)
{
    this->_owner.assert_thread();

    auto found = this->_entries.find(id);
    if (found == this->_entries.end())
        return nullptr;

    return &found->second;
}

std::optional<uint64_t> receipts::find(RECEIPT_TYPE type, uint64_t target) const
{
    this->_owner.assert_thread();

    for (const auto& [id, receipt] : this->_entries)
    {
        if (receipt.type == type && receipt.target == target)
            return id;
    }
    return std::nullopt;
}

void receipts::erase(uint64_t id)
{
    this->_owner.assert_thread();
    this->_entries.erase(id);
}

const receipts::entry_map& receipts::entries() const
{
    this->_owner.assert_thread();
    return this->_entries;
}

void receipts::load(const std::vector<dto>& rows)
{
    this->_owner.assert_thread();

    for (const auto& row : rows)
    {
        auto reader = Json::Reader{};
        auto detail = Json::Value{};
        if (reader.parse(row.detail, detail) == false || detail.isObject() == false)
        {
            fb::logger::warn("receipts.load bad detail user={} receipt={}", this->_owner.id, row.id);
            detail = Json::Value{Json::objectValue};
        }

        this->_entries[row.id] = entry{static_cast<RECEIPT_TYPE>(row.type), row.target, std::move(detail), false};
    }
}

std::vector<receipts::dto> receipts::to_protocol() const
{
    this->_owner.assert_thread();

    auto builder           = Json::StreamWriterBuilder{};
    builder["emitUTF8"]    = true;
    builder["indentation"] = "";

    auto dtos = std::vector<dto>{};
    dtos.reserve(this->_entries.size());
    for (const auto& [id, receipt] : this->_entries)
    {
        dtos.push_back(dto{this->_owner.id,
                           id,
                           static_cast<uint8_t>(receipt.type),
                           receipt.target,
                           Json::writeString(builder, receipt.detail)});
    }
    return dtos;
}

async::task<void> receipts::settle()
{
    this->_owner.assert_thread();

    if (this->_settling)
        co_return;

    auto targets = std::vector<uint64_t>{};
    for (const auto& [id, receipt] : this->_entries)
    {
        if (receipt.in_flight == false)
            targets.push_back(id);
    }
    if (targets.empty())
        co_return;

    auto weak       = this->_owner.weak_from_this_as<character>();
    auto owner_id   = this->_owner.id;
    this->_settling = true;
    for (auto id : targets)
    {
        auto found = this->_entries.find(id);
        if (found == this->_entries.end() || found->second.in_flight)
            continue;

        auto type   = found->second.type;
        auto target = found->second.target;
        try
        {
            switch (type)
            {
            case RECEIPT_TYPE::CLAN_MONEY:
                co_await this->_owner.settle_clan_money(id);
                break;

            case RECEIPT_TYPE::STORAGE_CLAIM:
                std::ignore = co_await this->_owner.storage_box.receive_reward(static_cast<uint32_t>(target));
                break;

            case RECEIPT_TYPE::MARKET_LIST:
                co_await this->_owner.marketplace.settle_list(id);
                break;

            case RECEIPT_TYPE::MARKET_PURCHASE:
                co_await this->_owner.marketplace.settle_purchase(id);
                break;

            default:
                fb::logger::warn("receipts.settle unknown type user={} receipt={} type={}",
                                 owner_id,
                                 id,
                                 static_cast<int>(type));
                break;
            }
        }
        catch (const std::exception& e)
        {
            fb::logger::warn("receipts.settle failed user={} receipt={}: {}", owner_id, id, e.what());
        }

        if (weak.expired())
            co_return;
    }
    this->_settling = false;
}
