#include <fb/game/character.h>

#include <fb/lua.h>

#include <cstdint>
#include <string_view>

using namespace fb::game;

void character::dialog::open(fb::lua::context* context, uint8_t maxlen, uint16_t items)
{
    this->cancel("dialog replaced");
    this->_context = context;
    this->_maxlen  = maxlen;
    this->_items   = items;
}

fb::lua::context* character::dialog::close()
{
    auto context   = this->_context;
    this->_context = nullptr;
    this->_maxlen  = 0xFF;
    this->_items   = 0;
    return context;
}

uint8_t character::dialog::maxlen() const
{
    return this->_maxlen;
}

uint16_t character::dialog::items() const
{
    return this->_items;
}

void character::dialog::cancel(std::string_view reason)
{
    auto context = this->close();
    if (context != nullptr)
        context->cancel(reason);
}
