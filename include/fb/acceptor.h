#ifndef __FB_ACCEPTOR_H__
#define __FB_ACCEPTOR_H__

#include <ctime>
#include <fb/mutex.h>
#include <fb/protocol/flatbuffer/protocol.h>
#include <fb/protocol/transfer.h>
#include <fb/protocol_handler_registry.h>
#include <fb/amqp_handler_registry.h>
#include <fb/socket.h>
#include <iomanip>
#include <fb/amqp.h>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/stacktrace.hpp>

using namespace std::chrono_literals;

namespace fb {

/**
 * @brief      A high-performance network acceptor that manages client connections and protocol handling.
 *
 *             Core functionality:
 *             - TCP socket accept/close operations
 *             - Protocol handler registration and dispatch
 *             - AMQP message queue integration
 *             - Thread pool management for I/O and logic
 *
 *             The acceptor provides thread-safe operations and uses boost::asio for asynchronous I/O.
 *             Derived classes must implement handle_accepted() for session initialization and
 *             handle_init_amqp() for AMQP setup.
 *
 * @tparam     T    The session type for socket connections
 */
template <typename T>
class acceptor : public fb::acceptable
{
public:
    using socket_container      = std::unordered_map<uint32_t, std::unique_ptr<fb::socket<T>>>;
    using socket_container_lock = fb::locker<socket_container>;
    using boost_timers          = std::vector<std::shared_ptr<boost::asio::deadline_timer>>;

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

