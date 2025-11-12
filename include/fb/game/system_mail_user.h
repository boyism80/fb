#ifndef __FB_GAME_SYSTEM_MAIL_USER_H__
#define __FB_GAME_SYSTEM_MAIL_USER_H__

#include <cstdint>
#include <fb/model/datetime.h>

namespace fb::game {

struct system_mail_user
{
    uint32_t                           mail_id = 0;
    bool                               read    = false;
    std::optional<fb::model::datetime> expire_date;

public:
    system_mail_user() = default;
    system_mail_user(uint32_t mail_id, bool read, const std::optional<std::string>& expire_date);
};

} // namespace fb::game

#endif // __FB_GAME_SYSTEM_MAIL_USER_H__
