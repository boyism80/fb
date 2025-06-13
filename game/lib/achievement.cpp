#include <fb/game/achievement.h>

using namespace fb::game;

achievement::achievement(const fb::model::achievement& model, const std::optional<std::string>& text) :
    model(model),
    text(text)
{ }