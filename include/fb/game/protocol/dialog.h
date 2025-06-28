#ifndef __PROTOCOL_GAME_DIALOG_H__
#define __PROTOCOL_GAME_DIALOG_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>
#ifndef BOT
#include <fb/game/character.h>
#endif

using namespace fb::model::enum_value;

namespace fb::protocol::game::request {

class dialog : public fb::protocol::header
{
public:
    static constexpr uint8_t header = 0x3A;

public:
#ifdef BOT
    const uint8_t       interaction;
    const uint8_t       action;
    const std::string   message;
    const uint16_t      index;
    const uint16_t      pursuit;
    const std::string   name;
    const DIALOG_RESULT button;
#else
    fb::game::dialog::interaction interaction;
    uint8_t                       action;
    std::string                   message;
    uint16_t                      index;
    uint16_t                      pursuit;
    std::string                   name;
    DIALOG_RESULT                 button;
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