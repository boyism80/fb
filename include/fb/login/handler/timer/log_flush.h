#ifndef __FB_LOGIN_HANDLER_TIMER_LOG_FLUSH_H__
#define __FB_LOGIN_HANDLER_TIMER_LOG_FLUSH_H__

#include <fb/handler.h>
#include <fb/login/server.h>

namespace fb::login::handler::timer {

class log_flush : public fb::handler::timer<fb::login::server>
{
public:
    log_flush(fb::login::server& server);

    async::task<void> handle() override;
};

} // namespace fb::login::handler::timer

#endif // !__FB_LOGIN_HANDLER_TIMER_LOG_FLUSH_H__