    handler handler; ///< Public handler container for protocol handlers

private:
    boost_timers _timers; ///< Collection of boost::asio timers for periodic tasks

protected:
    socket_container_lock _sockets; ///< Thread-safe container holding all active socket connections

protected:
    /**
     * @brief      Constructs a new acceptor instance with network and threading configuration.
     *
     *             Initializes the acceptor with the specified I/O context, service name, and port.
     *             The constructor sets up the base acceptable class with the configured number of
     *             logic threads from the configuration system and initializes the internal mutex
     *             with a reference to this acceptor instance.
     *
     * @param      context  The boost::asio I/O context for handling network operations.
     * @param[in]  name     The service name identifier for this acceptor instance.
     * @param[in]  port     The TCP port number to bind and listen on for incoming connections.
     */
    acceptor(boost::asio::io_context& context, const std::string& name, uint16_t port) :
        fb::acceptable(context, name, config<uint32_t>("thread:logic"), port),
        handler(*this)
    { }

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

protected:
    /**
     * @brief      Gets the IPv4 address from a hostname or IP address string.
     *
     * @param[in]  ip  The hostname or IP address to resolve.
     *
     * @return     The resolved IPv4 address as a string.
     */
    std::string ipv4(const std::string& ip) const
    {
        try
        {
            auto resolver = boost::asio::ip::tcp::resolver(this->_boost_context);
            auto results  = resolver.resolve(ip, "0");

            for (const auto& entry : results)
            {
                auto addr = entry.endpoint().address();
                if (addr.is_v4())
                    return addr.to_string();
            }

            throw std::runtime_error(std::format("Failed to resolve IPv4 address for: {}", ip));
        }
        catch (const std::exception& e)
        {
            throw std::runtime_error(std::format("Error resolving address: {}", e.what()));
        }
    }

private:
    /**
     * @brief      Performs an asynchronous HTTP GET request using boost::beast.
     *
     * @param[in]  host      The target host in format "hostname:port" or "hostname" (defaults to port 80).
     *                       Supports http:// and https:// prefixes which are automatically stripped.
     * @param[in]  path      The target path including query parameters (will be URL encoded).
     * @param[in]  headers   Custom HTTP headers to include in the request.
     * @param[in]  timeout   Maximum time to wait for connection, request, and response operations.
     *
     * @return     A coroutine task that completes with the response body as a byte vector.
     *
     * @throws     std::exception on network errors, timeout, or invalid response.
     */
    boost::asio::awaitable<std::vector<uint8_t>> boost_get_async(std::string                         host,
                                                                 std::string                         path,
                                                                 std::map<std::string, std::string>  headers,
                                                                 std::chrono::steady_clock::duration timeout)
    {
        try
        {
            auto raw_host = host;
            if (raw_host.rfind("http://", 0) == 0)
                raw_host.erase(0, 7);
            else if (raw_host.rfind("https://", 0) == 0)
                raw_host.erase(0, 8);

            auto const colon_pos = raw_host.find(':');
            auto const host_name = (colon_pos == std::string::npos ? raw_host : raw_host.substr(0, colon_pos));
            auto const port = (colon_pos == std::string::npos ? std::string("80") : raw_host.substr(colon_pos + 1));

            auto resolver = boost::asio::ip::tcp::resolver{this->_boost_context};
            auto stream   = boost::beast::tcp_stream{this->_boost_context};

            stream.expires_after(timeout);
            auto const results = co_await resolver.async_resolve(host_name, port, boost::asio::use_awaitable);
            co_await stream.async_connect(results, boost::asio::use_awaitable);

            auto req =
                boost::beast::http::request<boost::beast::http::empty_body>{boost::beast::http::verb::get,
                                                                            url_encode(UTF8(path, PLATFORM::WINDOWS)),
                                                                            11};
            req.set(boost::beast::http::field::host, host_name);
            req.set(boost::beast::http::field::user_agent, BOOST_BEAST_VERSION_STRING);
            for (auto const& h : headers)
            {
                req.set(h.first, h.second);
            }

            stream.expires_after(timeout);
            co_await boost::beast::http::async_write(stream, req, boost::asio::use_awaitable);

            auto buffer = boost::beast::flat_buffer{};
            auto res    = boost::beast::http::response<boost::beast::http::dynamic_body>{};
            stream.expires_after(timeout);
            co_await boost::beast::http::async_read(stream, buffer, res, boost::asio::use_awaitable);

            auto body_bytes = std::vector<uint8_t>{};
            if (res.body().size() > 0)
            {
                body_bytes.reserve(res.body().size());
            }
            for (auto const& seq : res.body().data())
            {
                auto buf      = seq; // boost::asio::const_buffer
                auto data_ptr = static_cast<const uint8_t*>(buf.data());
                body_bytes.insert(body_bytes.end(), data_ptr, data_ptr + buf.size());
            }

            auto ec = boost::beast::error_code{};
            stream.socket().shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);

            co_return body_bytes;
        }
        catch (const std::exception& e)
        {
            throw std::runtime_error(std::format("HTTP GET request failed: {}", e.what()));
        }
        catch (...)
        {
            throw std::runtime_error("HTTP GET request failed: Unknown error occurred");
        }
    }

private:
    /**
     * @brief      Performs a typed HTTP GET request with automatic FlatBuffer deserialization.
     *
     * @param[in]  host      The target host in "hostname:port" format.
     * @param[in]  path      The target path for the GET request.
     *
     * @tparam     Response  The FlatBuffer response type that implements Deserialize() method.
     *
     * @return     An async task that completes with the deserialized response object.
     *
     * @throws     std::exception on network errors or deserialization failures.
     */
    template <typename Response>
    [[nodiscard]] async::task<Response> boost_get_async(const std::string& host, const std::string& path)
    {
        auto promise = std::make_shared<async::task_completion_source<Response>>();
        auto headers = std::map<std::string, std::string>{
            {"Content-Type", "application/octet-stream"},
        };

        auto thread = this->threads.current();
        boost::asio::co_spawn(this->_boost_context,
                              this->boost_get_async(host, path, headers, 5s),
                              [this, promise, thread](std::exception_ptr ep, std::vector<uint8_t> bytes) {
                                  async::awaitable_then(thread->switching(), [promise, ep, bytes](auto result) mutable {
                                      if (ep)
                                      {
                                          try
                                          {
                                              std::rethrow_exception(ep);
                                          }
                                          catch (...)
                                          {
                                              promise->set_exception(std::current_exception());
                                              return;
                                          }
                                      }

                                      try
                                      {
                                          auto reader        = fb::stream_reader<big_endian>(bytes);
                                          auto protocol_type = reader.read<uint32_t>();
                                          auto protocol_size = reader.read<uint32_t>();
                                          auto offset        = bytes.data() + sizeof(uint32_t) + sizeof(uint32_t);
                                          promise->set_value(Response::Deserialize(offset));
                                      }
                                      catch (std::exception& e)
                                      {
                                          promise->set_exception(std::make_exception_ptr(e));
                                      }
                                  });
                              });

        return promise->task();
    }

private:
    /**
     * @brief      Performs an asynchronous HTTP POST request using boost::beast with binary payload support.
     *
     * @param[in]  host      The target host in format "hostname:port" or "hostname" (defaults to port 80).
     *                       Supports http:// and https:// prefixes which are automatically stripped.
     * @param[in]  path      The target path including query parameters (will be URL encoded).
     * @param[in]  headers   Custom HTTP headers to include in the request.
     * @param[in]  timeout   Maximum time to wait for connection, request, and response operations.
     * @param[in]  body      The binary request body data to send in the POST request.
     *
     * @return     A coroutine task that completes with the response body as a byte vector.
     *
     * @throws     std::exception on network errors, timeout, or invalid response.
     */
    boost::asio::awaitable<std::vector<uint8_t>> boost_post_async(std::string                         host,
                                                                  std::string                         path,
                                                                  std::map<std::string, std::string>  headers,
                                                                  std::chrono::steady_clock::duration timeout,
                                                                  std::vector<uint8_t>                body)
    {
        try
        {
            auto raw_host = host;
            if (raw_host.rfind("http://", 0) == 0)
                raw_host.erase(0, 7);
            else if (raw_host.rfind("https://", 0) == 0)
                raw_host.erase(0, 8);

            auto const colon_pos = raw_host.find(':');
            auto const host_name = (colon_pos == std::string::npos ? raw_host : raw_host.substr(0, colon_pos));
            auto const port = (colon_pos == std::string::npos ? std::string("80") : raw_host.substr(colon_pos + 1));

            auto resolver = boost::asio::ip::tcp::resolver{this->_boost_context};
            auto stream   = boost::beast::tcp_stream{this->_boost_context};

            stream.expires_after(timeout);
            auto const results = co_await resolver.async_resolve(host_name, port, boost::asio::use_awaitable);
            co_await stream.async_connect(results, boost::asio::use_awaitable);

            auto req = boost::beast::http::request<boost::beast::http::vector_body<uint8_t>>{
                boost::beast::http::verb::post,
                url_encode(UTF8(path, PLATFORM::WINDOWS)),
                11};

            req.set(boost::beast::http::field::host, host_name);
            req.set(boost::beast::http::field::user_agent, BOOST_BEAST_VERSION_STRING);

            for (auto const& h : headers)
            {
                req.set(h.first, h.second);
            }

            req.body() = body;
            req.prepare_payload();

            stream.expires_after(timeout);
            co_await boost::beast::http::async_write(stream, req, boost::asio::use_awaitable);

            auto buffer = boost::beast::flat_buffer{};
            auto res    = boost::beast::http::response<boost::beast::http::dynamic_body>{};
            stream.expires_after(timeout);
            co_await boost::beast::http::async_read(stream, buffer, res, boost::asio::use_awaitable);

            auto body_bytes = std::vector<uint8_t>{};
            if (res.body().size() > 0)
            {
                body_bytes.reserve(res.body().size());
            }
            for (auto const& seq : res.body().data())
            {
                auto const buf      = seq;
                auto const data_ptr = static_cast<const uint8_t*>(buf.data());
                body_bytes.insert(body_bytes.end(), data_ptr, data_ptr + buf.size());
            }

            auto ec = boost::beast::error_code{};
            stream.socket().shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);

            co_return body_bytes;
        }
        catch (const std::exception& e)
        {
            throw std::runtime_error(std::format("HTTP request failed: {}", e.what()));
        }
        catch (...)
        {
            throw std::runtime_error("HTTP request failed: Unknown error occurred");
        }
    }

