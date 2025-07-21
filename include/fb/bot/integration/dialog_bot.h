#ifndef __BOT_INTEGRATION_DIALOG_BOT_H__
#define __BOT_INTEGRATION_DIALOG_BOT_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>
#include <vector>
#include <string>

namespace fb::bot::integration {

enum class dialog_type : uint8_t
{
    menu   = 0x01, // MENU
    input  = 0x03, // INPUT
    item   = 0x04, // ITEM
    slot   = 0x05  // SLOT
};

struct dialog_item_data
{
    uint16_t    look;
    uint8_t     color;
    uint32_t    value;
    std::string name;
    std::string desc;
};

class dialog_bot : public fb::protocol::header
{
public:
    static constexpr uint8_t header = 0x2F;

public:
    dialog_type type;

    // Common fields
    uint16_t    look;
    uint8_t     color;
    std::string message;
    uint32_t    oid;
    uint8_t     interaction;

    // Input dialog specific
    std::vector<uint8_t> input_slots;

    // Item dialog specific
    std::vector<dialog_item_data> item_items;
    uint16_t                      item_pursuit;

    // Menu dialog specific
    std::vector<std::string> menu_menus;

    // Slot dialog specific
    std::vector<uint8_t> slot_slots;

public:
    dialog_bot() = default;

public:
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader);
};

} // namespace fb::bot::integration

#endif