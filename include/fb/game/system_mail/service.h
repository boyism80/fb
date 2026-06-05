#ifndef __SYSTEM_MAIL_SERVICE_H__
#define __SYSTEM_MAIL_SERVICE_H__

#include <fb/async_generator.h>
#include <fb/game/system_mail.h>
#include <fb/model/datetime.h>
#include <optional>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>
#include <cstdint>

namespace fb::protocol::internal {
class SystemMail;
}

namespace fb::game {

class server;

class system_mail_service
{
public:
    fb::game::server& server;

private:
    static constexpr std::size_t chunk_limit = 100;

    uint32_t                  _poll_offset = 0;
    std::vector<system_mail>  _pending_mails;
    fb::async_generator<void> _delivery;

    static system_mail from_system_mail_dto(const fb::protocol::internal::SystemMail& dto);
    static bool        expired(const system_mail& mail, const fb::model::datetime& now);
    static void        prune_expired_mails(std::vector<system_mail>& mails, const fb::model::datetime& now);

    fb::async_generator<void> delivery_coroutine();

public:
    explicit system_mail_service(fb::game::server& server);

    async::task<bool> create(uint32_t                          sender,
                             std::string_view                  title,
                             std::string_view                  contents,
                             const std::optional<std::string>& expire_date);
    async::task<void> poll_and_deliver();
};

} // namespace fb::game

#endif
