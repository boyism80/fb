#ifndef FB_LOGIN_HANDLER_TIMER_NAME_KEEP_ALIVE_H
#define FB_LOGIN_HANDLER_TIMER_NAME_KEEP_ALIVE_H

#include <fb/handler.h>
#include <fb/login/server.h>

namespace fb::login::handler::timer {

// Refreshes name reservations of live sessions so they only expire after the session or the server is gone.
class name_keep_alive : public fb::handler::timer<fb::login::server>
{
public:
    name_keep_alive(fb::login::server& server);

    async::task<void> handle() override;
};

} // namespace fb::login::handler::timer

#endif // FB_LOGIN_HANDLER_TIMER_NAME_KEEP_ALIVE_H
