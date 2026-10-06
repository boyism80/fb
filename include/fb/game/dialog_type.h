#ifndef __FB_GAME_DIALOG_TYPE_H__
#define __FB_GAME_DIALOG_TYPE_H__

#include <cstdint>

namespace fb::game::dialog {

enum class type : uint8_t
{
    MENU_NO_EXT  = 0, // menu without ext blob
    MENU         = 1, // menu with ext
    INPUT_NO_EXT = 2, // input without ext blob
    INPUT        = 3, // input with ext
    ITEM         = 4,
    SLOT         = 5,
    PURSUIT      = 6,
    SPELL        = 8,
    BUY          = 10,
};

enum class list_type : uint8_t
{
    TEXT                  = 0, // dialog with message
    TEXT_NO_MSG           = 1, // dialog without message
    LIST                  = 2, // list with message
    LIST_NO_MSG           = 3, // list without message
    INPUT                 = 4, // plaintext input with message
    INPUT_NO_MSG          = 5, // plaintext input without message
    INPUT_PASSWORD        = 7, // password input with message
    INPUT_PASSWORD_NO_MSG = 8, // password input without message
    EMAIL                 = 9,
    LOOK                  = 10, // look picker (DLGMSGH.EPF)
};

// The reply a parked dialog accepts. Grouped by how the reply is handled, so a client cannot
// answer a name-matched dialog with a raw index (or the reverse).
enum class response : uint8_t
{
    MENU,        // 0x39 MENU / MENU_NO_EXT, index checked against the choice count
    INPUT,       // 0x39 INPUT / INPUT_NO_EXT
    SLOT,        // 0x39 SLOT / SPELL
    SELECT,      // 0x39 ITEM / PURSUIT / BUY, matched by name
    TEXT,        // 0x3A TEXT / TEXT_NO_MSG
    LIST,        // 0x3A LIST / LIST_NO_MSG, index checked against the choice count
    LIST_INPUT,  // 0x3A INPUT* / EMAIL
    POPUP_INPUT, // popup input submit
};

} // namespace fb::game::dialog

#endif
