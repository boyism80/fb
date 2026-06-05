#ifndef FB_GAME_SYSTEM_MAIL_H
#define FB_GAME_SYSTEM_MAIL_H

#include <cstdint>
#include <string>
#include <optional>
#include <fb/model/datetime.h>

namespace fb::game {

struct system_mail
{
    uint32_t                           id;
    uint32_t                           sender;
    std::string                        title;
    std::string                        contents;
    std::optional<fb::model::datetime> expire_date;
    fb::model::datetime                created_date;
};

} // namespace fb::game

#endif // FB_GAME_SYSTEM_MAIL_H
