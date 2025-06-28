#ifndef __BOT_BULLETIN_H__
#define __BOT_BULLETIN_H__

#include <stdint.h>
#include <string>

namespace fb::bot {

/**
 * @brief      Represents a game bulletin board section.
 *
 *             This class encapsulates information about a bulletin section
 *             in the game, including its unique identifier and display title.
 *             Bulletins are used for player communication, announcements,
 *             and information sharing within the game world.
 */
class bulletin
{
public:
    const uint16_t    id = 0; ///< Unique identifier for this bulletin section
    const std::string title;  ///< Display title of the bulletin section

public:
    /**
     * @brief      Default constructor for an empty bulletin.
     */
    bulletin() = default;

    /**
     * @brief      Constructs a bulletin with specified ID and title.
     *
     * @param[in]  id     The unique identifier for this bulletin.
     * @param[in]  title  The display title for this bulletin.
     */
    bulletin(uint16_t id, const std::string& title) :
        id(id),
        title(title)
    { }

    /**
     * @brief      Copy constructor for duplicating an existing bulletin.
     *
     * @param[in]  r  The source bulletin to copy from.
     */
    bulletin(const bulletin& r) :
        id(r.id),
        title(r.title)
    { }

    /**
     * @brief      Default destructor.
     */
    ~bulletin() = default;
};

/**
 * @brief      Represents an article or post within a bulletin.
 *
 *             This class will contain information about individual
 *             posts or articles within a bulletin section. Currently
 *             serves as a placeholder for future implementation.
 */
class article
{ };

} // namespace fb::bot

#endif