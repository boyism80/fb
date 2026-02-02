#ifndef __BOT_BULLETIN_H__
#define __BOT_BULLETIN_H__

#include <stdint.h>
#include <string>
#include <string_view>

namespace fb::bot {

class bulletin
{
public:
    const uint16_t    id = 0;
    const std::string title;

public:
    bulletin() = default;
    bulletin(uint16_t id, std::string_view title) :
        id(id),
        title(std::string(title))
    { }

    bulletin(const bulletin& r) :
        id(r.id),
        title(r.title)
    { }

    ~bulletin() = default;
};

class article
{ };

} // namespace fb::bot

#endif