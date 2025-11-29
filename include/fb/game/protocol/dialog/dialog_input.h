#ifndef __PROTOCOL_GAME_INPUT_H__
#define __PROTOCOL_GAME_INPUT_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>
#ifndef BOT
#include <fb/game/dialog.h>
#endif

using namespace fb::model::enum_value;

namespace fb::protocol::game::response {

class dialog_input : public fb::protocol::header
{
public:
    static constexpr uint8_t header = 0x2F;
#ifndef BOT
    using portrait_ptr = std::unique_ptr<fb::game::portrait>;
#endif

public:
#ifndef BOT
    const portrait_ptr                  portrait;
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
                 const std::string&            message,
                 uint32_t                      oid         = 0xFFFFFFFD,
                 fb::game::dialog::interaction interaction = fb::game::dialog::interaction::INPUT);

    dialog_input(const fb::game::object&       object,
                 const std::string&            message,
                 uint32_t                      oid         = 0xFFFFFFFD,
                 fb::game::dialog::interaction interaction = fb::game::dialog::interaction::INPUT);
#else
    dialog_input() = default;
#endif

public:
#ifndef BOT
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const;
#else
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

} // namespace fb::protocol::game::response

#endif