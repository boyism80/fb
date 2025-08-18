#ifndef __TRACE_H__
#define __TRACE_H__

#include <fb/model/model.h>
#include <fb/lua.h>

namespace fb::game {

class achievement : public lua::luable
{
public:
    LUA_PROTOTYPE

public:
    const fb::model::achievement&    model;
    const std::optional<std::string> text;
    const std::optional<uint8_t>     icon;
    const std::optional<uint16_t>    color;

public:
    achievement(const fb::model::achievement&     model,
                const std::optional<std::string>& text  = std::nullopt,
                const std::optional<uint8_t>&     icon  = std::nullopt,
                const std::optional<uint16_t>&    color = std::nullopt);
    ~achievement() = default;
};

} // namespace fb::game

#endif
