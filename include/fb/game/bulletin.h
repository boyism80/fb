#ifndef __BULLETIN_H__
#define __BULLETIN_H__

#include <ctime>
#include <stdint.h>
#include <optional>
#include <string>

namespace fb::game::bulletin {

class section
{
public:
    const uint32_t               id;
    const std::string            title;
    const std::optional<uint8_t> min_level;
    const std::optional<uint8_t> max_level;
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
    const bool        next;

public:
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

} // namespace fb::game::bulletin

#endif // !__BULLETIN_H__
