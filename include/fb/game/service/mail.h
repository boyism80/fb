#ifndef __FB_GAME_SERVICE_MAIL_H__
#define __FB_GAME_SERVICE_MAIL_H__

#include <fb/game/mail_box.h>
#include <string>
#include <string_view>
#include <vector>
#include <cstdint>

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

    async::task<void> send(character& sender, std::string_view to, std::string_view title, std::string_view contents);
    async::task<std::vector<mail_box::summary>> list(const character& ch, uint16_t offset, uint16_t count);
    async::task<mail_box::mail>                 read(character& ch, uint16_t id);
    async::task<void>                           remove(character& ch, uint16_t id);

    async::task<void> on_received(uint32_t user_id, uint16_t unread, const mail_box::summary& snapshot);
    async::task<void> on_received_batch(const std::vector<mail_box::summary>& snapshots,
                                        const std::vector<uint32_t>&          user_ids,
                                        const std::vector<uint16_t>&          unread_counts);

    void on_error(uint32_t error) const;

private:
    static mail_box::summary to_summary(const fb::protocol::internal::MailSummary& mail);
    static mail_box::summary to_summary(const fb::protocol::internal::Mail& mail);
    static mail_box::mail    to_mail(const fb::protocol::internal::Mail& mail);
    void                     apply_received(character& ch, uint16_t unread, const mail_box::summary& snapshot);
};

} // namespace fb::game::service

#endif
