#include <fb/game/castle.h>

#include <fb/game/server.h>

#include <chrono>
#include <cstdint>
#include <optional>

using namespace fb::game;
using namespace std::chrono_literals;

castle::castle(server&                             server,
               fb::model::enum_value::DIVINE_BEAST divine_beast,
               const std::optional<uint32_t>&      owner_clan_id) :
    _server(server),
    _divine_beast(divine_beast),
    _owner_clan_id(owner_clan_id)
{ }

castle::castle(castle&& r) :
    _server(r._server),
    _divine_beast(r._divine_beast),
    _owner_clan_id(r._owner_clan_id),
    _siege_id(r._siege_id),
    _siege_end_at(r._siege_end_at),
    _siege_active_until(r._siege_active_until),
    _revision(r._revision),
    _siege_ended(r._siege_ended),
    _siege_ending(r._siege_ending),
    _siege_winner(r._siege_winner)
{ }

uint32_t castle::id() const
{
    return static_cast<uint32_t>(this->_divine_beast);
}

fb::model::enum_value::DIVINE_BEAST castle::divine_beast() const
{
    return this->_divine_beast;
}

const std::optional<uint32_t>& castle::owner_clan_id() const
{
    return this->_owner_clan_id;
}

void castle::owner_clan_id(const std::optional<uint32_t>& value)
{
    this->_owner_clan_id = value;
}

bool castle::siege_active() const
{
    if (this->siege_pending() == false)
        return false;

    return std::chrono::steady_clock::now() < this->_siege_active_until;
}

bool castle::siege_pending() const
{
    return this->_siege_id.has_value() && this->_siege_ended == false;
}

const std::optional<uint64_t>& castle::siege_id() const
{
    return this->_siege_id;
}

std::optional<uint32_t> castle::siege_remaining_seconds() const
{
    if (this->siege_pending() == false)
        return std::nullopt;

    auto now = std::chrono::steady_clock::now();
    if (now >= this->_siege_end_at)
        return 0;

    auto remaining = std::chrono::duration_cast<std::chrono::seconds>(this->_siege_end_at - now);
    return static_cast<uint32_t>(remaining.count());
}

bool castle::apply(uint64_t                       revision,
                   const std::optional<uint32_t>& owner_clan_id,
                   const std::optional<uint64_t>& siege_id,
                   uint32_t                       remaining,
                   uint32_t                       active_remaining,
                   bool                           ended)
{
    if (revision < this->_revision)
        return false;

    auto now        = std::chrono::steady_clock::now();
    auto same_round = this->_siege_id == siege_id;

    this->_revision           = revision;
    this->_owner_clan_id      = owner_clan_id;
    this->_siege_id           = siege_id;
    this->_siege_end_at       = now + std::chrono::seconds(remaining);
    this->_siege_active_until = now + std::chrono::seconds(active_remaining);
    this->_siege_ended        = ended;
    if (same_round == false || ended)
    {
        this->_siege_ending = false;
        this->_siege_winner = std::nullopt;
    }
    return true;
}

bool castle::siege_ending() const
{
    return this->_siege_ending;
}

void castle::siege_ending(bool value)
{
    this->_siege_ending = value;
}

const std::optional<std::optional<uint32_t>>& castle::siege_winner() const
{
    return this->_siege_winner;
}

void castle::siege_winner(const std::optional<uint32_t>& value)
{
    this->_siege_winner = std::make_optional(value);
}
