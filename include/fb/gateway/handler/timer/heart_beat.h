#ifndef FB_GATEWAY_HANDLER_TIMER_HEART_BEAT_H
#define FB_GATEWAY_HANDLER_TIMER_HEART_BEAT_H

#include <fb/handler.h>
#include <fb/gateway/server.h>

namespace fb::gateway::handler::timer {

class heart_beat : public fb::handler::timer<fb::gateway::server>
{
public:
    heart_beat(fb::gateway::server& server);

    async::task<void> handle() override;
};

} // namespace fb::gateway::handler::timer

#endif // FB_GATEWAY_HANDLER_TIMER_HEART_BEAT_H

