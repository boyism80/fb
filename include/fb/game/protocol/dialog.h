#ifndef __PROTOCOL_GAME_DIALOG_H__
#define __PROTOCOL_GAME_DIALOG_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>
#include <fb/game/dialog_type.h>
#ifndef BOT
#include <fb/game/character.h>
#include <fb/game/dialog.h>
#endif
#include <string_view>

namespace fb::protocol::game::request {

using namespace fb::model::enum_value;

class dialog : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x39;

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
#else
    fb::game::dialog::type type   = fb::game::dialog::type::MENU;
    uint32_t               oid    = 0;
    uint16_t               seq    = 0;
    uint8_t                action = 0;
    std::string            message;
    uint16_t               index   = 0;
    uint16_t               pursuit = 0;
    std::string            name;
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
           uint16_t               seq = 0) :
        type(type),
        oid(oid),
        seq(seq),
        action(action),
        message(std::move(message)),
        index(index),
        pursuit(pursuit),
        name(std::move(name))
    { }
#endif

public:
#ifdef BOT
    void serialize(fb::stream_writer<big_endian>& writer) const;
#else
    void deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

class dialog_list : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x3A;

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
                uint16_t                    seq = 0) :
        type(type),
        oid(oid),
        seq(seq),
        action(action),
        message(std::move(message)),
        index(index),
        button(button)
    { }
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
    static constexpr uint8_t                     opcode = 0x30;
    static constexpr fb::game::dialog::list_type type   = fb::game::dialog::list_type::TEXT;
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
    uint16_t    look;
    uint8_t     color;
    std::string message;
    bool        button_prev;
    bool        button_next;
    uint8_t     type_echo;
    uint32_t    oid;
#endif

public:
#ifndef BOT
    dialog(std::string_view message, bool button_prev, bool button_next, uint32_t oid = 0xFFFFFFFD);

    dialog(const fb::model::object& object,
           std::string_view         message,
           bool                     button_prev,
           bool                     button_next,
           uint32_t                 oid = 0xFFFFFFFD);

    dialog(const fb::game::object& object,
           std::string_view        message,
           bool                    button_prev,
           bool                    button_next,
           uint32_t                oid = 0xFFFFFFFD);
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
