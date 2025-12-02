#include <fb/game/quest.h>
#include <fb/game/server.h>
#include <fb/game/character.h>
#include <fb/encoding.h>
#include <json/json.h>

using namespace fb::game;
using table = fb::model::table;

quest::quest(uint32_t id, std::weak_ptr<fb::game::character> owner) :
    id(id),
    owner(owner)
{ }

quest::quest(uint32_t id, std::weak_ptr<fb::game::character> owner, uint32_t step, uint32_t progress, bool completed, std::string param) :
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

void quest::param(const std::string& param)
{
    this->_param = param;
}

bool quest::inc_progress(uint32_t value)
{
    auto owner = this->owner.lock();
    if (owner == nullptr)
        return false;

    auto& server = owner->server;
    auto& model  = table::quest[this->id][this->_step];

    this->_progress = std::min(this->_progress + value, model.progress);
    return true;
}

bool quest::inc_step(uint32_t value)
{
    if (value == 0)
        return false;

    auto owner = this->owner.lock();
    if (owner == nullptr)
        return false;

    auto& server   = owner->server;
    auto  max_step = table::quest[this->id].size();

    for (int i = 0, remains = std::min(value, max_step - this->_step); i < remains; i++)
    {
        auto& model = table::quest[this->id][this->_step + i];
        if (model.step_reward.empty() == false)
        {
            auto& reward = table::reward[model.step_reward];
            if (owner->reward(reward.dsl) == false)
                return false;
        }
    }

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

    auto& server = owner->server;
    auto& attr   = table::quest_attribute[this->id];
    if (!attr.reward.empty() && table::reward.contains(attr.reward))
    {
        auto& reward = table::reward[attr.reward];
        if (owner->reward(reward.dsl) == false)
            return false;
    }

    this->_completed = true;

    // Log quest complete event
    auto log_data              = Json::Value();
    log_data["character_id"]   = static_cast<Json::Int64>(owner->id());
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

    if (table::quest.contains(id) == false)
        return false;

    auto& attr = table::quest_attribute[id];
    if (owner->condition(attr.condition) == false)
        return false;

    this->add(id, 0, 0, false, "");

    // Log quest start event
    auto log_data              = Json::Value();
    log_data["character_id"]   = static_cast<Json::Int64>(owner->id());
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