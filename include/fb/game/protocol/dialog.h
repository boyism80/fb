#ifndef __PROTOCOL_GAME_DIALOG_H__
#define __PROTOCOL_GAME_DIALOG_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>
#ifndef BOT
#include <fb/game/dialog.h>
#endif

using namespace fb::model::enum_value;

namespace fb::protocol::game::request {

class dialog : public fb::protocol::header
{
public:
    inline static uint8_t header = 0x3A;

public:
#ifndef BOT
    fb::game::dialog::interaction interaction;
    uint8_t                       action;  // NORMAL
    std::string                   message; // INPUT
    uint16_t                      index;   // MENU
    uint16_t                      pursuit; // SELL
    std::string                   name;    // SELL
#else

#endif

public:
    dialog() = default;

public:
#ifdef BOT
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const;
#else
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

} // namespace fb::protocol::game::request

#endif