#ifndef __BOT_INTEGRATION_DIALOG_EXT_BOT_H__
#define __BOT_INTEGRATION_DIALOG_EXT_BOT_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>
#include <vector>
#include <string>

namespace fb::bot::integration {

enum class dialog_ext_type : uint8_t
{
    normal    = 0x00,
    list      = 0x02,
    input_ext = 0x04
};

class dialog_ext_bot : public fb::protocol::header
{
public:
    static constexpr uint8_t header = 0x30;

public:
    dialog_ext_type type;

    // Common fields
    uint16_t    look;
    uint8_t     color;
    std::string message;
    uint32_t    oid;
    uint8_t     interaction;

    // Normal dialog specific
    bool normal_button_prev;
    bool normal_button_next;

    // List dialog specific
    std::vector<std::string> list_lists;
    bool                     list_button_prev;

    // Input ext dialog specific
    std::vector<uint8_t> input_ext_slots;
    std::string          input_ext_top;
    std::string          input_ext_bottom;
    uint8_t              input_ext_maxlen;
    bool                 input_ext_button_prev;

public:
    dialog_ext_bot() = default;

public:
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader);
};

} // namespace fb::bot::integration

#endif