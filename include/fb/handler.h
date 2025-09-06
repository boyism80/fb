#ifndef FB_HANDLER_H
#define FB_HANDLER_H

#include <thread>
#include <async/task.h>
#include <fb/model/datetime.h>

namespace fb::handler {

template <typename ServerType, typename ProtocolType, uint32_t DurationMs = 1000, uint32_t Limit = 10>
class protocol
{
public:
    using server_type   = ServerType;
    using protocol_type = ProtocolType;
    using session_type  = server_type::session_type;

    // Rate limiting configuration as static constants
    static constexpr uint32_t duration_ms = DurationMs;
    static constexpr uint32_t limit       = Limit;

protected:
    server_type& server;

protected:
    protocol(server_type& server) :
        server(server)
    { }

    virtual async::task<bool> handle(session_type& session, protocol_type& protocol) = 0;
};

template <typename ServerType, typename MessageType>
class amqp
{
public:
    using server_type  = ServerType;
    using message_type = MessageType;

protected:
    server_type& server;

protected:
    amqp(server_type& server) :
        server(server)
    { }

    virtual async::task<void> handle(const message_type& message) = 0;
};

template <typename ServerType>
class timer
{
public:
    using server_type = ServerType;

protected:
    server_type& server;

protected:
    timer(server_type& server) :
        server(server)
    { }

public:
    // logic thread timer handler
    virtual async::task<void> handle(const fb::model::datetime& now, std::thread::id id)
    {
        co_return;
    }

    // global timer handler
    virtual async::task<void> handle()
    {
        co_return;
    }
};

} // namespace fb::handler

#endif // FB_HANDLER_H