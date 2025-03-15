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
    inline static uint8_t header = 0x2F;

public:
#ifndef BOT
    const fb::model::object&            obj;
    const std::vector<uint8_t>          slots;
    const std::string                   message;
    const fb::game::dialog::interaction interaction;
#else

#endif

public:
#ifndef BOT
    dialog_input(const fb::model::object&      obj,
                 const std::string&            message,
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