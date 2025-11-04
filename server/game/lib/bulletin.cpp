#include <fb/game/bulletin.h>
#include <fb/game/character.h>

using namespace fb::game;
using namespace fb::model::enum_value;

bulletin::bulletin(character& owner) :
    owner(owner)
{ }

void bulletin::show()
{
    this->owner.listener.on_show_bulletin(this->owner);
}

void bulletin::show(const fb::model::bulletin& model, const std::list<article>& articles, BULLETIN_BUTTON_ENABLE flag)
{
    this->owner.listener.on_show_bulletin(this->owner, model, articles, flag);
}

void bulletin::show(const article& article, BULLETIN_BUTTON_ENABLE flag)
{
    this->owner.listener.on_show_bulletin(this->owner, article, flag);
}

void bulletin::message(const std::string& message, bool success)
{
    this->owner.listener.on_show_bulletin_message(this->owner, message, success, false);
}

bool bulletin::section::writable(uint8_t level, bool admin) const
{
    if (admin)
        return true;

    if (this->admin)
        return false;

    if (this->min_level != std::nullopt && level < this->min_level.value())
        return false;

    if (this->max_level != std::nullopt && level > this->max_level.value())
        return false;

    return true;
}