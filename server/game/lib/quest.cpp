#include <fb/game/quest.h>
#include <fb/game/server.h>
#include <fb/game/character.h>

using namespace fb::game;

quest::quest(uint32_t id, std::weak_ptr<fb::game::character> owner) :
    id(id),
    owner(owner)
{ }

quest::quest(uint32_t id, std::weak_ptr<fb::game::character> owner, uint32_t step, uint32_t progress, bool completed) :
    id(id),
    owner(owner),
    _step(step),
    _progress(progress),
    _completed(completed)
{ }

uint32_t quest::step() const
{
    return this->_step;
}

uint32_t quest::progress() const
{
    return this->_progress;
}

bool quest::inc_progress(uint32_t value)
{
    auto owner = this->owner.lock();
    if (owner == nullptr)
        return false;

    auto& server = owner->server;
    auto& model  = server.model.quest[this->id][this->_step];

    this->_progress = std::min(this->_progress + value, model.progress);
    return model.progress == this->_progress;
}

bool quest::inc_step(uint32_t value)
{
    if (value == 0)
        return false;

    auto owner = this->owner.lock();
    if (owner == nullptr)
        return false;

    auto& server   = owner->server;
    auto  max_step = server.model.quest[this->id].size();

    for (int i = 0, remains = std::min(value, max_step - this->_step); i < remains; i++)
    {
        auto& model = server.model.quest[this->id][this->_step + i];
        if (model.step_reward.empty() == false)
        {
            auto& reward = server.model.reward[model.step_reward];
            if (owner->reward(reward.dsl) == false)
                return false;
        }
    }

    this->_step     += value;
    this->_progress  = 0;
    return this->_step == max_step;
}

bool quest::complete()
{
    if (this->_completed)
        return true;

    auto owner = this->owner.lock();
    if (owner == nullptr)
        return false;

    auto& server = owner->server;
    auto& attr   = server.model.quest_attribute[this->id];
    if (!attr.reward.empty() && server.model.reward.contains(attr.reward))
    {
        auto& reward = server.model.reward[attr.reward];
        if (owner->reward(reward.dsl) == false)
            return false;
    }

    this->_completed = true;
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

void quests::add(uint32_t id, uint32_t step, uint32_t progress, bool completed)
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        return;

    this->insert({id, std::make_shared<quest>(id, this->_owner, step, progress, completed)});
}

void quests::remove(uint32_t id)
{
    this->erase(id);
}