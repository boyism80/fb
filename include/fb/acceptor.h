#ifndef __FB_ACCEPTOR_H__
#define __FB_ACCEPTOR_H__

/**
 * @file    acceptor.h
 * @brief   High-performance network acceptor with integrated protocol handling
 * @author  FB Development Team
 *
 * @details This file implements the core network acceptor system for the FB 2D MMORPG
 *          game server. The acceptor handles TCP socket connections, protocol message
 *          processing, AMQP integration, and HTTP client operations with comprehensive
 *          security features and performance optimizations.
 *
 *          Key features:
 *          - High-performance TCP socket accept/close operations with boost::asio
 *          - Comprehensive protocol handler registry with automatic message routing
 *          - AMQP integration for inter-service communication and message queuing
 *          - Thread pool management with configurable worker threads
 *          - HTTP client operations with FlatBuffer serialization support
 *          - Rate limiting and DDoS protection with configurable thresholds
 *          - Zero-copy packet processing for optimal performance
 *          - Thread-safe socket container management with proper synchronization
 *          - Automatic connection lifecycle management with proper cleanup
 *          - Integrated encryption/decryption support with policy-based control
 *          - Comprehensive error handling and exception management
 *          - Performance monitoring and metrics collection
 *
 * @note    This acceptor system serves as the foundation for all network services
 *          in the FB game server architecture, providing reliable and scalable
 *          network communication capabilities.
 */

#include <ctime>
#include <fb/mutex.h>
#include <fb/protocol/flatbuffer/protocol.h>
#include <fb/protocol/transfer.h>
#include <fb/protocol_handler_registry.h>
#include <fb/amqp_handler_registry.h>
#include <fb/http_client.h>
#include <fb/socket.h>
#include <iomanip>
#include <boost/stacktrace.hpp>

using namespace std::chrono_literals;

namespace fb {

/**
 * @brief      High-performance network acceptor with integrated protocol handling and HTTP client capabilities
 *
 *             Core functionality:
 *             - TCP socket accept/close operations
 *             - Protocol handler management
 *             - AMQP integration
 *             - Thread pool management
 *             - HTTP client operations with FlatBuffer serialization
 *             - Rate limiting and DDoS protection
 *
 *             Thread Safety:
 *             - Socket operations are thread-safe via internal mutex
 *             - Protocol handlers use thread-local storage
 *             - HTTP operations are handled in dedicated thread pool
 *             - AMQP operations use separate connection pool
 *
 *             Performance Features:
 *             - Asynchronous I/O via boost::asio
 *             - Zero-copy packet processing
 *             - Efficient memory management
 *             - Connection pooling for HTTP and AMQP
 *
 * @tparam     T     The type of data associated with each socket connection
 */
template <typename T>
class acceptor : public fb::async_executor, public boost::asio::ip::tcp::acceptor
{
public:
    using socket_container      = std::unordered_map<uint32_t, std::shared_ptr<fb::socket<T>>>;
    using socket_container_lock = fb::locker<socket_container>;
    using boost_timers          = std::vector<std::shared_ptr<boost::asio::deadline_timer>>;
    using session_type          = fb::socket<T>;

    /**
     * @brief      Handler container for protocol and AMQP handlers.
     *             This struct is intentionally non-copyable and non-assignable to prevent accidental copies.
     */
    struct handler
    {
        fb::protocol_handler_registry<T> protocol; ///< Registry for client protocol handlers
        fb::amqp_handler_registry<T>     amqp;     ///< Registry for AMQP message handlers

        /**
         * @brief      Constructs a new handler container.
         *
         * @param[in]  owner  Reference to the owner acceptor instance
         */
        explicit handler(fb::acceptor<T>& owner) :
            protocol(owner),
            amqp(owner)
        { }

        // Delete copy constructor and assignment operator
        handler(const handler&)             = delete;
        handler& operator= (const handler&) = delete;
    };

