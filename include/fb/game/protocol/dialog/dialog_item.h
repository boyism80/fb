#ifndef __PROTOCOL_GAME_ITEM_H__
#define __PROTOCOL_GAME_ITEM_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>
#ifndef BOT
#include <fb/game/dialog.h>
#endif

using namespace fb::model::enum_value;

namespace fb::protocol::game::response {

class dialog_item : public fb::protocol::header
{
public:
    inline static uint8_t header = 0x2F;

public:
#ifndef BOT
    const fb::model::object&            obj;
    const fb::game::dialog::item_pairs& items;
    const std::string                   message;
    const uint16_t                      pursuit;
    const fb::game::dialog::interaction interaction;
    const uint32_t                      sequence;
#else

#endif

public:
#ifndef BOT
    dialog_item(const fb::model::object&            obj,
                const fb::game::dialog::item_pairs& items,
                const std::string&                  message,
                uint32_t                            sequence    = 0xFFFFFFFD,
                uint16_t                            pursuit     = 0xFFFF,
                fb::game::dialog::interaction       interaction = fb::game::dialog::interaction::ITEM);
#else
    dialog_item() = default;
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