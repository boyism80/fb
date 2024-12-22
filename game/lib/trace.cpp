#include <trace.h>

using namespace fb::game;

trace::trace(const fb::model::trace& model, const std::optional<std::string>& text) :
    model(model),
    text(text)
{ }