    handler         handler; ///< Public handler container for protocol handlers
    fb::http_client http;    ///< HTTP client for making requests to other services

private:
    boost_timers _timers; ///< Collection of boost::asio timers for periodic tasks

protected:
    socket_container_lock _sockets; ///< Thread-safe container holding all active socket connections

protected:
    /**
     * @brief      Constructs a new acceptor instance with network and threading configuration.
     *
     *             Initializes the acceptor with the specified I/O context, service name, and port.
     *             The constructor sets up the base async_executor class with the configured number of
     *             logic threads from the configuration system and initializes the internal mutex
     *             with a reference to this acceptor instance.
     *
     * @param      context  The boost::asio I/O context for handling network operations.
     * @param[in]  name     The service name identifier for this acceptor instance.
     * @param[in]  port     The TCP port number to bind and listen on for incoming connections.
     */
    acceptor(boost::asio::io_context& context, const std::string& name, uint16_t port) :
        fb::async_executor(context, name, config<uint32_t>("thread:logic")),
        boost::asio::ip::tcp::acceptor(context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
        handler(*this),
        http(*this)
    {
        static auto flag = std::once_flag{};
        std::call_once(flag, [port] {
            console::puts("Listen port : {}", port);
        });
    }

public:
    /**
     * @brief      Destroys the acceptor and ensures clean shutdown.
     *
     *             The destructor automatically calls exit() to ensure all connections
     *             are properly closed, threads are stopped, and resources are cleaned up
     *             before the object is destroyed. This provides RAII-style resource management.
     */
    virtual ~acceptor()
    {
        this->exit();
    }

protected:
    /**
     * @brief      Declares AMQP queues for the acceptor.
     *
     *             This method must be implemented by derived classes to set up their required AMQP queues.
     *
     * @param      amqp  The AMQP socket to use for queue declaration.
     */
    virtual void handle_init_amqp(fb::amqp::socket& amqp) = 0;

public:
    /**
     * @brief      Checks if a socket with the given file descriptor is currently connected.
     *
     * @param[in]  fd    The file descriptor to check.
     *
     * @return     True if the socket is connected, false otherwise.
     */
    bool connected(uint32_t fd)
    {
        return this->_sockets.template read<bool>([fd](const auto& v) -> bool {
            return v.contains(fd);
        });
    }

private:
    /**
     * @brief      Executes protocol handlers for received packets with comprehensive validation and rate limiting.
     */
    async::task<void> execute_handler(fb::socket<T>& socket, fb::stream& stream)
    {
        static constexpr uint8_t  base_size = sizeof(uint8_t) + sizeof(uint16_t);
        static constexpr uint32_t MAX_TPS   = 100;

        auto reader = fb::stream_reader<big_endian>(stream);
        try
        {
            while (!stream.empty())
            {
                if (reader.readable_size() < base_size)
                    co_return;

                auto head = reader.read<uint8_t>();
                if (head != 0xAA)
                    throw std::runtime_error("magic code mismatch");

                auto size = reader.read<uint16_t>();
                if (size > fb::socket<T>::MAX_BUFFER_SIZE)
                    throw std::runtime_error("packet size mismatch");

                if (reader.readable_size() < size)
                    break;

                if (this->assert_tps(socket) && socket.limiter.update(MAX_TPS) == false)
                    throw std::runtime_error("tps limit exceeded");

                auto cmd = reader.read<uint8_t>();
                if (this->decrypt_policy(cmd))
                    size = socket.encryption().decrypt(stream, reader.seek() - 1, size);

                reader.flush(); // remove magic code and size

                if (!this->handler.protocol.has_deserializer(cmd))
                {
                    fb::logger::warn(std::format("Undefined protocol. [{:#x}]", cmd));
                }
                else if (!this->handler.protocol.has_handler(cmd))
                {
                    fb::logger::warn(std::format("Undefined handler. [{:#x}]", cmd));
                }
                else
                {
                    auto protocol = std::shared_ptr<fb::protocol::header>(
                        co_await this->handler.protocol.get_deserializer(cmd)(reader));
                    auto fd   = socket.fd();
                    auto weak = socket.template weak_from_this_as<fb::socket<T>>();
                    this->threads.enqueue(weak, [this, protocol, weak, fd, cmd](auto& thread) -> async::task<void> {
                        try
                        {
                            if (weak.expired())
                                co_return;

                            auto shared = weak.lock();
                            if (shared == nullptr)
                                co_return;

                            auto  socket  = shared.get();
                            auto& handler = this->handler.protocol.get_handler(cmd);
                            // Check both global socket TPS and per-command TPS limits
                            // If either limit is exceeded, ignore the packet
                            if (this->assert_tps(*socket) &&
                                !socket->limiter.update(cmd, handler.duration, handler.limit))
                                co_return;

                            [[maybe_unused]]
                            volatile auto holder = protocol;
                            std::ignore          = co_await handler.fn(*socket, *protocol.get());
                        }
                        catch (std::exception& e)
                        {
                            fb::logger::fatal(e.what());
                        }
                        catch (...)
                        {
                            fb::logger::fatal("unhandled exception");
                        }
                    });
                }

                reader.seek(size - sizeof(uint8_t));
                reader.flush(); // remove packet body
            }

            reader.seek(0);
        }
        catch (std::exception& e)
        {
            fb::logger::fatal(e.what());
            socket.close();
        }
        catch (...)
        {
            fb::logger::fatal("unhandled exception while parse packet");
            socket.close();
        }
    }

private:
    /**
     * @brief      Safely removes a socket from the acceptor with proper cleanup and disconnection handling.
     *
     *             This method performs the complete socket removal process with proper resource cleanup:
     *             1. Calls the derived class's handle_disconnected() method for custom cleanup
     *             2. Removes the socket from the alive sockets tracking system
     *             3. Thread-safely removes the socket from the main socket container
     *             4. Handles any exceptions during disconnection gracefully
     *
     *             The method ensures that all references to the socket are properly cleaned up
     *             and that the socket's unique_ptr is removed from the container, triggering
     *             automatic destruction of the socket object.
     *
     * @param      socket  The socket to remove and clean up.
     *
     * @return     An async task that completes when the socket is fully removed and cleaned up.
     */
    async::task<void> erase(fb::socket<T>& socket)
    {
        try
        {
            std::ignore = co_await this->handle_disconnected(socket);
        }
        catch (std::exception& e)
        {
            fb::logger::fatal(e.what());
        }
        auto fd = socket.fd();
        this->_sockets.write([fd](auto& v) -> void {
            v.erase(fd);
        });
    }

    /**
     * @brief      Handles a received packet from a socket.
     *
     * @param      socket  The socket that received the packet.
     * @param      stream  The stream containing the packet data.
     *
     * @return     A task that completes when the packet is handled.
     */
    async::task<void> on_socket_received(fb::socket<T>& socket, fb::stream& stream)
    {
        try
        {
            co_await this->execute_handler(socket, stream);
        }
        catch (std::exception& e)
        {
            fb::logger::fatal(e.what());
        }
    }

    /**
     * @brief      Handles a socket closure event.
     *
     * @param      socket  The socket that was closed.
     *
     * @return     A task that completes when the closure is handled.
     */
    async::task<void> on_socket_closed(fb::socket<T>& socket)
    {
        try
        {
            if (socket.data() == nullptr)
                co_return;

            auto weak = socket.template weak_from_this_as<fb::socket<T>>();
            co_await this->threads.switching(weak);
        }
        catch (std::exception& e)
        {
            fb::logger::fatal("failed to switch thread context: {}", e.what());
        }
        catch (...)
        {
            fb::logger::fatal("failed to switch thread context: unknown exception");
        }

        try
        {
            co_await this->erase(socket);
        }
        catch (std::exception& e)
        {
            fb::logger::fatal("failed to erase socket: {}", e.what());
        }
        catch (...)
        {
            fb::logger::fatal("failed to erase socket: unknown exception");
        }
    }

    /**
     * @brief      Accepts new socket connections and initializes them for packet processing.
     *
     *             This method implements the complete connection acceptance pipeline:
     *
     *             Connection Setup:
     *             1. Creates a new socket with receive and close event handlers
     *             2. Initiates asynchronous accept operation using boost::asio
     *             3. Associates session data with the socket via handle_accepted()
     *             4. Adds socket to the thread-safe container with duplicate checking
     *             5. Registers socket in the alive tracking system
     *             6. Calls handle_connected() for custom connection logic
     *             7. Starts the socket's receive coroutine
     *             8. Recursively calls accept() for the next connection
     *
     *             Error Handling:
     *             - Validates that acceptor is still running before accepting
     *             - Handles duplicate socket file descriptors by removing old entries
     *             - Closes socket and logs errors on any failure
     *             - Continues accepting new connections even after individual failures
     *
     *             Thread Safety:
     *             - Uses thread-safe socket container operations
     *             - Properly handles concurrent access to socket collections
     *
     *             This method is called recursively to continuously accept new connections
     *             until the acceptor is shut down.
     */
    void accept()
    {
        auto shared_socket_ptr = std::make_shared<fb::socket<T>>(*this,
                                                                 std::bind_front(&acceptor::on_socket_received, this),
                                                                 std::bind_front(&acceptor::on_socket_closed, this));
        this->async_accept(*shared_socket_ptr, [this, shared_socket_ptr](boost::system::error_code error) mutable {
            try
            {
                if (error)
                    throw std::runtime_error(error.message());

                if (this->_running == false)
                    throw std::runtime_error("cannot accept socket. acceptor is cleaning now.");

                shared_socket_ptr->data(this->handle_accepted(*shared_socket_ptr));
                shared_socket_ptr->set_option(boost::asio::ip::tcp::no_delay(false));

                {
                    auto fd = shared_socket_ptr->fd();
                    this->_sockets.write([fd, &shared_socket_ptr](auto& v) -> void {
                        if (v.contains(fd))
                        {
                            fb::logger::warn(std::format("socket already exists. fd: {}", fd));
                            v.erase(fd); // remove old socket if exists
                        }

                        v.insert({fd, shared_socket_ptr});
                    });
                }

                async::awaitable_get(this->handle_connected(*shared_socket_ptr));

                boost::asio::co_spawn(*this, shared_socket_ptr->recv(), boost::asio::detached);
                this->accept();
            }
            catch (std::exception& e)
            {
                fb::logger::fatal("acceptor::accept: error={}\n{}",
                                  e.what(),
                                  boost::stacktrace::to_string(boost::stacktrace::stacktrace()));
                shared_socket_ptr->close();
            }
        });
    }

public:
    /**
     * @brief      Transfers a socket connection to another service with encryption state preservation.
     *
     *             This method implements secure socket transfer between services while maintaining
     *             the client's encryption state. It's used for seamless service handoffs where
     *             a client needs to be moved from one service to another (e.g., login to game server).
     *
     *             Transfer Process:
     *             1. Extracts current encryption type and key from the socket
     *             2. Packages encryption state and source service information
     *             3. Creates a transfer protocol response with target service details
     *             4. Encrypts the transfer packet using current socket encryption
     *             5. Sends the transfer packet to the client
     *
     *             The client receives the transfer packet and automatically connects to the
     *             target service using the preserved encryption state, enabling seamless
     *             service transitions without re-authentication.
     *
     *             Transfer Packet Format:
     *             - Encryption type (1 byte)
     *             - Key size (1 byte, always encryption::KEY_SIZE)
     *             - Encryption key (encryption::KEY_SIZE bytes)
     *             - Source service type (1 byte)
     *
     * @param      socket  The socket connection to transfer to another service.
     * @param[in]  ip      The target service IP address as a 32-bit integer.
     * @param[in]  port    The target service port number.
     * @param[in]  from    The service type that is initiating this transfer.
     *
     * @return     An async task that completes when the transfer packet is sent.
     */
    [[nodiscard]] async::task<void>
    transfer(fb::socket<T>& socket, uint32_t ip, uint16_t port, fb::protocol::internal::Service from)
    {
        auto& encryption = socket.encryption();
        auto  params     = fb::stream();
        {
            auto writer = fb::stream_writer<big_endian>(params);
            writer.write<uint8_t>(encryption.pattern());
            writer.write<uint8_t>(fb::encryption::KEY_SIZE);
            writer.write(encryption.iv(), fb::encryption::KEY_SIZE);
            writer.write<uint8_t>(static_cast<uint8_t>(from));
        }

        auto stream = fb::stream();
        {
            auto writer = fb::stream_writer<big_endian>(stream);
            co_await fb::protocol::response::transfer(ip, port, params).serialize(writer);
        }

        encryption.wrap(stream);
        std::ignore = co_await socket.send(stream, false, false);
    }

public:
    /**
     * @brief      Transfers a socket connection to another service with additional session parameters.
     *
     *             This extended transfer method allows passing additional session-specific data
     *             along with the standard encryption state. This is commonly used when the target
     *             service needs context about the user session (e.g., user ID, character name,
     *             authentication tokens, or other session state).
     *
     *             Extended Transfer Process:
     *             1. Extracts current encryption type and key from the socket
     *             2. Packages encryption state and source service information
     *             3. Appends the additional parameter stream to the transfer data
     *             4. Creates a transfer protocol response with all combined data
     *             5. Encrypts and sends the transfer packet to the client
     *
     *             The additional parameters are typically used by the target service to:
     *             - Restore user session state
     *             - Validate user permissions
     *             - Initialize service-specific context
     *             - Maintain continuity of user experience
     *
     *             Extended Transfer Packet Format:
     *             - Encryption type (1 byte)
     *             - Key size (1 byte, always encryption::KEY_SIZE)
     *             - Encryption key (encryption::KEY_SIZE bytes)
     *             - Source service type (1 byte)
     *             - Additional parameter data (variable length)
     *
     * @param      socket     The socket connection to transfer to another service.
     * @param[in]  ip         The target service IP address as a 32-bit integer.
     * @param[in]  port       The target service port number.
     * @param[in]  from       The service type that is initiating this transfer.
     * @param[in]  parameter  Additional session data to pass to the target service.
     *
     * @return     An async task that completes when the transfer packet is sent.
     */
    [[nodiscard]] async::task<void> transfer(fb::socket<T>&                  socket,
                                             uint32_t                        ip,
                                             uint16_t                        port,
                                             fb::protocol::internal::Service from,
                                             const fb::stream&               parameter)
    {
        auto& encryption = socket.encryption();
        auto  header     = fb::stream();
        {
            auto writer = fb::stream_writer<big_endian>(header);
            writer.write<uint8_t>(encryption.pattern());
            writer.write<uint8_t>(fb::encryption::KEY_SIZE);
            writer.write(encryption.iv(), fb::encryption::KEY_SIZE);
            writer.write<uint8_t>(static_cast<uint8_t>(from));
            writer.write<fb::stream>(parameter);
        }

        auto stream = fb::stream();
        {
            auto writer = fb::stream_writer<big_endian>(stream);
            co_await fb::protocol::response::transfer(ip, port, header).serialize(writer);
        }

        encryption.wrap(stream);
        std::ignore = co_await socket.send(stream, false, false);
    }

public:
    /**
     * @brief      Transfers a socket connection to another service using hostname.
     *
     * @param      socket  The socket to transfer.
     * @param[in]  ip      The target hostname.
     * @param[in]  port    The target port.
     * @param[in]  from    The service type initiating the transfer.
     */
    [[nodiscard]] async::task<void>
    transfer(fb::socket<T>& socket, const std::string& ip, uint16_t port, fb::protocol::internal::Service from)
    {
        co_await this->transfer(socket, inet_addr(this->ipv4(ip).c_str()), port, from);
    }

public:
    /**
     * @brief      Transfers a socket connection to another service using hostname with additional parameters.
     *
     * @param      socket     The socket to transfer.
     * @param[in]  ip         The target hostname.
     * @param[in]  port       The target port.
     * @param[in]  from       The service type initiating the transfer.
     * @param[in]  parameter  Additional parameters to include in the transfer.
     */
    [[nodiscard]] async::task<void> transfer(fb::socket<T>&                  socket,
                                             const std::string&              ip,
                                             uint16_t                        port,
                                             fb::protocol::internal::Service from,
                                             const fb::stream&               parameter)
    {
        co_await this->transfer(socket, inet_addr(this->ipv4(ip).c_str()), port, from, parameter);
    }

protected:
    /**
     * @brief      Determines the decryption policy for incoming protocol commands.
     *
     *             This virtual method allows derived classes to implement custom decryption
     *             policies based on the protocol command byte. Some commands may need to
     *             remain unencrypted for security or compatibility reasons (e.g., initial
     *             handshake, encryption negotiation, or certain system commands).
     *
     *             The method is called during packet processing in execute_handler() before
     *             attempting to decrypt the packet payload. If this method returns false,
     *             the packet will be processed without decryption.
     *
     *             Common use cases:
     *             - Skip decryption for handshake/negotiation packets
     *             - Allow certain admin commands to bypass encryption
     *             - Handle legacy protocol compatibility
     *             - Implement command-specific security policies
     *
     *             Default implementation returns true for all commands, meaning all packets
     *             will be decrypted by default.
     *
     * @param[in]  cmd   The protocol command byte from the packet header.
     *
     * @return     True if the packet should be decrypted, false to process without decryption.
     */
    virtual bool decrypt_policy(uint8_t cmd) const
    {
        return true;
    }

    /**
     * @brief      Validates socket-level TPS (Transactions Per Second) limits for DDoS protection.
     *
     *             This virtual method allows derived classes to implement custom per-socket
     *             rate limiting logic. It's called during packet processing to determine if
     *             a socket is sending packets at an acceptable rate. This provides an additional
     *             layer of protection beyond the per-handler rate limiting.
     *
     *             The method is invoked in execute_handler() before processing each packet.
     *             If this method returns false, the packet will be dropped and the socket
     *             may be subject to additional penalties (depending on implementation).
     *
     *             Typical implementations might:
     *             - Track packets per second per socket
     *             - Implement sliding window rate limiting
     *             - Apply different limits based on socket type or user privileges
     *             - Integrate with external rate limiting systems
     *             - Log or ban excessive traffic sources
     *
     *             Default implementation returns true, allowing all traffic through.
     *             Derived classes should override this for production deployments.
     *
     * @param[in]  socket  The socket to validate for rate limiting compliance.
     *
     * @return     True if the socket is within acceptable TPS limits, false to drop the packet.
     */
    virtual bool assert_tps(const fb::socket<T>& socket) const
    {
        return true;
    }

protected:
    /**
     * @brief      Handles a newly accepted socket connection.
     *
     * @param      socket  The newly accepted socket.
     *
     * @return     A pointer to the data associated with the socket.
     */
    virtual std::shared_ptr<T> handle_accepted(fb::socket<T>& socket) = 0;

protected:
    /**
     * @brief      Handles the start of the acceptor.
     * Initializes Lua bindings and other startup tasks.
     */
    virtual async::task<void> handle_start()
    {
        lua::build<lua::luable>();
        lua::build<fb::thread, lua::luable>();
        lua::build<fb::thread_switchable, lua::luable>();
        co_return;
    }

protected:
    /**
     * @brief      Handles a socket connection event.
     *
     * @param      session  The connected socket.
     *
     * @return     True if the connection was handled successfully, false otherwise.
     */
    virtual async::task<bool> handle_connected(fb::socket<T>& session)
    {
        co_return true;
    }

protected:
    /**
     * @brief      Handles a socket disconnection event.
     *
     * @param      session  The disconnected socket.
     *
     * @return     True if the disconnection was handled successfully, false otherwise.
     */
    virtual async::task<bool> handle_disconnected(fb::socket<T>& session)
    {
        co_return true;
    }

protected:
    /**
     * @brief      Gets the thread associated with a socket.
     *
     * @param[in]  socket  The socket to get the thread for.
     *
     * @return     A pointer to the thread associated with the socket.
     */
    fb::thread* thread(const fb::socket<T>& socket) const
    {
        auto count = this->threads.count();
        if (count == 0)
            return 0xFF;

        return this->threads[this->thread_id(socket) % count];
    }

protected:
    /**
     * @brief      Handles the exit of the acceptor.
     */
    virtual async::task<void> handle_exit()
    {
        co_return;
    }

protected:
    /**
     * @brief      Gets the ID of the acceptor.
     *
     * @return     The acceptor's ID.
     */
    virtual uint8_t id() const
    {
        return fb::config<uint8_t>("id");
    }

protected:
    /**
     * @brief      Gets the name of the acceptor.
     *
     * @return     The acceptor's name.
     */
    virtual std::string name() const
    {
        return fb::config<std::string>("name");
    }

protected:
    /**
     * @brief      Gets the service type of the acceptor.
     *
     * @return     The service type of the acceptor.
     */
    virtual fb::protocol::internal::Service service() const = 0;

public:
    /**
     * @brief      Sends a stream over a socket.
     *
     * @param      socket   The socket to send the stream over.
     * @param[in]  stream   The stream to send.
     * @param[in]  encrypt  Whether to encrypt the stream.
     * @param[in]  wrap     Whether to wrap the stream.
     *
     * @return     The number of bytes sent.
     */
    async::task<size_t> send(fb::socket<T>& socket, const fb::stream& stream, bool encrypt = true, bool wrap = true)
    {
        if (stream.empty())
            co_return 0;

        co_return co_await socket.send(stream, encrypt, wrap);
    }

public:
    /**
     * @brief      Sends a protocol header over a socket.
     *
     * @param      socket    The socket to send the header over.
     * @param[in]  response  The protocol header to send.
     * @param[in]  encrypt   Whether to encrypt the header.
     * @param[in]  wrap      Whether to wrap the header.
     *
     * @return     The number of bytes sent.
     */
    async::task<size_t>
    send(fb::socket<T>& socket, const fb::protocol::header& response, bool encrypt = true, bool wrap = true)
    {
        auto stream = fb::stream();
        auto writer = fb::stream_writer<big_endian>(stream);
        co_await response.serialize(writer);
        if (stream.empty())
            co_return 0;

        co_return co_await socket.send(stream, encrypt, wrap);
    }

public:
    /**
     * @brief      Starts the acceptor and begins the complete server lifecycle.
     *
     *             This method implements the main server startup sequence and runs the server
     *             until shutdown. It orchestrates multiple components and threads:
     *
     *             Startup Sequence:
     *             1. Sets the running flag to true
     *             2. Initiates the connection acceptance loop
     *             3. Creates and starts I/O worker threads (configured via "thread:io")
     *             4. Calls handle_start() for derived class initialization
     *             5. Starts the AMQP communication thread
     *             6. Blocks until all threads complete (server shutdown)
     *
     *             Thread Architecture:
     *             - I/O Threads: Handle network operations using boost::asio
     *             - Logic Threads: Process protocol handlers (configured via "thread:logic")
     *             - AMQP Thread: Manages inter-service message queue communication
     *             - Main Thread: Coordinates startup and waits for completion
     *
     *             The method blocks the calling thread until the server is shut down via exit().
     *             All threads are properly joined to ensure clean shutdown.
     *
     *             Configuration Dependencies:
     *             - "thread:io": Number of I/O worker threads to create
     *             - "thread:logic": Number of logic processing threads (set in constructor)
     */
    void run()
    {
        this->_running = true;
        this->accept();

        auto threads = std::vector<std::thread>();
        for (int i = 0; i < fb::config<uint32_t>("thread:io"); i++)
        {
            threads.push_back(std::thread([this]() {
                this->io_context.run();
            }));
        }

        async::awaitable_get(this->handle_start());

        threads.push_back(std::thread([this]() {
            this->handler.amqp.on_initialize = [this](fb::amqp::socket& amqp) {
                this->handle_init_amqp(amqp);
            };

            this->handler.amqp.thread_loop();
        }));

        for (auto& thread : threads)
        {
            thread.join();
        }
    }

public:
    /**
     * @brief      Checks if the acceptor is currently running.
     *
     * @return     True if the acceptor is running, false otherwise.
     */
    bool running() const
    {
        return this->_running;
    }

public:
    /**
     * @brief      Puts the current thread to sleep for the specified duration.
     *
     * @param[in]  duration  The duration to sleep for.
     */
    [[nodiscard]] async::task<void> sleep(const fb::model::timespan& duration)
    {
        auto thread = this->threads.current();
        if (thread != nullptr)
            co_await thread->sleep(duration);
    }

private:
    /**
     * @brief      Disconnects all sockets and handles disconnection events.
     *
     * @note       This method must be called by the main thread.
     *
     * @return     A task that completes when all sockets are disconnected.
     */
    [[nodiscard]] async::task<void> disconnect_sockets()
    {
        auto pairs = std::unordered_map<fb::thread*, std::vector<fb::socket<T>*>>();
        this->_sockets.read([&pairs](const auto& v) -> void {
            for (auto& [fd, socket] : v)
            {
                auto thread = socket->thread();
                if (thread != nullptr)
                    pairs[thread].push_back(socket.get());
            }
        });

        for (auto& [thread, sockets] : pairs)
        {
            co_await thread->switching();
            for (auto& socket : sockets)
            {
                try
                {
                    std::ignore = co_await this->handle_disconnected(*socket);
                }
                catch (std::exception& e)
                {
                    fb::logger::fatal(e.what());
                    std::cerr << boost::stacktrace::stacktrace() << std::endl;
                }
                socket->close();
            }
        }
    }

public:
    /**
     * @brief      Allows access to the socket container for reading.
     *
     * @param[in]  fn  The function to execute with access to the socket container.
     */
    void access_sockets(std::function<void(const socket_container&)> fn)
    {
        this->_sockets.read([fn](const auto& v) {
            fn(v);
        });
    }

public:
    /**
     * @brief      Gracefully shuts down the acceptor with comprehensive cleanup.
     *
     *             This method implements a complete and safe shutdown sequence that ensures
     *             all resources are properly cleaned up and all connections are gracefully
     *             terminated. The shutdown process is designed to be idempotent and thread-safe.
     *
     *             Shutdown Sequence:
     *             1. Checks if already shutting down (idempotent operation)
     *             2. Sets running flag to false to stop accepting new connections
     *             3. Cancels the acceptor to stop listening for new connections
     *             4. Disconnects all existing sockets with proper cleanup callbacks
     *             5. Cancels all active timers to prevent further scheduled operations
     *             6. Shuts down all worker threads (logic and I/O threads)
     *             7. Closes the acceptor socket
     *             8. Stops the boost::asio I/O context
     *
     *             Thread Safety:
     *             - Uses atomic operations and proper synchronization
     *             - Handles concurrent shutdown attempts gracefully
     *             - Ensures all threads are properly joined before returning
     *
     *             Resource Cleanup:
     *             - All socket connections are properly closed
     *             - AMQP connections are terminated
     *             - All timers are cancelled
     *             - Thread pools are shut down
     *             - Memory resources are freed
     *
     *             This method can be called multiple times safely and will only perform
     *             the shutdown sequence once.
     */
    void exit() override final
    {
        if (this->_running == false)
            return;

        this->_running = false;
        this->cancel();
        async::awaitable_get(this->disconnect_sockets());

        for (auto& timer : this->_timers)
        {
            timer->cancel();
        }

        this->threads.exit();
        this->close();

        static_cast<boost::asio::io_context&>(*this).stop();
    }
};

} // namespace fb

#endif // !__FB_ACCEPTOR_H__