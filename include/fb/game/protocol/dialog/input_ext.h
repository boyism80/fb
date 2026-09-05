#ifndef __PROTOCOL_GAME_INPUT_EXT_H__
#define __PROTOCOL_GAME_INPUT_EXT_H__

#include <fb/protocol/header.h>
#include <fb/protocol/client_version.h>
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

#ifndef BOT
template <CLIENT_VERSION V>
class dialog_input_ext : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x30;
    FB_PROTOCOL_VERSION_TAGS(V);
    using appearance_ptr = std::unique_ptr<fb::game::appearance>;

public:
    const appearance_ptr             appearance;
    const std::optional<std::string> message; // nullopt → subtype 5/8 (no text)
    const std::string                top, bottom;
    const int                        maxlen;
    const bool                       button_prev;
    const bool                       password; // false → 4/5; true → 7/8
    const uint32_t                   oid;

public:
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
    dialog_input_ext(appearance_ptr&&           appearance,
                     std::optional<std::string> message,
                     std::string_view           top,
                     std::string_view           bottom,
                     int                        maxlen,
                     bool                       button_prev,
                     bool                       password = false,
                     uint32_t                   oid      = 0xFFFFFFFD);

public:
    void serialize(fb::stream_writer<big_endian>& writer) const;
};

template <>
void dialog_input_ext<CLIENT_VERSION::v651>::serialize(fb::stream_writer<big_endian>& writer) const;

using dialog_input_ext_v550 = dialog_input_ext<CLIENT_VERSION::v550>;
using dialog_input_ext_v565 = dialog_input_ext<CLIENT_VERSION::v565>;
using dialog_input_ext_v651 = dialog_input_ext<CLIENT_VERSION::v651>;
#else
class dialog_input_ext : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x30;

public:
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

public:
    dialog_input_ext() = default;

public:
    void deserialize(fb::stream_reader<big_endian>& reader);
};
#endif

} // namespace fb::protocol::game::response

#endif
