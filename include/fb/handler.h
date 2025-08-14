#ifndef FB_HANDLER_H
#define FB_HANDLER_H

#include <async/task.h>

namespace fb {

/**
 * @brief   Base template class for protocol packet handlers.
 *
 *          This template provides the foundation for handling client protocol packets.
 *          Each handler processes a specific packet type and returns success/failure.
 *
 * @tparam  ServerType    The server class type (e.g., fb::game::server)
 * @tparam  ProtocolType  The protocol packet type (e.g., fb::protocol::game::request::login)
 */
template <typename ServerType, typename ProtocolType>
class handler
{
public:
    using server_type   = ServerType;
    using protocol_type = ProtocolType;
    using session_type  = server_type::session_type;

protected:
    server_type& server;

protected:
    handler(server_type& server) :
        server(server)
    { }

    virtual async::task<bool> handle(session_type& session, protocol_type& protocol) = 0;
};

/**
 * @brief   Base template class for AMQP message handlers.
 *
 *          This template provides the foundation for handling inter-service AMQP messages.
 *          Each handler processes a specific message type for server-to-server communication.
 *
 * @tparam  ServerType   The server class type (e.g., fb::game::server)
 * @tparam  MessageType  The AMQP message type (e.g., internal_resp::KickOut)
 */
template <typename ServerType, typename MessageType>
class amqp_handler
{
public:
    using server_type  = ServerType;
    using message_type = MessageType;

protected:
    server_type& server;

protected:
    amqp_handler(server_type& server) :
        server(server)
    { }

    virtual async::task<void> handle(const message_type& message) = 0;
};

} // namespace fb

#endif // FB_HANDLER_H