#ifndef __TRACE_H__
#define __TRACE_H__

#include <fb/lua.h>

#include <cstddef>
#include <cstdint>
#include <string>

namespace fb::game {

/**
 * In-game achievement: (id, text, icon, color). No table/model dependency.
 * id is passed when pushed via character::push_achievement(id, text, icon, color).
 */
class achievement : public lua::luable
{
public:
    LUA_PROTOTYPE

    static constexpr int64_t MAX_ICON  = 0xFF;
    static constexpr int64_t MAX_COLOR = 0xFF;
    static constexpr size_t  MAX_COUNT = 0xFF;

public:
    const uint32_t    id;
    const std::string text;
    const uint8_t     icon;
    const uint16_t    color;

public:
    achievement(uint32_t id, std::string text, uint8_t icon, uint16_t color);
    ~achievement() = default;
};

} // namespace fb::game

#endif
