#ifndef __BOARD_H__
#define __BOARD_H__

#include <string>
#include <vector>
#include <ctime>
#include <fb/game/session.h>

namespace fb { namespace game {

enum class board_button_enabled : uint8_t { NEXT = 0x01, WRITE = 0x02 };

class board_section
{
public:
    const uint32_t                  id;
    const std::string               title;

public:
    board_section(uint32_t id, const std::string& title) : id(id), title(title)
    {}
    ~board_section() = default;
};

class board_article
{
public:
    const uint32_t                  id;
    const uint32_t                  section;
    const uint32_t                  uid;
    const uint8_t                   month, day;
    const std::string               uname;
    const std::string               title;
    const std::string               contents;

public:
    board_article(uint32_t id, uint32_t section, uint32_t uid, const std::string& uname, const std::string& title, uint8_t month, uint8_t day, const std::string& contents = "") : 
        id(id), section(section), uid(uid), uname(uname), title(title), month(month), day(day), contents(contents)
    { }
    ~board_article() = default;
};

} }

#endif // !__BOARD_H__
