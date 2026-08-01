#ifndef __BOT_INTEGRATION_DIALOG_EXT_BOT_H__
#define __BOT_INTEGRATION_DIALOG_EXT_BOT_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>
#include <fb/game/dialog_type.h>
#include <vector>
#include <string>

namespace fb::bot::integration {

// Wire values match fb::game::dialog::list_type (opcode 0x30).
enum class dialog_ext_type : uint8_t
{
    normal                = static_cast<uint8_t>(fb::game::dialog::list_type::TEXT),
    normal_no_msg         = static_cast<uint8_t>(fb::game::dialog::list_type::TEXT_NO_MSG),
    list                  = static_cast<uint8_t>(fb::game::dialog::list_type::LIST),
    list_no_msg           = static_cast<uint8_t>(fb::game::dialog::list_type::LIST_NO_MSG),
    input_ext             = static_cast<uint8_t>(fb::game::dialog::list_type::INPUT),
    input_ext_no_msg      = static_cast<uint8_t>(fb::game::dialog::list_type::INPUT_NO_MSG),
    input_password        = static_cast<uint8_t>(fb::game::dialog::list_type::INPUT_PASSWORD),
    input_password_no_msg = static_cast<uint8_t>(fb::game::dialog::list_type::INPUT_PASSWORD_NO_MSG),
    email                 = static_cast<uint8_t>(fb::game::dialog::list_type::EMAIL),
    look                  = static_cast<uint8_t>(fb::game::dialog::list_type::LOOK),
};

class dialog_ext_bot : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x30;

public:
    dialog_ext_type type = dialog_ext_type::normal;

    uint16_t    look  = 0;
    uint8_t     color = 0;
    std::string message;
    uint32_t    oid         = 0;
    uint8_t     interaction = 0;

    bool normal_button_prev = false;
    bool normal_button_next = false;

    std::vector<std::string> list_lists;
    bool                     list_button_prev = false;

    std::string input_ext_top;
    std::string input_ext_bottom;
    uint8_t     input_ext_maxlen      = 0;
    bool        input_ext_button_prev = false;
    bool        input_ext_password    = false;

    // EMAIL reuses top/bottom slots as str1/str2.
    std::string email_str1;
    std::string email_str2;

public:
    dialog_ext_bot() = default;

public:
    void deserialize(fb::stream_reader<big_endian>& reader);
};

} // namespace fb::bot::integration

#endif
