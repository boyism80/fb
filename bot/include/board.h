#ifndef __BOT_BOARD_H__
#define __BOT_BOARD_H__

#include <stdint.h>
#include <string>

namespace fb { namespace bot {

class board
{
public:
    const uint16_t      id = 0;
    const std::string   title;

public:
    board() = default;
    board(uint16_t id, const std::string& title) : id(id), title(title)
    { }
    board(const board& r) : id(r.id), title(r.title)
    { }
    ~board() = default;
};

class article
{};

} }

#endif