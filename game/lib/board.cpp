#include <fb/game/board.h>

bool fb::game::board::section::writable(uint8_t level, bool admin) const
{
    if(admin)
        return true;

    if(this->admin)
        return false;

    if(this->min_level != std::nullopt && level < this->min_level.value())
        return false;

    if(this->max_level != std::nullopt && level > this->max_level.value())
        return false;

    return true;
}