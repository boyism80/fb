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
#include <fb/thread_container.h>

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
    bool
    connect(std::string_view hostname, uint16_t port, std::string_view id, std::string_view pw, std::string_view vhost);
    queue& declare_queue(bool durable, bool exclusive, bool auto_delete, bool quorum, fb::thread_container& threads);
    bool   select(const timeval* timeout = nullptr);
    bool   publish(std::string_view               exchange,
                   std::string_view               routing_key,
                   const std::vector<uint8_t>&    message,
                   const amqp_basic_properties_t* properties = nullptr);

public:
    operator amqp_connection_state_t ();
};

} // namespace fb::amqp

#endif