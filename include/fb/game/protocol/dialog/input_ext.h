#ifndef __PROTOCOL_GAME_INPUT_EXT_H__
#define __PROTOCOL_GAME_INPUT_EXT_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>
#ifndef BOT
#include <fb/game/dialog.h>
#else
#include <fb/game/dialog_type.h>
#endif
#include <optional>
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
    const appearance_ptr             appearance;
    const std::optional<std::string> message; // nullopt → subtype 5/8 (no text)
    const std::string                top, bottom;
    const int                        maxlen;
    const bool                       button_prev;
    const bool                       password; // false → 4/5; true → 7/8
    const uint32_t                   oid;
#else
    uint16_t                   look;
    uint8_t                    color;
    std::optional<std::string> message;
    std::string                top;
    std::string                bottom;
    uint8_t                    maxlen;
    bool                       button_prev;
    bool                       password = false;
    uint8_t                    type_echo;
    uint32_t                   oid;
#endif

public:
#ifndef BOT
    dialog_input_ext(const fb::model::object&   obj,
                     std::optional<std::string> message,
                     std::string_view           top,
                     std::string_view           bottom,
                     int                        maxlen,
                     bool                       button_prev,
                     bool                       password = false,
                     uint32_t                   oid      = 0xFFFFFFFD);

    dialog_input_ext(const fb::game::object&    object,
                     std::optional<std::string> message,
                     std::string_view           top,
                     std::string_view           bottom,
                     int                        maxlen,
                     bool                       button_prev,
                     bool                       password = false,
                     uint32_t                   oid      = 0xFFFFFFFD);
#else
    dialog_input_ext() = default;
#endif

public:
#ifndef BOT
    void serialize(fb::stream_writer<big_endian>& writer) const;
#else
    void deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

} // namespace fb::protocol::game::response

#endif
