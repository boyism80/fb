#ifndef __AMQP_SOCKET_H__
#define __AMQP_SOCKET_H__

#ifdef _WIN32
#include <WinSock2.h>
#else
#include <sys/time.h>
#endif
#include <stdexcept>
#include <string>
#include <vector>
#include <memory>
#include <rabbitmq-c/amqp.h>
#include <rabbitmq-c/tcp_socket.h>
#include <async/awaitable_then.h>

namespace fb::amqp {
class queue;

class socket
{
private:
    amqp_socket_t*                      _socket = nullptr; ///< Low-level AMQP socket handle
    amqp_connection_state_t             _conn   = nullptr; ///< AMQP connection state
    std::vector<std::unique_ptr<queue>> _queues;           ///< Collection of declared queues

public:
    socket();

    ~socket();

public:
    bool connect(const std::string& hostname, uint16_t port, const std::string& id, const std::string& pw, const std::string& vhost);

    queue& declare_queue();
    queue& declare_queue(const std::string& queue_name, bool durable = true, bool exclusive = false, bool auto_delete = false, bool quorum = false);

    bool select(const timeval* timeout = nullptr);

    /// <summary>
    /// Publishes a message to the specified exchange and routing key.
    /// </summary>
    /// <param name="exchange">Exchange name to publish to (empty string for default exchange).</param>
    /// <param name="routing_key">Routing key for message routing (typically the queue name for default exchange).</param>
    /// <param name="message">Message body to publish.</param>
    /// <param name="properties">Optional AMQP properties (nullptr for default).</param>
    /// <returns>True if publish was successful, false otherwise.</returns>
    bool publish(const std::string& exchange, const std::string& routing_key, const std::vector<uint8_t>& message, const amqp_basic_properties_t* properties = nullptr);

public:
    operator amqp_connection_state_t ();
};

} // namespace fb::amqp

#endif