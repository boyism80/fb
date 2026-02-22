#include <fb/game/achievement.h>

using namespace fb::game;

achievement::achievement(uint32_t id, std::string text, uint8_t icon, uint16_t color)
    : id(id)
    , text(std::move(text))
    , icon(icon)
    , color(color)
{
}