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
    _owner_clan_id(owner_clan_id),
    _siege_end_at(server.now()),
    _siege_active_until(server.now())
{ }

castle::castle(castle&& r) :
    _server(r._server),
    _divine_beast(r._divine_beast),
    _owner_clan_id(r._owner_clan_id),
    _siege_id(r._siege_id),
    _siege_end_at(r._siege_end_at),
    _siege_active_until(r._siege_active_until),
    _revision(r._revision),
    _phase(r._phase),
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

    return this->_server.now() < this->_siege_active_until;
}

bool castle::siege_pending() const
{
    return this->_siege_id.has_value() && this->_phase != siege_phase::ENDED;
}

const std::optional<uint64_t>& castle::siege_id() const
{
    return this->_siege_id;
}

std::optional<uint32_t> castle::siege_remaining_seconds() const
{
    if (this->siege_pending() == false)
        return std::nullopt;

    auto now = this->_server.now();
    if (now >= this->_siege_end_at)
        return 0;

    auto remaining = this->_siege_end_at - now;
    return static_cast<uint32_t>(remaining.total_milliseconds() / 1000);
}

bool castle::load(const castle_dto& row)
{
    if (row.revision < this->_revision)
        return false;

    auto now        = this->_server.now();
    auto same_round = this->_siege_id == row.siege_id;

    this->_revision           = row.revision;
    this->_owner_clan_id      = row.owner_clan_id;
    this->_siege_id           = row.siege_id;
    this->_siege_end_at       = now + fb::model::timespan(std::chrono::seconds(row.siege_remaining));
    this->_siege_active_until = now + fb::model::timespan(std::chrono::seconds(row.siege_active_remaining));
    if (row.siege_ended)
    {
        this->_phase        = siege_phase::ENDED;
        this->_siege_winner = std::nullopt;
    }
    else if (same_round == false || this->_phase == siege_phase::ENDED)
    {
        this->_phase        = siege_phase::RUNNING;
        this->_siege_winner = std::nullopt;
    }
    return true;
}

castle::siege_phase castle::phase() const
{
    return this->_phase;
}

void castle::phase(siege_phase value)
{
    this->_phase = value;
}

const std::optional<uint32_t>& castle::siege_winner() const
{
    return this->_siege_winner;
}

void castle::siege_winner(const std::optional<uint32_t>& value)
{
    this->_siege_winner = value;
}
