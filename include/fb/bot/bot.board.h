#ifndef __BOT_BOARD_H__
#define __BOT_BOARD_H__

#include <stdint.h>
#include <string>

namespace fb::bot {

/**
 * @brief      Represents a game board or bulletin board section.
 *
 *             This class encapsulates information about a board section
 *             in the game, including its unique identifier and display title.
 *             Boards are used for player communication, announcements,
 *             and information sharing within the game world.
 */
class board
{
public:
    const uint16_t    id = 0; ///< Unique identifier for this board section
    const std::string title;  ///< Display title of the board section

public:
    /**
     * @brief      Default constructor for an empty board.
     */
    board() = default;

    /**
     * @brief      Constructs a board with specified ID and title.
     *
     * @param[in]  id     The unique identifier for this board.
     * @param[in]  title  The display title for this board.
     */
    board(uint16_t id, const std::string& title) :
        id(id),
        title(title)
    { }

    /**
     * @brief      Copy constructor for creating a board from another board.
     *
     * @param[in]  r  The board to copy from.
     */
    board(const board& r) :
        id(r.id),
        title(r.title)
    { }

    /**
     * @brief      Default destructor.
     */
    ~board() = default;
};

/**
 * @brief      Represents an article or post within a board.
 *
 *             This class will contain information about individual
 *             posts or articles within a board section. Currently
 *             serves as a placeholder for future implementation.
 */
class article
{ };

} // namespace fb::bot

#endif