private:
    /**
     * @brief      Performs a typed HTTP POST request with automatic FlatBuffer serialization and deserialization.
     *
     * @param[in]  host     The target host in "hostname:port" format.
     * @param[in]  path     The target path for the POST request.
     * @param[in]  body     The request object to serialize and send.
     *
     * @tparam     Request   The FlatBuffer request type that implements Serialize() method.
     * @tparam     Response  The FlatBuffer response type that implements Deserialize() method.
     *
     * @return     An async task that completes with the deserialized response object.
     *
     * @throws     std::exception on network errors, serialization/deserialization failures.
     */
    template <typename Request, typename Response>
    [[nodiscard]] async::task<Response> boost_post_async(std::string const& host,
                                                         std::string const& path,
                                                         Request const&     body)
    {
        auto const serialized_payload = body.Serialize();
        auto       stream_req         = fb::stream();
        auto       writer             = fb::stream_writer<>(stream_req);

        writer.write<uint32_t>(static_cast<uint32_t>(Request::FlatBufferProtocolType));
        writer.write<uint32_t>(serialized_payload.size());
        writer.write(serialized_payload.data(), serialized_payload.size());

        auto promise = std::make_shared<async::task_completion_source<Response>>();
        auto headers = std::map<std::string, std::string>{
            {"Content-Type", "application/octet-stream"}
        };

        auto thread = this->threads.current();
        boost::asio::co_spawn(this->_boost_context,
                              this->boost_post_async(host, path, headers, std::chrono::seconds{5}, stream_req),
                              [promise, thread](std::exception_ptr ep, std::vector<uint8_t> bytes) {
                                  async::awaitable_then(thread->switching(), [promise, ep, bytes](auto result) mutable {
                                      if (ep)
                                      {
                                          promise->set_exception(ep);
                                          return;
                                      }
                                      try
                                      {
                                          auto reader        = fb::stream_reader<big_endian>(bytes);
                                          auto protocol_type = reader.read<uint32_t>();
                                          auto protocol_len  = reader.read<uint32_t>();
                                          auto offset        = bytes.data() + sizeof(uint32_t) * 2;

                                          promise->set_value(Response::Deserialize(offset));
                                      }
                                      catch (...)
                                      {
                                          promise->set_exception(std::current_exception());
                                      }
                                  });
                              });

        return promise->task();
    }

