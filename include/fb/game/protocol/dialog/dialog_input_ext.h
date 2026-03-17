#ifndef __PROTOCOL_GAME_INPUT_EXT_H__
#define __PROTOCOL_GAME_INPUT_EXT_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>
#ifndef BOT
#include <fb/game/dialog.h>
#endif
#include <string_view>

namespace fb::protocol::game::response {

using namespace fb::model::enum_value;

class dialog_input_ext : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x30;
#ifndef BOT
    using appearance_ptr = std::unique_ptr<fb::game::appearance>;
#endif

public:
#ifndef BOT
    const appearance_ptr                appearance;
    const std::vector<uint8_t>          slots;
    const std::string                   message;
    const std::string                   top, bottom;
    const int                           maxlen;
    const bool                          button_prev;
    const fb::game::dialog::interaction interaction;
    const uint32_t                      oid;
#else
    uint16_t    look;
    uint8_t     color;
    std::string message;
    std::string top;
    std::string bottom;
    uint8_t     maxlen;
    bool        button_prev;
    uint8_t     interaction;
    uint32_t    oid;
#endif

public:
#ifndef BOT
    dialog_input_ext(const fb::model::object&      obj,
                     std::string_view              message,
                     std::string_view              top,
                     std::string_view              bottom,
                     int                           maxlen      = 0xFF,
                     bool                          button_prev = false,
                     uint32_t                      oid         = 0xFFFFFFFD,
                     fb::game::dialog::interaction interaction = fb::game::dialog::interaction::INPUT_EX);
    dialog_input_ext(const fb::game::object&       object,
                     std::string_view              message,
                     std::string_view              top,
                     std::string_view              bottom,
                     int                           maxlen      = 0xFF,
                     bool                          button_prev = false,
                     uint32_t                      oid         = 0xFFFFFFFD,
                     fb::game::dialog::interaction interaction = fb::game::dialog::interaction::INPUT_EX);
#else
    dialog_input_ext() = default;
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