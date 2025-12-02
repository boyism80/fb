#include <fb/game/achievement.h>

using namespace fb::game;

achievement::achievement(const fb::model::achievement& model, const std::optional<std::string>& text, const std::optional<uint8_t>& icon, const std::optional<uint16_t>& color) :
    model(model),
    text(text),
    icon(icon),
    color(color)
{ }