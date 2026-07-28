#ifndef __FB_GAME_DIALOG_TYPE_H__
#define __FB_GAME_DIALOG_TYPE_H__

#include <cstdint>

namespace fb::game::dialog {

enum class type : uint8_t
{
    MENU  = 1,
    INPUT = 3,
    ITEM  = 4,
    SLOT  = 5,
};

enum class list_type : uint8_t
{
    TEXT  = 0,
    LIST  = 2,
    INPUT = 4,
};

} // namespace fb::game::dialog

#endif
