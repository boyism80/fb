#ifndef __PROTOCOL_GAME_UI_SCREEN_H__
#define __PROTOCOL_GAME_UI_SCREEN_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>

namespace fb::protocol::game::response {

using namespace fb::model::enum_value;

enum class UI_SCREEN : uint8_t
{
    INVENTORY   = 0, ///< Inventory window
    SKILL       = 2, ///< Skill window
    STATUS      = 4, ///< Status window
    BULLETIN    = 6, ///< Bulletin board
    DETAIL      = 7, ///< Detail status
    ACHIEVEMENT = 8, ///< Achievement window
};

class ui_screen : public fb::protocol::header
{
public:
    static constexpr uint8_t header = 0x3E;

public:
#ifndef BOT
    const UI_SCREEN screen;
#else
    UI_SCREEN screen;
#endif

public:
#ifndef BOT
    explicit ui_screen(UI_SCREEN screen) :
        screen(screen)
    { }
#else
    ui_screen() = default;
#endif

public:
#ifndef BOT
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const;
#else
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

} // namespace fb::protocol::game::response

#endif
