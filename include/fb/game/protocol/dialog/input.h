#ifndef __PROTOCOL_GAME_INPUT_H__
#define __PROTOCOL_GAME_INPUT_H__

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

class dialog_input : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x2F;
#ifndef BOT
    using appearance_ptr = std::unique_ptr<fb::game::appearance>;
#endif

public:
#ifndef BOT
    const appearance_ptr             appearance;
    const std::string                message;
    const uint32_t                   oid;
    const std::optional<std::string> ext;
    const uint16_t                   pursuit;
#else
    uint16_t                   look;
    uint8_t                    color;
    std::string                message;
    uint8_t                    type_echo;
    uint32_t                   oid;
    std::optional<std::string> ext;
    uint16_t                   pursuit = 0xFFFF;
#endif

public:
#ifndef BOT
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
#else
    dialog_input() = default;
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
