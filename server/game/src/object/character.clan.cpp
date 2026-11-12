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

async::task<void> character::clan::leave(std::string message)
{
    this->_owner.assert_thread();

    this->reset();
    this->_owner.message(message, MESSAGE_TYPE::NOTIFY);
    co_await this->on_changed();
}

async::task<void> character::clan::on_changed()
{
    this->_owner.assert_thread();

    auto lua = this->_owner.server.lua.open("scripts/interaction.lua", "on_clan_changed");
    if (lua)
    {
        lua->pushobject(this->_owner);
        std::ignore = co_await fb::lua::call(std::move(lua), this->_owner.weak_from_this_as<character>(), 1);
    }
}

async::task<std::optional<bool>> character::clan::deposit(uint64_t amount)
{
    this->_owner.assert_thread();

    if (this->_id.has_value() == false)
        co_return false;

    auto delta = static_cast<int64_t>(amount);
    co_return co_await this->transfer_money(this->_id.value(), delta, -delta);
}

async::task<std::optional<bool>> character::clan::withdraw(uint64_t amount)
{
    this->_owner.assert_thread();

    if (this->_id.has_value() == false)
        co_return false;

    auto clan_id = this->_id.value();
    {
        auto guard = this->_owner.server.clans.try_enter_read(clan_id);
        if (guard.has_value() == false || guard->value() == nullptr)
            co_return false;

        auto member = guard->value()->member(this->_owner.name());
        if (member == nullptr || member->role != CLAN_ROLE::MASTER)
            co_return false;
    }

    auto delta = static_cast<int64_t>(amount);
    co_return co_await this->transfer_money(clan_id, -delta, delta);
}

async::task<std::optional<bool>> character::clan::transfer_money(uint32_t clan_id,
                                                                 int64_t  clan_delta,
                                                                 int64_t  money_delta)
{
    this->_owner.assert_thread();

    auto hold             = money_delta < 0 ? static_cast<uint64_t>(-money_delta) : uint64_t{0};
    auto weak             = this->_owner.weak_from_this_as<character>();
    auto world            = this->_owner.world();
    auto detail           = Json::Value{Json::objectValue};
    detail["clan_delta"]  = static_cast<Json::Int64>(clan_delta);
    detail["money_delta"] = static_cast<Json::Int64>(money_delta);

    auto added = this->_owner.unsettled.add(UNSETTLED_TYPE::CLAN_MONEY, clan_id, hold, {}, detail);
    if (added.has_value() == false)
        co_return false;

    auto request_id = added.value();

    auto applied = co_await this->_owner.server.clans.add_money(world, clan_id, clan_delta, request_id);
    co_await this->_owner.server.threads.switching(weak);
    if (weak.expired())
        co_return applied;

    if (applied.has_value())
        this->complete_money(request_id, applied.value());
    else
        this->_owner.unsettled.leave(request_id);
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
    if (applied)
    {
        this->_owner.unsettled.confirm(request_id);
        if (money_delta > 0)
            this->_owner.money_add(static_cast<uint64_t>(money_delta));
    }
    else
    {
        this->_owner.unsettled.refuse(request_id);
    }
}

async::task<bool> character::clan::map(std::shared_ptr<fb::game::map>      map,
                                       std::optional<fb::model::point16_t> position,
                                       map_options                         options,
                                       uint32_t                            clan_id,
                                       uint64_t                            money,
                                       uint64_t                            clan_money)
{
    this->_owner.assert_thread();

    auto detail          = Json::Value{Json::objectValue};
    detail["clan_delta"] = static_cast<Json::Int64>(clan_money);

    auto added = this->_owner.unsettled.add(UNSETTLED_TYPE::CLAN_FEE, clan_id, money, {}, detail);
    if (added.has_value() == false)
        co_return false;

    auto request_id = added.value();

    auto ch        = this->_owner.shared_from_this_as<character>();
    auto weak      = std::weak_ptr<character>(ch);
    options.before = [before = options.before, weak, request_id]() -> async::task<bool> {
        if (before && co_await before() == false)
            co_return false;

        auto locked = weak.lock();
        co_return locked != nullptr && locked->unsettled.charge(request_id);
    };

    auto success = co_await ch->map(map, position, options);
    co_await ch->server.threads.switching(weak);
    if (success == false)
    {
        if (ch->unsettled.held(request_id))
            ch->unsettled.refuse(request_id);
        else
            ch->unsettled.leave(request_id);
    }
    co_return success;
}

async::task<void> character::clan::settle_fee(uint64_t request_id)
{
    this->_owner.assert_thread();

    auto request = this->_owner.unsettled.find(request_id);
    if (request == nullptr || request->in_flight)
        co_return;

    if (this->_owner.unsettled.held(request_id))
    {
        this->_owner.unsettled.refuse(request_id);
    }
    else
    {
        auto weak       = this->_owner.weak_from_this_as<character>();
        auto world      = this->_owner.world();
        auto clan_id    = static_cast<uint32_t>(request->target);
        auto clan_delta = request->detail["clan_delta"].asInt64();
        this->_owner.unsettled.retry(request_id);

        auto sent = co_await this->_owner.server.clans.send_money(world, clan_id, clan_delta, request_id);
        co_await this->_owner.server.threads.switching(weak);
        if (weak.expired())
            co_return;

        if (this->_owner.unsettled.find(request_id) == nullptr)
            co_return;

        if (sent.has_value())
        {
            if (sent.value() == false)
                fb::logger::warn("{} clan fee {} dropped: clan {} is gone", this->_owner.name(), request_id, clan_id);
            this->_owner.unsettled.confirm(request_id);
        }
        else
        {
            this->_owner.unsettled.leave(request_id);
        }
    }
}