public:
    /**
     * @brief      Performs a configuration-based HTTP GET request to another service.
     *
     *             This method provides a high-level interface for making HTTP GET requests to
     *             other services using configuration-based routing. It automatically constructs
     *             the target URL from configuration parameters and handles the complete request
     *             lifecycle with automatic response deserialization.
     *
     *             The method looks up the target service configuration using the route parameter
     *             and constructs the full URL as "http://ip:port" + path. This enables
     *             service-to-service communication without hardcoding endpoints.
     *
     *             Configuration format expected:
     *             ```json
     *             {
     *               "route_name": {
     *                 "ip": "service.hostname.com",
     *                 "port": 8080
     *               }
     *             }
     *             ```
     *
     * @param[in]  route     The configuration route name to look up service endpoint details.
     * @param[in]  path      The target path to append to the service base URL.
     *
     * @tparam     Response  The FlatBuffer response type expected from the service.
     *
     * @return     An async task that completes with the deserialized response object.
     *
     * @throws     std::exception if route configuration is missing or request fails.
     */
    template <typename Response>
    [[nodiscard]] async::task<Response> get(const std::string& route, const std::string& path)
    {
        auto& config = fb::config<>(route);
        auto  host   = std::format("http://{}:{}", config["ip"].asCString(), config["port"].asUInt());
        co_return co_await this->boost_get_async<Response>(host, path);
    }

