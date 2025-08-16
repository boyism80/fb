#ifndef FB_LOGIN_HANDLER_TIMER_HEART_BEAT_H
#define FB_LOGIN_HANDLER_TIMER_HEART_BEAT_H

#include <fb/handler.h>
#include <fb/login/server.h>

namespace fb::login::handler::timer {

class heart_beat : public fb::handler::timer<fb::login::server>
{
public:
    heart_beat(fb::login::server& server);

    async::task<void> handle() override;
};

} // namespace fb::login::handler::timer

#endif // FB_LOGIN_HANDLER_TIMER_HEART_BEAT_H
