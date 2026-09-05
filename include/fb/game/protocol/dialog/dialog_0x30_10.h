#ifndef __PROTOCOL_GAME_DIALOG_0X30_10_H__
#define __PROTOCOL_GAME_DIALOG_0X30_10_H__

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
class dialog_0x30_10 : public fb::protocol::header
{
public:
    static constexpr uint8_t                     opcode = 0x30;
    static constexpr fb::game::dialog::list_type type   = fb::game::dialog::list_type::LOOK;
    FB_PROTOCOL_VERSION_TAGS(V);
    using appearance_ptr = std::unique_ptr<fb::game::appearance>;

public:
    const appearance_ptr appearance;
    const std::string    message;
    const bool           button_prev;
    const bool           button_next;
    const uint32_t       oid;

public:
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
    dialog_0x30_10(appearance_ptr&& appearance,
                   std::string_view message,
                   bool             button_prev = false,
                   bool             button_next = false,
                   uint32_t         oid         = 0xFFFFFFFD);

public:
    void serialize(fb::stream_writer<big_endian>& writer) const;
};

template <>
void dialog_0x30_10<CLIENT_VERSION::v651>::serialize(fb::stream_writer<big_endian>& writer) const;

using dialog_0x30_10_v550 = dialog_0x30_10<CLIENT_VERSION::v550>;
using dialog_0x30_10_v565 = dialog_0x30_10<CLIENT_VERSION::v565>;
using dialog_0x30_10_v651 = dialog_0x30_10<CLIENT_VERSION::v651>;
#else
class dialog_0x30_10 : public fb::protocol::header
{
public:
    static constexpr uint8_t                     opcode = 0x30;
    static constexpr fb::game::dialog::list_type type   = fb::game::dialog::list_type::LOOK;

public:
    uint8_t     type_echo = 0;
    uint32_t    oid       = 0;
    std::string message;
    bool        button_prev = false;
    bool        button_next = false;

public:
    dialog_0x30_10() = default;

public:
    void deserialize(fb::stream_reader<big_endian>& reader);
};
#endif

} // namespace fb::protocol::game::response

#endif
