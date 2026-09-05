#ifndef __PROTOCOL_GAME_INPUT_H__
#define __PROTOCOL_GAME_INPUT_H__

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
class dialog_input : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x2F;
    FB_PROTOCOL_VERSION_TAGS(V);
    using appearance_ptr = std::unique_ptr<fb::game::appearance>;

public:
    const appearance_ptr             appearance;
    const std::string                message;
    const uint32_t                   oid;
    const std::optional<std::string> ext;
    const uint16_t                   pursuit;

public:
    dialog_input(const fb::model::object&   obj,
                 std::string_view           message,
                 uint32_t                   oid     = 0xFFFFFFFD,
                 std::optional<std::string> ext     = std::nullopt,
                 uint16_t                   pursuit = 0xFFFF);
    dialog_input(const fb::game::object&    object,
                 std::string_view           message,
                 uint32_t                   oid     = 0xFFFFFFFD,
                 std::optional<std::string> ext     = std::nullopt,
                 uint16_t                   pursuit = 0xFFFF);
    dialog_input(appearance_ptr&&           appearance,
                 std::string_view           message,
                 uint32_t                   oid     = 0xFFFFFFFD,
                 std::optional<std::string> ext     = std::nullopt,
                 uint16_t                   pursuit = 0xFFFF);

public:
    void serialize(fb::stream_writer<big_endian>& writer) const;
};

template <>
void dialog_input<CLIENT_VERSION::v651>::serialize(fb::stream_writer<big_endian>& writer) const;

using dialog_input_v550 = dialog_input<CLIENT_VERSION::v550>;
using dialog_input_v565 = dialog_input<CLIENT_VERSION::v565>;
using dialog_input_v651 = dialog_input<CLIENT_VERSION::v651>;
#else
class dialog_input : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x2F;

public:
    uint16_t                   look;
    uint8_t                    color;
    std::string                message;
    uint8_t                    type_echo;
    uint32_t                   oid;
    std::optional<std::string> ext;
    uint16_t                   pursuit = 0xFFFF;

public:
    dialog_input() = default;

public:
    void deserialize(fb::stream_reader<big_endian>& reader);
};
#endif

} // namespace fb::protocol::game::response

#endif
