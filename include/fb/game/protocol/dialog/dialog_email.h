#ifndef __PROTOCOL_GAME_DIALOG_EMAIL_H__
#define __PROTOCOL_GAME_DIALOG_EMAIL_H__

#include <fb/protocol/header.h>
#include <fb/protocol/client_version.h>
#include <fb/model/model.h>
#ifndef BOT
#include <fb/game/dialog.h>
#else
#include <fb/game/dialog_type.h>
#endif
#include <string_view>

namespace fb::protocol::game::response {

#ifndef BOT
template <CLIENT_VERSION V>
class dialog_email : public fb::protocol::header
{
public:
    static constexpr uint8_t                     opcode = 0x30;
    static constexpr fb::game::dialog::list_type type   = fb::game::dialog::list_type::EMAIL;
    FB_PROTOCOL_VERSION_TAGS(V);
    using appearance_ptr = std::unique_ptr<fb::game::appearance>;

public:
    const appearance_ptr appearance;
    const std::string    message;
    const std::string    str1;
    const std::string    str2;
    const bool           button_prev;
    const bool           button_next;
    const uint32_t       oid;

public:
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
    dialog_email(appearance_ptr&& appearance,
                 std::string_view message,
                 std::string_view str1,
                 std::string_view str2,
                 bool             button_prev = false,
                 bool             button_next = false,
                 uint32_t         oid         = 0xFFFFFFFD);

public:
    void serialize(fb::stream_writer<big_endian>& writer) const;
};

template <>
void dialog_email<CLIENT_VERSION::v651>::serialize(fb::stream_writer<big_endian>& writer) const;

using dialog_email_v550 = dialog_email<CLIENT_VERSION::v550>;
using dialog_email_v565 = dialog_email<CLIENT_VERSION::v565>;
using dialog_email_v651 = dialog_email<CLIENT_VERSION::v651>;
#else
class dialog_email : public fb::protocol::header
{
public:
    static constexpr uint8_t                     opcode = 0x30;
    static constexpr fb::game::dialog::list_type type   = fb::game::dialog::list_type::EMAIL;

public:
    uint8_t     type_echo = 0;
    uint32_t    oid       = 0;
    std::string message;
    std::string str1;
    std::string str2;
    bool        button_prev = false;
    bool        button_next = false;

public:
    dialog_email() = default;

public:
    void deserialize(fb::stream_reader<big_endian>& reader);
};
#endif

} // namespace fb::protocol::game::response

#endif
