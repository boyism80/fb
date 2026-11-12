#include <fb/game/character.h>

#include <fb/lua.h>

#include <cstdint>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

using namespace fb::game;

void character::dialog::open(fb::lua::context* context, fb::game::dialog::reply reply, uint16_t limit)
{
    this->cancel("dialog replaced");
    this->_context = context;
    this->_reply   = reply;
    this->_limit   = limit;
}

void character::dialog::open(fb::lua::context* context, fb::game::dialog::reply reply, std::vector<std::string> choices)
{
    this->open(context, reply);
    this->_choices = std::move(choices);
}

character::dialog::pending character::dialog::take(fb::game::dialog::reply reply)
{
    if (this->_context == nullptr)
        return pending{};

    if (this->_reply != reply)
    {
        this->cancel("dialog reply mismatch");
        return pending{};
    }

    auto taken     = pending{this->_context, this->_limit, std::move(this->_choices)};
    this->_context = nullptr;
    this->_choices.clear();
    return taken;
}

bool character::dialog::opened() const
{
    return this->_context != nullptr;
}

void character::dialog::cancel(std::string_view reason)
{
    auto context   = this->_context;
    this->_context = nullptr;
    this->_choices.clear();
    if (context != nullptr)
        context->cancel(reason);
}
