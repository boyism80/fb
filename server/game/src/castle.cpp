#include <fb/game/castle.h>
#include <fb/game/server.h>

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
    _siege_active(r._siege_active),
    _siege_end_at(r._siege_end_at)
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
    return this->_siege_active;
}

std::optional<uint32_t> castle::siege_remaining_seconds() const
{
    if (this->_siege_active == false)
        return std::nullopt;

    auto now = std::chrono::steady_clock::now();
    if (now >= this->_siege_end_at)
        return 0;

    auto remaining = std::chrono::duration_cast<std::chrono::seconds>(this->_siege_end_at - now);
    return static_cast<uint32_t>(remaining.count());
}

void castle::start_siege(uint32_t duration_seconds)
{
    this->_siege_active = true;
    this->_siege_end_at = std::chrono::steady_clock::now() + std::chrono::seconds(duration_seconds);
}

void castle::clear_siege()
{
    this->_siege_active = false;
    this->_siege_end_at = std::chrono::steady_clock::time_point{};
}