public:
    /**
     * @brief      Performs a configuration-based HTTP POST request to another service with typed request/response.
     *
     *             This method provides a high-level interface for making HTTP POST requests to
     *             other services using configuration-based routing. It automatically constructs
     *             the target URL from configuration parameters and handles the complete request
     *             lifecycle with automatic serialization and response deserialization.
     *
     *             The method looks up the target service configuration using the route parameter
     *             and constructs the full URL as "http://ip:port" + path. This enables
     *             service-to-service communication without hardcoding endpoints, making the
     *             system more maintainable and configurable.
     *
     *             Request Flow:
     *             1. Looks up service endpoint from configuration using route name
     *             2. Constructs full URL from configuration IP and port
     *             3. Serializes the request body using FlatBuffer protocol
     *             4. Sends HTTP POST request with binary payload
     *             5. Receives and deserializes the response
     *             6. Returns strongly-typed response object
     *
     *             Configuration format expected:
     *             ```json
     *             {
     *               "route_name": {
     *                 "ip": "service.hostname.com",
     *                 "port": 8080
     *               }
     *             }
     *             ```
     *
     *             This method is commonly used for:
     *             - User authentication requests to auth service
     *             - Database operations via data service
     *             - Inter-service state synchronization
     *             - Distributed transaction coordination
     *
     * @param[in]  route     The configuration route name to look up service endpoint details.
     * @param[in]  path      The target path to append to the service base URL.
     * @param[in]  body      The request object to serialize and send to the target service.
     *
     * @tparam     Request   The FlatBuffer request type that implements Serialize() method.
     * @tparam     Response  The FlatBuffer response type expected from the target service.
     *
     * @return     An async task that completes with the deserialized response object.
     *
     * @throws     std::exception if route configuration is missing or request fails.
     */
    template <typename Request, typename Response>
    [[nodiscard]] async::task<Response> post(const std::string& route, const std::string& path, const Request& body)
    {
        auto& config = fb::config<>(route);
        auto  host   = std::format("http://{}:{}", config["ip"].asCString(), config["port"].asUInt());
        co_return co_await this->boost_post_async<Request, Response>(host, path, body);
    }

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

                if (this->assert_tps(socket) && socket.update_tps(MAX_TPS) == false)
                    throw std::runtime_error("tps limit exceeded");

                auto cmd = reader.read<uint8_t>();
                if (this->decrypt_policy(cmd))
                    size = socket.crt().decrypt(stream, reader.seek() - 1, size);

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
                    auto fd = socket.fd();
                    this->threads.enqueue(socket,
                                          [this, protocol, &socket, fd, cmd](auto& thread) -> async::task<void> {
                                              try
                                              {
                                                  if (this->connected(fd) == false)
                                                      co_return;

                                                  auto& handler = this->handler.protocol.get_handler(cmd);
                                                  if (this->assert_tps(socket) && handler.update_tps() == false)
                                                      co_return;

                                                  std::ignore = co_await handler.fn(socket, *protocol.get());
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
        this->pop_alive(socket);

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

            co_await this->threads.dispatch(socket, [this, &socket](auto&) -> async::task<void> {
                co_await this->erase(socket);
            });
        }
        catch (std::exception& e)
        {
            fb::logger::fatal(e.what());
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
        auto socket = std::make_unique<fb::socket<T>>(*this,
                                                      std::bind_front(&acceptor::on_socket_received, this),
                                                      std::bind_front(&acceptor::on_socket_closed, this));
        auto ptr    = socket.get();
        this->async_accept(*ptr, [this, socket = std::move(socket), ptr](boost::system::error_code error) mutable {
            try
            {
                if (error)
                    throw std::runtime_error(error.message());

                if (this->_running == false)
                    throw std::runtime_error("cannot accept socket. acceptor is cleaning now.");

                ptr->data(this->handle_accepted(*ptr));

                {
                    auto fd = socket->fd();
                    this->_sockets.write([fd, &socket](auto& v) -> void {
                        if (v.contains(fd))
                        {
                            fb::logger::warn(std::format("socket already exists. fd: {}", fd));
                            v.erase(fd); // remove old socket if exists
                        }

                        v.insert({fd, std::move(socket)});
                    });
                }

                this->push_alive(*ptr);
                async::awaitable_get(this->handle_connected(*ptr));

                boost::asio::co_spawn(*this, ptr->recv(), boost::asio::detached);
                this->accept();
            }
            catch (std::exception& e)
            {
                fb::logger::fatal(e.what());
                socket->close();
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
     *             - Key size (1 byte, always crypto::KEY_SIZE)
     *             - Encryption key (crypto::KEY_SIZE bytes)
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
        auto& crt    = socket.crt();
        auto  params = fb::stream();
        {
            auto writer = fb::stream_writer<big_endian>(params);
            writer.write<uint8_t>(crt.type());
            writer.write<uint8_t>(crypto::KEY_SIZE);
            writer.write(crt.key(), crypto::KEY_SIZE);
            writer.write<uint8_t>(static_cast<uint8_t>(from));
        }

        auto stream = fb::stream();
        {
            auto writer = fb::stream_writer<big_endian>(stream);
            co_await fb::protocol::response::transfer(ip, port, params).serialize(writer);
        }

        crt.wrap(stream);
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
     *             - Key size (1 byte, always crypto::KEY_SIZE)
     *             - Encryption key (crypto::KEY_SIZE bytes)
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
        auto& crt    = socket.crt();
        auto  header = fb::stream();
        {
            auto writer = fb::stream_writer<big_endian>(header);
            writer.write<uint8_t>(crt.type());
            writer.write<uint8_t>(crypto::KEY_SIZE);
            writer.write(crt.key(), crypto::KEY_SIZE);
            writer.write<uint8_t>(static_cast<uint8_t>(from));
            writer.write<fb::stream>(parameter);
        }

        auto stream = fb::stream();
        {
            auto writer = fb::stream_writer<big_endian>(stream);
            co_await fb::protocol::response::transfer(ip, port, header).serialize(writer);
        }

        crt.wrap(stream);
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
    virtual T* handle_accepted(fb::socket<T>& socket) = 0;

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
                this->_boost_context.run();
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

protected:
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
                    co_await this->handle_disconnected(*socket);
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