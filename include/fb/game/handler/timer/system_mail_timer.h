#ifndef FB_GAME_HANDLER_TIMER_SYSTEM_MAIL_TIMER_H
#define FB_GAME_HANDLER_TIMER_SYSTEM_MAIL_TIMER_H

#include <fb/async_generator.h>
#include <fb/handler.h>
#include <fb/game/server.h>
#include <fb/game/system_mail.h>
#include <fb/model/datetime.h>
#include <tuple>
#include <vector>

namespace fb::game::handler::timer {

class system_mail_timer : public fb::handler::timer<fb::game::server>
{
private:
    static constexpr std::size_t chunk_limit = 100;

    fb::async_generator<void> _delivery;

private:
    [[nodiscard]] fb::async_generator<void>                delivery_coroutine();
    std::vector<std::tuple<uint32_t, fb::model::datetime>> collect_online_users() const;

    static bool is_expired(const fb::game::system_mail& mail, const fb::model::datetime& now);
    static void prune_expired_mails(std::vector<fb::game::system_mail>& mails, const fb::model::datetime& now);

public:
    system_mail_timer(fb::game::server& server);

    async::task<void> handle() override;
};

} // namespace fb::game::handler::timer

#endif // FB_GAME_HANDLER_TIMER_SYSTEM_MAIL_TIMER_H
