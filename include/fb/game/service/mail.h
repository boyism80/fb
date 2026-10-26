#ifndef __FB_GAME_SERVICE_MAIL_H__
#define __FB_GAME_SERVICE_MAIL_H__

#include <fb/game/mail_box.h>

#include <cstdint>
#include <string_view>
#include <vector>

namespace fb::protocol::internal {
class MailSummary;
class Mail;
} // namespace fb::protocol::internal

namespace fb::game {
class server;
class character;
} // namespace fb::game

namespace fb::game::service {

class mail
{
public:
    fb::game::server& server;

public:
    explicit mail(fb::game::server& server);

    async::task<void> on_received(const mail_box::received& entry);

    void on_error(uint32_t error) const;

public:
    // clang-format off
    static mail_box::summary to_summary(const fb::protocol::internal::MailSummary& mail);
    static mail_box::summary to_summary(const fb::protocol::internal::Mail& mail);
    static mail_box::mail    to_mail(const fb::protocol::internal::Mail& mail);
    // clang-format on
};

} // namespace fb::game::service

#endif
