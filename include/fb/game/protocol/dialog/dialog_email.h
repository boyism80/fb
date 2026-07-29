#ifndef __PROTOCOL_GAME_DIALOG_EMAIL_H__
#define __PROTOCOL_GAME_DIALOG_EMAIL_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>
#ifndef BOT
#include <fb/game/dialog.h>
#else
#include <fb/game/dialog_type.h>
#endif
#include <string_view>

namespace fb::protocol::game::response {

class dialog_email : public fb::protocol::header
{
public:
    static constexpr uint8_t                     opcode = 0x30;
    static constexpr fb::game::dialog::list_type type   = fb::game::dialog::list_type::EMAIL;
#ifndef BOT
    using appearance_ptr = std::unique_ptr<fb::game::appearance>;
#endif

public:
#ifndef BOT
    const appearance_ptr appearance;
    const std::string    message;
    const std::string    str1;
    const std::string    str2;
    const bool           button_prev;
    const bool           button_next;
    const uint32_t       oid;
#else
    uint8_t     type_echo = 0;
    uint32_t    oid       = 0;
    std::string message;
    std::string str1;
    std::string str2;
    bool        button_prev = false;
    bool        button_next = false;
#endif

public:
#ifndef BOT
    dialog_email(const fb::model::object& obj,
                 std::string_view         message,
                 std::string_view         str1,
                 std::string_view         str2,
                 bool                     button_prev = false,
                 bool                     button_next = false,
                 uint32_t                 oid         = 0xFFFFFFFD);
    dialog_email(const fb::game::object& object,
                 std::string_view        message,
                 std::string_view        str1,
                 std::string_view        str2,
                 bool                    button_prev = false,
                 bool                    button_next = false,
                 uint32_t                oid         = 0xFFFFFFFD);
#else
    dialog_email() = default;
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
