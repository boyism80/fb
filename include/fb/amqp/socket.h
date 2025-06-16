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

/**
 * @brief      AMQP socket wrapper for RabbitMQ communication.
 *
 *             This class provides a high-level interface for AMQP (Advanced Message
 *             Queuing Protocol) communication using RabbitMQ. It manages connection
 *             lifecycle, queue declarations, and message operations. Used for
 *             inter-service communication between different server components.
 */
class socket
{
private:
    amqp_socket_t*                      _socket = nullptr; ///< Low-level AMQP socket handle
    amqp_connection_state_t             _conn   = nullptr; ///< AMQP connection state
    std::vector<std::unique_ptr<queue>> _queues;           ///< Collection of declared queues

public:
    /**
     * @brief      Constructs a new AMQP socket.
     */
    socket();

    /**
     * @brief      Destroys the AMQP socket and cleans up connections.
     */
    ~socket();

public:
    /**
     * @brief      Connects to an AMQP broker (RabbitMQ server).
     *
     * @param[in]  hostname  The hostname or IP address of the AMQP broker.
     * @param[in]  port      The port number of the AMQP broker.
     * @param[in]  id        The username for authentication.
     * @param[in]  pw        The password for authentication.
     * @param[in]  vhost     The virtual host to connect to.
     *
     * @return     True if connection was successful, false otherwise.
     */
    bool connect(const std::string& hostname,
                 uint16_t           port,
                 const std::string& id,
                 const std::string& pw,
                 const std::string& vhost);

    /**
     * @brief      Declares a new message queue.
     *
     * @return     A reference to the newly declared queue.
     */
    queue& declare_queue();

    /**
     * @brief      Waits for incoming messages with optional timeout.
     *
     * @param[in]  timeout  The timeout for waiting (nullptr for blocking).
     *
     * @return     True if messages are available, false on timeout.
     */
    bool select(const timeval* timeout = nullptr);

public:
    /**
     * @brief      Conversion operator to get the underlying AMQP connection.
     *
     * @return     The AMQP connection state for low-level operations.
     */
    operator amqp_connection_state_t ();
};

} // namespace fb::amqp

#endif