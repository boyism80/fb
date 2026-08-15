#ifndef __PROTOCOL_GAME_DIALOG_H__
#define __PROTOCOL_GAME_DIALOG_H__

#include <fb/protocol/header.h>
#include <fb/protocol/client_version.h>
#include <fb/model/model.h>
#include <fb/game/dialog_type.h>
#ifndef BOT
#include <fb/game/character.h>
#include <fb/game/dialog.h>
#endif
#include <optional>
#include <string_view>

namespace fb::protocol::game::request {

using namespace fb::model::enum_value;

template <CLIENT_VERSION V>
class dialog : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x39;
    FB_PROTOCOL_VERSION_TAGS(V);

public:
#ifdef BOT
    const fb::game::dialog::type type;
    const uint32_t               oid;
    const uint16_t               seq;
    const uint8_t                action;
    const std::string            message;
    const uint16_t               index;
    const uint16_t               pursuit;
    const std::string            name;
    const std::string            ext; // S2C echo blob (MENU/INPUT only)
#else
    fb::game::dialog::type type   = fb::game::dialog::type::MENU;
    uint32_t               oid    = 0;
    uint16_t               seq    = 0;
    uint8_t                action = 0;
    std::string            message;
    uint16_t               index   = 0;
    uint16_t               pursuit = 0;
    std::string            name;
    std::string            ext; // S2C echo blob (MENU/INPUT only)
#endif

public:
#ifndef BOT
    dialog() = default;
#else
    dialog(fb::game::dialog::type type,
           uint8_t                action,
           std::string            message,
           uint16_t               index,
           uint16_t               pursuit,
           std::string            name,
           uint32_t               oid = 0,
           uint16_t               seq = 0,
           std::string            ext = {});
#endif

public:
#ifdef BOT
    void serialize(fb::stream_writer<big_endian>& writer) const;
#else
    void deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

template <>
class dialog<CLIENT_VERSION::v651> : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x39;
    FB_PROTOCOL_VERSION_TAGS(CLIENT_VERSION::v651);

public:
#ifdef BOT
    const fb::game::dialog::type type;
    const uint32_t               oid;
    const uint16_t               seq;
    const uint8_t                action;
    const std::string            message;
    const uint16_t               index;
    const uint16_t               pursuit;
    const std::string            name;
    const std::string            ext;
    const uint32_t               item_value;
#else
    fb::game::dialog::type type   = fb::game::dialog::type::MENU;
    uint32_t               oid    = 0;
    uint16_t               seq    = 0;
    uint8_t                action = 0;
    std::string            message;
    uint16_t               index   = 0;
    uint16_t               pursuit = 0;
    std::string            name;
    std::string            ext;
    uint32_t               item_value = 0;
#endif

public:
#ifndef BOT
    dialog() = default;
#else
    dialog(fb::game::dialog::type type,
           uint8_t                action,
           std::string            message,
           uint16_t               index,
           uint16_t               pursuit,
           std::string            name,
           uint32_t               oid        = 0,
           uint16_t               seq        = 0,
           std::string            ext        = {},
           uint32_t               item_value = 0);
#endif

public:
#ifdef BOT
    void serialize(fb::stream_writer<big_endian>& writer) const;
#else
    void deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

template <CLIENT_VERSION V>
class dialog_list : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x3A;
    FB_PROTOCOL_VERSION_TAGS(V);

public:
#ifdef BOT
    const fb::game::dialog::list_type type;
    const uint32_t                    oid;
    const uint16_t                    seq;
    const uint8_t                     action;
    const std::string                 message;
    const uint16_t                    index;
    const DIALOG_RESULT               button;
#else
    fb::game::dialog::list_type type   = fb::game::dialog::list_type::TEXT;
    uint32_t                    oid    = 0;
    uint16_t                    seq    = 0;
    uint8_t                     action = 0;
    std::string                 message;
    uint16_t                    index  = 0;
    DIALOG_RESULT               button = DIALOG_RESULT::QUIT;
#endif

public:
#ifndef BOT
    dialog_list() = default;
#else
    dialog_list(fb::game::dialog::list_type type,
                uint8_t                     action,
                std::string                 message,
                uint16_t                    index,
                DIALOG_RESULT               button,
                uint32_t                    oid = 0,
                uint16_t                    seq = 0);
#endif

public:
#ifdef BOT
    void serialize(fb::stream_writer<big_endian>& writer) const;
#else
    void deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

} // namespace fb::protocol::game::request

namespace fb::protocol::game::response {

using namespace fb::model::enum_value;

class dialog : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x30;
#ifndef BOT
    using appearance_ptr = std::unique_ptr<fb::game::appearance>;
#endif

public:
#ifndef BOT
    const appearance_ptr             appearance;
    const std::optional<std::string> message; // nullopt → subtype 1 (no text)
    const bool                       button_prev;
    const bool                       button_next;
    const uint32_t                   oid;
#else
    uint16_t                   look;
    uint8_t                    color;
    std::optional<std::string> message;
    bool                       button_prev;
    bool                       button_next;
    uint8_t                    type_echo;
    uint32_t                   oid;
#endif

public:
#ifndef BOT
    dialog(std::optional<std::string> message, bool button_prev, bool button_next, uint32_t oid = 0xFFFFFFFD);

    dialog(const fb::model::object&   object,
           std::optional<std::string> message,
           bool                       button_prev,
           bool                       button_next,
           uint32_t                   oid = 0xFFFFFFFD);

    dialog(const fb::game::object&    object,
           std::optional<std::string> message,
           bool                       button_prev,
           bool                       button_next,
           uint32_t                   oid = 0xFFFFFFFD);
#else
    dialog() = default;
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
