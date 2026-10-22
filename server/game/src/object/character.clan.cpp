#include <fb/game/character.h>

#include <fb/game/server.h>
#include <fb/logger.h>

#include <json/json.h>

#include <cstdint>
#include <optional>

using namespace fb::game;

character::clan::clan(character& owner) :
    _owner(owner)
{ }

const std::optional<uint32_t>& character::clan::id() const
{
    this->_owner.assert_thread();
    return this->_id;
}

void character::clan::id(std::optional<uint32_t> value)
{
    this->_owner.assert_thread();
    this->_id = value;
}

void character::clan::reset()
{
    this->_owner.assert_thread();

    this->_id.reset();
    this->_owner.update_external();
}

async::task<std::optional<bool>> character::clan::transfer_money(uint32_t clan_id,
                                                                 int64_t  clan_delta,
                                                                 int64_t  money_delta)
{
    this->_owner.assert_thread();

    auto hold = money_delta < 0 ? static_cast<uint64_t>(-money_delta) : uint64_t{0};
    if (this->_owner.money() < hold)
        co_return false;

    auto weak             = this->_owner.weak_from_this_as<character>();
    auto world            = this->_owner.world();
    auto detail           = Json::Value{Json::objectValue};
    detail["clan_delta"]  = static_cast<Json::Int64>(clan_delta);
    detail["money_delta"] = static_cast<Json::Int64>(money_delta);

    this->_owner.money_reduce(hold);
    auto request_id = this->_owner.unsettled.add(UNSETTLED_TYPE::CLAN_MONEY, clan_id, detail);

    auto applied = co_await this->_owner.server.clans.add_money(world, clan_id, clan_delta, request_id);
    co_await this->_owner.server.threads.switching(weak);
    if (weak.expired())
        co_return applied;

    if (applied.has_value())
    {
        this->complete_money(request_id, applied.value());
    }
    else
    {
        auto request = this->_owner.unsettled.find(request_id);
        if (request != nullptr)
            request->in_flight = false;
    }
    co_return applied;
}

async::task<void> character::clan::settle_money(uint64_t request_id)
{
    this->_owner.assert_thread();

    auto request = this->_owner.unsettled.find(request_id);
    if (request == nullptr)
        co_return;

    auto weak    = this->_owner.weak_from_this_as<character>();
    auto world   = this->_owner.world();
    auto clan_id = static_cast<uint32_t>(request->target);

    auto applied = co_await this->_owner.server.clans.abort_money(world, clan_id, request_id);
    co_await this->_owner.server.threads.switching(weak);
    if (weak.expired())
        co_return;

    if (applied.has_value())
        this->complete_money(request_id, applied.value());
}

void character::clan::complete_money(uint64_t request_id, bool applied)
{
    this->_owner.assert_thread();

    auto request = this->_owner.unsettled.find(request_id);
    if (request == nullptr)
        return;

    auto money_delta = request->detail["money_delta"].asInt64();
    this->_owner.unsettled.erase(request_id);

    if (applied && money_delta > 0)
        this->_owner.money_add(static_cast<uint64_t>(money_delta));
    else if (applied == false && money_delta < 0)
        this->_owner.money_add(static_cast<uint64_t>(-money_delta));
}

std::optional<uint64_t> character::clan::hold_fee(uint32_t clan_id, uint64_t money, uint64_t clan_money)
{
    this->_owner.assert_thread();

    auto detail          = Json::Value{Json::objectValue};
    detail["clan_delta"] = static_cast<Json::Int64>(clan_money);

    auto request_id = this->_owner.unsettled.add(UNSETTLED_TYPE::CLAN_FEE, clan_id, detail);
    if (this->_owner.items.hold(request_id, money, {}) == false)
    {
        this->_owner.unsettled.erase(request_id);
        return std::nullopt;
    }
    return request_id;
}

bool character::clan::commit_fee(uint64_t request_id)
{
    this->_owner.assert_thread();

    auto request = this->_owner.unsettled.find(request_id);
    if (request == nullptr || this->_owner.items.escrow().contains(request_id) == false)
        return false;

    this->_owner.items.take(request_id);
    request->in_flight = false;
    return true;
}

void character::clan::release_fee(uint64_t request_id)
{
    this->_owner.assert_thread();

    auto request = this->_owner.unsettled.find(request_id);
    if (request == nullptr)
        return;

    if (this->_owner.items.escrow().contains(request_id))
    {
        this->_owner.items.release(request_id);
        this->_owner.unsettled.erase(request_id);
    }
    else
    {
        request->in_flight = false;
    }
}

async::task<void> character::clan::settle_fee(uint64_t request_id)
{
    this->_owner.assert_thread();

    auto request = this->_owner.unsettled.find(request_id);
    if (request == nullptr || request->in_flight)
        co_return;

    if (this->_owner.items.escrow().contains(request_id))
    {
        this->_owner.items.release(request_id);
        this->_owner.unsettled.erase(request_id);
    }
    else
    {
        auto weak          = this->_owner.weak_from_this_as<character>();
        auto world         = this->_owner.world();
        auto clan_id       = static_cast<uint32_t>(request->target);
        auto clan_delta    = request->detail["clan_delta"].asInt64();
        request->in_flight = true;

        auto sent = co_await this->_owner.server.clans.send_money(world, clan_id, clan_delta, request_id);
        co_await this->_owner.server.threads.switching(weak);
        if (weak.expired())
            co_return;

        request = this->_owner.unsettled.find(request_id);
        if (request == nullptr)
            co_return;

        if (sent.has_value())
        {
            if (sent.value() == false)
                fb::logger::warn("{} clan fee {} dropped: clan {} is gone", this->_owner.name(), request_id, clan_id);
            this->_owner.unsettled.erase(request_id);
        }
        else
        {
            request->in_flight = false;
        }
    }
}
