#ifndef __BOARD_H__
#define __BOARD_H__

#include <ctime>
#include <stdint.h>
#include <optional>
#include <string>

namespace fb::game::board {

/**
 * @brief      This class describes a section.
 */
class section
{
public:
    const uint32_t               id;
    const std::string            title;
    const std::optional<uint8_t> min_level, max_level;
    const bool                   admin;

public:
    /**
     * @brief      Constructs a new instance.
     *
     * @param[in]  id         The identifier
     * @param[in]  title      The title
     * @param[in]  min_level  The minimum level
     * @param[in]  max_level  The maximum level
     * @param[in]  admin      The admin
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
     * @brief      { function_description }
     *
     * @param[in]  level  The level
     * @param[in]  admin  The admin
     *
     * @return     { description_of_the_return_value }
     */
    bool writable(uint8_t level, bool admin) const;
};

/**
 * @brief      This class describes an article.
 */
class article
{
public:
    const uint32_t    id;
    const uint32_t    section;
    const uint32_t    user;
    const uint8_t     month, day;
    const std::string uname;
    const std::string title;
    const std::string contents;

public:
    /**
     * @brief      Constructs a new instance.
     *
     * @param[in]  id        The identifier
     * @param[in]  section   The section
     * @param[in]  user      The user
     * @param[in]  uname     The uname
     * @param[in]  title     The title
     * @param[in]  month     The month
     * @param[in]  day       The day
     * @param[in]  contents  The contents
     */
    article(uint32_t           id,
            uint32_t           section,
            uint32_t           user,
            const std::string& uname,
            const std::string& title,
            uint8_t            month,
            uint8_t            day,
            const std::string& contents = "") :
        id(id),
        section(section),
        user(user),
        uname(uname),
        title(title),
        month(month),
        day(day),
        contents(contents)
    { }
    ~article() = default;
};

} // namespace fb::game::board

#endif // !__BOARD_H__
