#ifndef __FB_GAME_SERVICE_SYSTEM_MAIL_H__
#define __FB_GAME_SERVICE_SYSTEM_MAIL_H__

#include <fb/async_generator.h>
#include <fb/game/system_mail.h>
#include <fb/model/datetime.h>

#include <cstddef>
#include <cstdint>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

namespace fb::protocol::internal {
class SystemMail;
}

namespace fb::game {
class server;
} // namespace fb::game

namespace fb::game::service {

class system_mail
{
public:
    fb::game::server& server;

private:
    static constexpr std::size_t chunk_limit = 100;

    std::vector<fb::game::system_mail> _pending_mails;
    std::mutex                         _joined_mutex;
    std::unordered_set<uint32_t>       _joined;
    fb::async_generator<void>          _sending;

private:
    // clang-format off
    static fb::game::system_mail from_system_mail_dto(const fb::protocol::internal::SystemMail& dto);
    static bool                  expired(const fb::game::system_mail& mail, const fb::model::datetime& now);
    static void                  prune_expired_mails(std::vector<fb::game::system_mail>& mails, const fb::model::datetime& now);
    fb::async_generator<void>    send_coroutine();
    // clang-format on

public:
    explicit system_mail(fb::game::server& server);

    // clang-format off
    async::task<std::optional<bool>> create(uint32_t world, uint32_t sender, std::string_view title, std::string_view contents, const std::optional<std::string>& expire_date);
    async::task<void>                send();
    void                             join(uint32_t user);
    // clang-format on
};

} // namespace fb::game::service

#endif
