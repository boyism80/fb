#include <fb/game/system_mail_user.h>

using namespace fb::game;

system_mail_user::system_mail_user(uint32_t mail_id, bool read, const std::optional<std::string>& expire_date) :
    mail_id(mail_id),
    read(read),
    expire_date(expire_date.has_value() ? std::make_optional<fb::model::datetime>(expire_date.value()) : std::nullopt)
{ }
