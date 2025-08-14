#ifndef __FB_GATEWAY_H__
#define __FB_GATEWAY_H__

/**
 * @file    server.h
 * @brief   Gateway server for client connection routing and login server discovery
 * @author  FB Development Team
 *
 * @details This file implements the gateway server that serves as the initial
 *          entry point for game clients in the FB 2D MMORPG distributed architecture.
 *          The gateway provides login server discovery services, client version
 *          validation, and connection routing to appropriate login servers.
 *
 *          Key features:
 *          - Initial client connection point for distributed server architecture
 *          - Login server discovery and endpoint distribution to clients
 *          - Client version validation and compatibility checking
 *          - Cryptographic handshake and secure connection establishment
 *          - Load balancing support for multiple login servers
 *          - AMQP integration for inter-service communication and monitoring
 *          - Heartbeat system for service health monitoring
 *          - Configurable endpoint management with dynamic server list updates
 *          - Session management with proper connection lifecycle handling
 *          - Support for server capacity monitoring and status reporting
 *
 * @note    The gateway server is the first point of contact for clients and
 *          plays a crucial role in the distributed server architecture by
 *          directing clients to appropriate login servers based on capacity
 *          and availability.
 */

#include <fb/acceptor.h>
#include <fb/gateway/session.h>
#include <fb/gateway/util.h>

using namespace fb::protocol::internal;
namespace internal_resp = fb::protocol::internal::response;

namespace fb::gateway {

using namespace fb::protocol::gateway;

/**
 * @brief      Gateway server that provides login server list to clients.
 *
 *             This class serves as the initial entry point for game clients, providing
 *             a list of available login servers for client selection. It handles client
 *             version checking and presents login server options with their status and
 *             capacity information. Clients connect to the gateway first to get the
 *             login server list, then connect to their chosen login server.
 */
class server : public fb::acceptor<fb::gateway::session>
{
private:
    std::vector<endpoint> _entrypoints;
    fb::stream            _endpoint_bytes;
    uint32_t              _endpoint_crc = 0;
    fb::stream            _connection_cache;

public:
    /**
     * @brief      Constructs a new gateway server.
     *
     * @param      io_context   The boost::asio I/O context for network operations.
     * @param[in]  port         The port number to listen on for client connections.
     */
    server(boost::asio::io_context& io_context, uint16_t port);
    /**
     * @brief      Destroys the object.
     */
    ~server();

    /**
     * @brief      Gets the list of available login server endpoints.
     *
     * @return     A reference to the vector of endpoints.
     */
    const std::vector<endpoint>& entrypoints() const;

    /**
     * @brief      Gets the serialized endpoint bytes for client transmission.
     *
     * @return     A reference to the endpoint bytes stream.
     */
    const fb::stream& endpoint_bytes() const;

    /**
     * @brief      Gets the CRC checksum of the endpoint data.
     *
     * @return     The CRC checksum value.
     */
    uint32_t endpoint_crc() const;

private:
    /**
     * @brief      Loads available login server endpoints from configuration.
     *
     *             Reads the list of available login servers from configuration files
     *             and prepares them for client distribution. This includes login server
     *             addresses, ports, names, and current status information.
     *
     * @return     An async task that completes when login server endpoints are loaded.
     */
    [[nodiscard]] async::task<void> load_entries();

private:
    /**
     * @brief      Creates a stream containing cryptographic information for client handshake.
     *
     *             Generates a data stream containing encryption parameters that will be
     *             sent to clients during the connection establishment process. This
     *             includes encryption keys and algorithm information.
     *
     * @param[in]  encryption   The cryptographic object to serialize.
     *
     * @return     A stream containing the serialized cryptographic data.
     */
    fb::stream make_crt_stream(const fb::encryption& encryption);

    // override

protected:
    /**
     * @brief      Determines the decryption policy for incoming packets.
     *
     *             Specifies whether packets with the given command should be decrypted
     *             before processing. Gateway servers typically handle unencrypted
     *             handshake packets and encrypted game data differently.
     *
     * @param[in]  cmd   The command byte of the incoming packet.
     *
     * @return     True if the packet should be decrypted, false otherwise.
     */
    bool decrypt_policy(uint8_t cmd) const override final;

    /**
     * @brief      Declares AMQP queues required for gateway server communication.
     *
     *             Sets up message queues for inter-service communication, including
     *             queues for login server status updates, capacity monitoring,
     *             and administrative commands.
     *
     * @param      amqp  The AMQP socket to use for queue declaration.
     */
    void handle_init_amqp(fb::amqp::socket& amqp) override final;

    /**
     * @brief      Handles gateway server startup initialization.
     *
     *             Performs initialization tasks when the gateway server starts,
     *             including loading login server endpoints, setting up heartbeat timers,
     *             and preparing the server for client connections.
     *
     * @return     An async task that completes when startup is finished.
     */
    [[nodiscard]] async::task<void> handle_start() final;

    /**
     * @brief      Handles new client connection acceptance.
     *
     *             Called when a new client connects to the gateway server. Creates
     *             and initializes a new session object for the client connection
     *             and performs initial handshake procedures.
     *
     * @param      socket  The socket representing the new client connection.
     *
     * @return     A pointer to the newly created session object.
     */
    std::shared_ptr<fb::gateway::session> handle_accepted(fb::socket<fb::gateway::session>& socket) final;
    /**
     * @brief      Handles successful client connection establishment.
     *
     * @param      session  The newly connected session to handle.
     *
     * @return     An async task returning true if connection handling succeeded.
     */
    [[nodiscard]] async::task<bool> handle_connected(fb::socket<fb::gateway::session>& session) final;
    /**
     * @brief      Handles client disconnection and cleanup.
     *
     * @param      session  The disconnected session to clean up.
     *
     * @return     An async task returning true if disconnection handling succeeded.
     */
    [[nodiscard]] async::task<bool> handle_disconnected(fb::socket<fb::gateway::session>& session) final;

    /**
     * @brief      Handles server shutdown command received via AMQP.
     *
     * @param[in]  response  The shutdown response containing shutdown parameters.
     *
     * @return     An async task that completes when shutdown handling is finished.
     */
    [[nodiscard]] async::task<void> handle_amqp_shutdown(const internal_resp::Shutdown& response);

    // for heart-beat

protected:
    /**
     * @brief      Gets the service type for this gateway server.
     *
     * @return     The service type identifier for gateway servers.
     */
    Service service() const override final
    {
        return Service::Gateway;
    }

public:
};

} // namespace fb::gateway

#endif // !__FB_GATEWAY_H__
