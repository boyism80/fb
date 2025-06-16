#ifndef __BOARD_H__
#define __BOARD_H__

#include <ctime>
#include <stdint.h>
#include <optional>
#include <string>

namespace fb::game::board {

/**
 * @brief      Represents a bulletin board section with access control.
 *
 *             This class defines a section of a bulletin board system, containing
 *             configuration for access permissions, level restrictions, and administrative
 *             controls. Each section can have different posting requirements and visibility
 *             settings based on player level and administrative status.
 *
 *             Key features:
 *             - Level-based access control for reading and writing
 *             - Administrative section support
 *             - Configurable minimum and maximum level requirements
 *             - Section identification and title management
 */
class section
{
public:
    const uint32_t               id;        ///< Unique section identifier
    const std::string            title;     ///< Display title of the section
    const std::optional<uint8_t> min_level; ///< Minimum level required to access
    const std::optional<uint8_t> max_level; ///< Maximum level allowed to access
    const bool                   admin;     ///< Whether this is an admin-only section

public:
    /**
     * @brief      Constructs a new board section with specified parameters.
     *
     *             Creates a bulletin board section with the given configuration,
     *             including access restrictions and administrative settings.
     *
     * @param[in]  id         The unique identifier for this section.
     * @param[in]  title      The display title of the section.
     * @param[in]  min_level  The minimum level required to access (optional).
     * @param[in]  max_level  The maximum level allowed to access (optional).
     * @param[in]  admin      Whether this section requires admin privileges.
     */
    section(uint32_t                      id,
            const std::string&            title,
            const std::optional<uint8_t>& min_level = std::nullopt,
            const std::optional<uint8_t>& max_level = std::nullopt,
            bool                          admin     = false) :
        id(id),
        title(title),
        min_level(min_level),
        max_level(max_level),
        admin(admin)
    { }
    ~section() = default;

public:
    /**
     * @brief      Checks if a user can write to this section.
     *
     *             Determines whether a user with the specified level and admin status
     *             has permission to post new articles in this section based on the
     *             section's access control settings.
     *
     * @param[in]  level  The user's character level.
     * @param[in]  admin  Whether the user has administrative privileges.
     *
     * @return     True if the user can write to this section, false otherwise.
     */
    bool writable(uint8_t level, bool admin) const;
};

/**
 * @brief      Represents a bulletin board article or post.
 *
 *             This class contains all the information for a single article posted
 *             to a bulletin board section. It includes metadata such as author
 *             information, posting date, and the article content. Articles are
 *             organized within sections and can be paginated for display.
 *
 *             Key features:
 *             - Complete article metadata and content storage
 *             - Author identification and display name
 *             - Date tracking for chronological organization
 *             - Section association for proper categorization
 *             - Pagination support for large article lists
 */
class article
{
public:
    const uint32_t    id;         ///< Unique article identifier
    const uint32_t    section;    ///< Section ID this article belongs to
    const uint32_t    user;       ///< User ID of the article author
    const uint8_t     month, day; ///< Publication date (month and day)
    const std::string uname;      ///< Display name of the article author
    const std::string title;      ///< Article title
    const std::string contents;   ///< Article content text
    const bool        next;       ///< Whether there are more articles in pagination

public:
    /**
     * @brief      Constructs a new bulletin board article.
     *
     *             Creates an article with the specified metadata and content,
     *             ready for display in the bulletin board system.
     *
     * @param[in]  id        The unique identifier for this article.
     * @param[in]  section   The section ID this article belongs to.
     * @param[in]  user      The user ID of the article author.
     * @param[in]  uname     The display name of the article author.
     * @param[in]  title     The title of the article.
     * @param[in]  month     The publication month.
     * @param[in]  day       The publication day.
     * @param[in]  contents  The article content text (optional).
     * @param[in]  next      Whether there are more articles for pagination.
     */
    article(uint32_t           id,
            uint32_t           section,
            uint32_t           user,
            const std::string& uname,
            const std::string& title,
            uint8_t            month,
            uint8_t            day,
            const std::string& contents = "",
            bool               next     = false) :
        id(id),
        section(section),
        user(user),
        uname(uname),
        title(title),
        month(month),
        day(day),
        contents(contents),
        next(next)
    { }
    ~article() = default;
};

} // namespace fb::game::board

#endif // !__BOARD_H__
