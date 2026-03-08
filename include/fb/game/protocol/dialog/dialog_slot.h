#ifndef __PROTOCOL_GAME_SLOT_H__
#define __PROTOCOL_GAME_SLOT_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>
#ifndef BOT
#include <fb/game/dialog.h>
#endif
#include <string_view>

namespace fb::protocol::game::response {

using namespace fb::model::enum_value;

class dialog_slot : public fb::protocol::header
{
public:
    static constexpr uint8_t header = 0x2F;
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
    uint16_t             look;
    uint8_t              color;
    std::vector<uint8_t> slots;
    std::string          message;
    uint8_t              interaction;
    uint32_t             oid;
#endif

public:
#ifndef BOT
    dialog_slot(const fb::model::object&      obj,
                const std::vector<uint8_t>&   slots,
                std::string_view              message,
                uint32_t                      oid         = 0xFFFFFFFD,
                fb::game::dialog::interaction interaction = fb::game::dialog::interaction::SLOT);
    dialog_slot(const fb::game::object&       object,
                const std::vector<uint8_t>&   slots,
                std::string_view              message,
                uint32_t                      oid         = 0xFFFFFFFD,
                fb::game::dialog::interaction interaction = fb::game::dialog::interaction::SLOT);
#else
    dialog_slot() = default;
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