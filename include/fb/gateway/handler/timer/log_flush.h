#ifndef __FB_GATEWAY_HANDLER_TIMER_LOG_FLUSH_H__
#define __FB_GATEWAY_HANDLER_TIMER_LOG_FLUSH_H__

#include <fb/handler.h>
#include <fb/gateway/server.h>

namespace fb::gateway::handler::timer {

class log_flush : public fb::handler::timer<fb::gateway::server>
{
public:
    log_flush(fb::gateway::server& server);

    async::task<void> handle() override;
};

} // namespace fb::gateway::handler::timer

#endif // !__FB_GATEWAY_HANDLER_TIMER_LOG_FLUSH_H__

