#include <fb/bot/cluster/journal.h>

using namespace fb::bot::cluster;

void journal::enable()
{
    this->_enabled = true;
}

void journal::record(Json::Value event)
{
    if (this->_enabled == false)
        return;

    auto lock = std::lock_guard(this->_mutex);
    this->_events.append(std::move(event));
}

Json::Value journal::drain()
{
    auto lock   = std::lock_guard(this->_mutex);
    auto events = Json::Value{Json::arrayValue};
    events.swapPayload(this->_events);
    return events;
}
