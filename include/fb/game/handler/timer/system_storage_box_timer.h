#ifndef FB_GAME_HANDLER_TIMER_SYSTEM_STORAGE_BOX_TIMER_H
#define FB_GAME_HANDLER_TIMER_SYSTEM_STORAGE_BOX_TIMER_H

#include <fb/async_generator.h>
#include <fb/handler.h>
#include <fb/game/server.h>
#include <fb/game/system_storage_box.h>
#include <fb/model/datetime.h>
#include <tuple>
#include <vector>

namespace fb::game::handler::timer {

class system_storage_box_timer : public fb::handler::timer<fb::game::server>
{
private:
    fb::async_generator<void> _delivery;

private:
    [[nodiscard]] fb::async_generator<void>                delivery_coroutine();
    std::vector<std::tuple<uint32_t, fb::model::datetime>> collect_online_users() const;

    static bool is_expired(const fb::game::system_storage_box& box, const fb::model::datetime& now);
    static void prune_expired_boxes(std::vector<fb::game::system_storage_box>& boxes, const fb::model::datetime& now);

public:
    system_storage_box_timer(fb::game::server& server);

    async::task<void> handle() override;
};

} // namespace fb::game::handler::timer

#endif // FB_GAME_HANDLER_TIMER_SYSTEM_STORAGE_BOX_TIMER_H
