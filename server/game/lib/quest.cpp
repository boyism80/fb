#include <fb/game/quest.h>
#include <fb/game/server.h>
#include <fb/game/character.h>
#include <fb/encoding.h>
#include <json/json.h>

using namespace fb::game;

quest::quest(uint32_t id, std::weak_ptr<fb::game::character> owner) :
    id(id),
    owner(owner)
{ }

quest::quest(uint32_t                           id,
             std::weak_ptr<fb::game::character> owner,
             uint32_t                           step,
             uint32_t                           progress,
             bool                               completed,
             std::string                        param) :
    id(id),
    owner(owner),
    _step(step),
    _progress(progress),
    _completed(completed),
    _param(param)
{ }

uint32_t quest::step() const
{
    return this->_step;
}

void quest::step(uint32_t step)
{
    this->_step = step;
}

void quest::progress(uint32_t progress)
{
    this->_progress = progress;
}

uint32_t quest::progress() const
{
    return this->_progress;
}

std::string quest::param() const
{
    return this->_param;
}

void quest::param(std::string_view param)
{
    this->_param = std::string(param);
}

bool quest::inc_progress(uint32_t value)
{
    auto owner = this->owner.lock();
    if (owner == nullptr)
        return false;

    this->_progress += value;
    return true;
}

bool quest::inc_step(uint32_t value)
{
    if (value == 0)
        return false;

    auto owner = this->owner.lock();
    if (owner == nullptr)
        return false;

    this->_step     += value;
    this->_progress  = 0;
    this->_param     = "";
    return true;
}

bool quest::complete()
{
    if (this->_completed)
        return true;

    auto owner = this->owner.lock();
    if (owner == nullptr)
        return false;

    this->_completed = true;

    auto log_data              = Json::Value();
    log_data["character_id"]   = static_cast<Json::Int64>(owner->id);
    log_data["character_name"] = UTF8(owner->name(), PLATFORM::WINDOWS);
    log_data["quest_id"]       = static_cast<Json::Int64>(this->id);
    log_data["step"]           = static_cast<Json::Int64>(this->_step);
    owner->server.log.write("quest_complete", log_data);

    return true;
}

bool quest::completed() const
{
    return this->_completed;
}

bool quest::resume()
{
    this->_completed = false;
    return true;
}

void quests::owner(std::weak_ptr<fb::game::character> owner)
{
    this->_owner = owner;
}

void quests::add(uint32_t id, uint32_t step, uint32_t progress, bool completed, std::string param)
{
    this->insert({id, std::make_shared<quest>(id, this->_owner, step, progress, completed, param)});
}

bool quests::start(uint32_t id)
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        return false;

    if (this->contains(id))
        return true;

    this->add(id, 0, 0, false, "");

    auto log_data              = Json::Value();
    log_data["character_id"]   = static_cast<Json::Int64>(owner->id);
    log_data["character_name"] = UTF8(owner->name(), PLATFORM::WINDOWS);
    log_data["quest_id"]       = static_cast<Json::Int64>(id);
    owner->server.log.write("quest_start", log_data);

    return true;
}

bool quests::remove(uint32_t id)
{
    if (this->contains(id) == false)
        return false;

    this->erase(id);
    return true;
}