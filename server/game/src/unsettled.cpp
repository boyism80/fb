#include <fb/game/unsettled.h>

#include <fb/game/character.h>
#include <fb/game/server.h>
#include <fb/logger.h>
#include <fb/protocol/flatbuffer/protocol.h>

#include <exception>
#include <tuple>
#include <utility>

using namespace fb::game;

unsettled_requests::unsettled_requests(character& owner) :
    _owner(owner)
{ }

uint64_t unsettled_requests::add(UNSETTLED_TYPE type, uint64_t target, Json::Value detail)
{
    this->_owner.assert_thread();

    auto id            = this->_owner.server.ids.next();
    this->_entries[id] = entry{type, target, std::move(detail), true};
    return id;
}

unsettled_requests::entry* unsettled_requests::find(uint64_t id)
{
    this->_owner.assert_thread();

    auto found = this->_entries.find(id);
    if (found == this->_entries.end())
        return nullptr;

    return &found->second;
}

std::optional<uint64_t> unsettled_requests::find(UNSETTLED_TYPE type, uint64_t target) const
{
    this->_owner.assert_thread();

    for (const auto& [id, request] : this->_entries)
    {
        if (request.type == type && request.target == target)
            return id;
    }
    return std::nullopt;
}

void unsettled_requests::erase(uint64_t id)
{
    this->_owner.assert_thread();
    this->_entries.erase(id);
}

const unsettled_requests::entry_map& unsettled_requests::entries() const
{
    this->_owner.assert_thread();
    return this->_entries;
}

void unsettled_requests::load(const std::vector<dto>& rows)
{
    this->_owner.assert_thread();

    for (const auto& row : rows)
    {
        auto reader = Json::Reader{};
        auto detail = Json::Value{};
        if (reader.parse(row.detail, detail) == false || detail.isObject() == false)
        {
            fb::logger::warn("unsettled.load bad detail user={} request={}", this->_owner.id, row.id);
            detail = Json::Value{Json::objectValue};
        }

        this->_entries[row.id] = entry{static_cast<UNSETTLED_TYPE>(row.type), row.target, std::move(detail), false};
    }
}

std::vector<unsettled_requests::dto> unsettled_requests::to_protocol() const
{
    this->_owner.assert_thread();

    auto builder           = Json::StreamWriterBuilder{};
    builder["emitUTF8"]    = true;
    builder["indentation"] = "";

    auto dtos = std::vector<dto>{};
    dtos.reserve(this->_entries.size());
    for (const auto& [id, request] : this->_entries)
    {
        dtos.push_back(dto{this->_owner.id,
                           id,
                           static_cast<uint8_t>(request.type),
                           request.target,
                           Json::writeString(builder, request.detail)});
    }
    return dtos;
}

async::task<void> unsettled_requests::settle()
{
    this->_owner.assert_thread();

    if (this->_settling)
        co_return;

    auto targets = std::vector<uint64_t>{};
    for (const auto& [id, request] : this->_entries)
    {
        if (request.in_flight == false)
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
            case UNSETTLED_TYPE::CLAN_MONEY:
                co_await this->_owner.clan.settle_money(id);
                break;

            case UNSETTLED_TYPE::STORAGE_CLAIM:
                std::ignore = co_await this->_owner.storage_box.receive_reward(static_cast<uint32_t>(target));
                break;

            case UNSETTLED_TYPE::MARKET_LIST:
                co_await this->_owner.marketplace.settle_list(id);
                break;

            case UNSETTLED_TYPE::MARKET_PURCHASE:
                co_await this->_owner.marketplace.settle_purchase(id);
                break;

            case UNSETTLED_TYPE::CLAN_FEE:
                co_await this->_owner.clan.settle_fee(id);
                break;

            default:
                fb::logger::warn("unsettled.settle unknown type user={} request={} type={}",
                                 owner_id,
                                 id,
                                 static_cast<int>(type));
                break;
            }
        }
        catch (const std::exception& e)
        {
            fb::logger::warn("unsettled.settle failed user={} request={}: {}", owner_id, id, e.what());
        }

        if (weak.expired())
            co_return;
    }
    this->_settling = false;
}
