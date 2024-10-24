#ifndef __BOARD_H__
#define __BOARD_H__

#include <string>
#include <vector>
#include <ctime>
#include <character.h>

namespace fb { namespace game { namespace board {

class section
{
public:
    const uint32_t               id;
    const std::string            title;
    const std::optional<uint8_t> min_level, max_level;
    const bool                   admin;

public:
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
    bool writable(uint8_t level, bool admin) const;
};

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
    article(uint32_t id, uint32_t section, uint32_t user, const std::string& uname, const std::string& title, uint8_t month, uint8_t day, const std::string& contents = "") :
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

}}} // namespace fb::game::board

#endif // !__BOARD_H__
