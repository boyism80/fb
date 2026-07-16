#include <fb/game/server.h>
#include <fb/game/character.h>
#include <fb/game/item.h>
#include <fb/encoding.h>
#include <json/json.h>

using namespace fb::game;

consume::consume(fb::game::server& server, const fb::model::consume& model, const initial_params& params) :
    fb::game::item(server, model, params)
{ }

consume::consume(const consume& right) :
    fb::game::item(right)
{ }

consume::~consume()
{ }

async::task<bool> consume::active()
{
    if (this->_container == nullptr)
        co_return false;

    auto owner = this->_container->owner();
    if (owner == nullptr)
        co_return false;

    if (this->_count == 0)
        co_return false;

    std::ignore = co_await fb::game::item::active();
    this->_count--;

    owner->action(ACTION::EAT, DURATION::EAT);
    owner->sound(SOUND::EAT);
    owner->listener.on_item_update(*owner, owner->items.index(this->shared_from_this_as<fb::game::item>()));

    // Log item consume event
    auto log_data               = Json::Value();
    log_data["character_id"]    = static_cast<Json::Int64>(owner->id);
    log_data["character_name"]  = UTF8(owner->name(), PLATFORM::WINDOWS);
    log_data["item_id"]         = static_cast<Json::Int64>(this->based<fb::model::item>().id);
    log_data["item_name"]       = UTF8(this->name(), PLATFORM::WINDOWS);
    log_data["remaining_count"] = static_cast<Json::Int64>(this->_count);
    owner->server.log.write("item_consume", log_data);

    if (this->empty())
        std::ignore = owner->items.remove(this->shared_from_this_as<fb::game::item>(), -1, ITEM_DELETE_TYPE::EAT);

    co_return true;
}