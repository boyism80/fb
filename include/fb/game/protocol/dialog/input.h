#ifndef __PROTOCOL_GAME_INPUT_H__
#define __PROTOCOL_GAME_INPUT_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>
#ifndef BOT
#include <fb/game/dialog.h>
#endif
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
    const appearance_ptr                appearance;
    const std::vector<uint8_t>          slots;
    const std::string                   message;
    const fb::game::dialog::interaction interaction;
    const uint32_t                      oid;
#else
    uint16_t    look;
    uint8_t     color;
    std::string message;
    uint8_t     interaction;
    uint32_t    oid;
#endif

public:
#ifndef BOT
    dialog_input(const fb::model::object&      obj,
                 std::string_view              message,
                 uint32_t                      oid         = 0xFFFFFFFD,
                 fb::game::dialog::interaction interaction = fb::game::dialog::interaction::INPUT);

    dialog_input(const fb::game::object&       object,
                 std::string_view              message,
                 uint32_t                      oid         = 0xFFFFFFFD,
                 fb::game::dialog::interaction interaction = fb::game::dialog::interaction::INPUT);
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