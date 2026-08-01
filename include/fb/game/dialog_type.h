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
    DUAL_FIELD   = 10,
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

} // namespace fb::game::dialog

#endif
