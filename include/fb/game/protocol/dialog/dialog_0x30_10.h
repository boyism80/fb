#ifndef __PROTOCOL_GAME_DIALOG_0X30_10_H__
#define __PROTOCOL_GAME_DIALOG_0X30_10_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>
#ifndef BOT
#include <fb/game/dialog.h>
#else
#include <fb/game/dialog_type.h>
#endif
#include <string_view>

namespace fb::protocol::game::response {

class dialog_0x30_10 : public fb::protocol::header
{
public:
    static constexpr uint8_t                     opcode = 0x30;
    static constexpr fb::game::dialog::list_type type   = fb::game::dialog::list_type::LOOK;
#ifndef BOT
    using appearance_ptr = std::unique_ptr<fb::game::appearance>;
#endif

public:
#ifndef BOT
    const appearance_ptr appearance;
    const std::string    message;
    const bool           button_prev;
    const bool           button_next;
    const uint32_t       oid;
#else
    uint8_t     type_echo = 0;
    uint32_t    oid       = 0;
    std::string message;
    bool        button_prev = false;
    bool        button_next = false;
#endif

public:
#ifndef BOT
    dialog_0x30_10(const fb::model::object& obj,
                   std::string_view         message,
                   bool                     button_prev = false,
                   bool                     button_next = false,
                   uint32_t                 oid         = 0xFFFFFFFD);
    dialog_0x30_10(const fb::game::object& object,
                   std::string_view        message,
                   bool                    button_prev = false,
                   bool                    button_next = false,
                   uint32_t                oid         = 0xFFFFFFFD);
#else
    dialog_0x30_10() = default;
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
