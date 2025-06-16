#ifndef __FB_ACCEPTOR_H__
#define __FB_ACCEPTOR_H__

#include <ctime>
#include <fb/mutex.h>
#include <fb/protocol/flatbuffer/protocol.h>
#include <fb/protocol/transfer.h>
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
 * @brief      This class describes an acceptor.
 *
 * @tparam     T     { description }
 */
template <typename T>
class acceptor : public fb::acceptable
{
public:
    using handle_func           = std::function<async::task<bool>(fb::socket<T>&, fb::protocol::header&)>;
    using deserilze_func        = std::function<async::task<fb::protocol::header*>(fb::stream_reader<big_endian>&)>;
    using socket_container      = std::unordered_map<uint32_t, std::unique_ptr<fb::socket<T>>>;
    using socket_container_lock = fb::locker<socket_container>;
    using amqp_handler_func     = std::function<async::task<void>(const uint8_t*)>;
    using amqp_handler_type     = std::unordered_map<std::string, std::unordered_map<uint32_t, amqp_handler_func>>;
    using boost_timers          = std::vector<std::shared_ptr<boost::asio::deadline_timer>>;

private:
    struct handler
    {
    private:
        fb::model::datetime last        = fb::model::datetime();
        uint32_t            transitions = 0;

    public:
        const handle_func                         fn;
        const std::chrono::steady_clock::duration duration = 1s;
        const uint32_t                            limit    = 0xFFFFFFFF;

    public:
        handler() = default;
        handler(const handle_func&                         fn,
                const std::chrono::steady_clock::duration& duration,
                uint32_t                                   limit = 0xFFFFFFFF) :
            fn(fn),
            duration(duration),
            limit(limit)
        { }

        bool update_tps()
        {
            auto elapsed_time = fb::model::datetime() - this->last;
            if (elapsed_time > duration)
            {
                this->last        = fb::model::datetime();
                this->transitions = 0;
            }

            if (++this->transitions > this->limit)
                return false;

            return true;
        }
    };

private:
    std::unordered_map<uint8_t, handler>        _handler;
    std::unordered_map<uint8_t, deserilze_func> _deserializer;
    amqp_handler_type                           _amqp_handler;
    std::unique_ptr<fb::amqp::socket>           _amqp;
    std::mutex                                  _mutex_exit;
    boost_timers                                _timers;

protected:
    fb::mutex             _mutex;
    socket_container_lock _sockets;

protected:
    /**
     * @brief      Constructs a new instance.
     *
     * @param      context  The context
     * @param[in]  name     The name
     * @param[in]  port     The port
     */
    acceptor(boost::asio::io_context& context, const std::string& name, uint16_t port) :
        fb::acceptable(context, name, config<uint32_t>("thread:logic"), port),
        _mutex(*this)
    { }

public:
    /**
     * @brief      Destroys the object.
     */
    virtual ~acceptor()
    {
        this->exit();
    }

protected:
    virtual void handle_declare_amqp_queue(fb::amqp::socket& amqp) = 0;

protected:
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
     * @brief      { function_description }
     */
    void amqp_thread_loop()
    {
        auto timeout = timeval{5, 0};
        while (this->_running)
        {
            try
            {
                this->_amqp = std::make_unique<fb::amqp::socket>();
                this->_amqp->connect(fb::config<std::string>("amqp:ip"),
                                     fb::config<uint16_t>("amqp:port"),
                                     fb::config<std::string>("amqp:uid"),
                                     fb::config<std::string>("amqp:pwd"),
                                     "/");

                this->handle_declare_amqp_queue(*this->_amqp);
            }
            catch (std::exception& e)
            {
                fb::logger::fatal(e.what());
                std::this_thread::sleep_for(1s);
                continue;
            }

            while (this->_running)
            {
                try
                {
                    if (this->_amqp->select(&timeout) == false)
                        continue;
                }
                catch (std::exception&)
                {
                    break;
                }
            }
        }
    }

    /**
     * @brief      GET request to the host and return the response bytes
     *
     * @param[in]  host      The host (hostname:port)
     * @param[in]  path      The path (target)
     * @param[in]  headers   The headers
     * @param[in]  timeout   The timeout
     *
     * @return     The response bytes
     */
    boost::asio::awaitable<std::vector<uint8_t>> boost_get_async(std::string                         host,
                                                                 std::string                         path,
                                                                 std::map<std::string, std::string>  headers,
                                                                 std::chrono::steady_clock::duration timeout)
    {
        auto raw_host = host;
        if (raw_host.rfind("http://", 0) == 0)
            raw_host.erase(0, 7);
        else if (raw_host.rfind("https://", 0) == 0)
            raw_host.erase(0, 8);

        auto const colon_pos = raw_host.find(':');
        auto const host_name = (colon_pos == std::string::npos ? raw_host : raw_host.substr(0, colon_pos));
        auto const port      = (colon_pos == std::string::npos ? std::string("80") : raw_host.substr(colon_pos + 1));

        auto resolver = boost::asio::ip::tcp::resolver{this->_boost_context};
        auto stream   = boost::beast::tcp_stream{this->_boost_context};

        stream.expires_after(timeout);
        auto const results = co_await resolver.async_resolve(host_name, port, boost::asio::use_awaitable);
        co_await stream.async_connect(results, boost::asio::use_awaitable);

        auto req =
            boost::beast::http::request<boost::beast::http::empty_body>{boost::beast::http::verb::get,
                                                                        url_encode(UTF8(path, PLATFORM::Windows)),
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

private:
    /**
     * @brief      GET request to the host and return the response
     *
     * @param[in]  host      The host (hostname:port)
     * @param[in]  path      The path (target)
     *
     * @tparam     Response  The type of the response
     *
     * @return     The response bytes
     */
    template <typename Response>
    [[nodiscard]] async::task<Response> boost_get_async(const std::string& host, const std::string& path)
    {
        auto promise = std::make_shared<async::task_completion_source<Response>>();
        auto headers = std::map<std::string, std::string>{
            {"Content-Type", "application/octet-stream"},
        };

        boost::asio::co_spawn(this->_boost_context,
                              this->boost_get_async(host, path, headers, 5s),
                              [this, promise](std::exception_ptr ep, std::vector<uint8_t> bytes) {
                                  if (ep)
                                  {
                                      try
                                      {
                                          std::rethrow_exception(ep);
                                      }
                                      catch (std::exception const& e)
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

        return promise->task();
    }

public:
    /**
     * @brief      GET request to the host and return the response bytes
     *
     * @param[in]  route     The route
     * @param[in]  path      The path
     *
     * @tparam     Response  The type of the response
     *
     * @return     The response bytes
     */
    template <typename Response>
    [[nodiscard]] async::task<Response> get(const std::string& route, const std::string& path)
    {
        auto& config = fb::config<>(route);
        auto  host   = std::format("http://{}:{}", config["ip"].asCString(), config["port"].asUInt());
        co_return co_await this->boost_get_async<Response>(host, path);
    }

private:
    /**
     * @brief      POST request to the host and return the response bytes
     *
     * @param[in]  host     The host (hostname:port)
     * @param[in]  path     The path (target)
     * @param[in]  headers  The headers
     * @param[in]  timeout  The timeout
     * @param[in]  body     The body
     *
     * @return     The response bytes
     */
    boost::asio::awaitable<std::vector<uint8_t>> boost_post_async(std::string                         host,
                                                                  std::string                         path,
                                                                  std::map<std::string, std::string>  headers,
                                                                  std::chrono::steady_clock::duration timeout,
                                                                  std::vector<uint8_t>                body)
    {
        auto raw_host = host;
        if (raw_host.rfind("http://", 0) == 0)
            raw_host.erase(0, 7);
        else if (raw_host.rfind("https://", 0) == 0)
            raw_host.erase(0, 8);

        auto const colon_pos = raw_host.find(':');
        auto const host_name = (colon_pos == std::string::npos ? raw_host : raw_host.substr(0, colon_pos));
        auto const port      = (colon_pos == std::string::npos ? std::string("80") : raw_host.substr(colon_pos + 1));

        auto resolver = boost::asio::ip::tcp::resolver{this->_boost_context};
        auto stream   = boost::beast::tcp_stream{this->_boost_context};

        stream.expires_after(timeout);
        auto const results = co_await resolver.async_resolve(host_name, port, boost::asio::use_awaitable);
        co_await stream.async_connect(results, boost::asio::use_awaitable);

        auto req = boost::beast::http::request<boost::beast::http::vector_body<uint8_t>>{
            boost::beast::http::verb::post,
            url_encode(UTF8(path, PLATFORM::Windows)),
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

private:
    /**
     * @brief      POST request to the host and return the response
     *
     * @param[in]  host     The host (hostname:port)
     * @param[in]  path     The path (target)
     * @param[in]  body     The body
     *
     * @tparam     Request   The type of the request
     * @tparam     Response  The type of the response
     *
     * @return     The response
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

        boost::asio::co_spawn(this->_boost_context,
                              this->boost_post_async(host, path, headers, std::chrono::seconds{5}, stream_req),
                              [promise](std::exception_ptr ep, std::vector<uint8_t> bytes) {
                                  if (ep)
                                  {
                                      promise->set_exception(ep);
                                      return;
                                  }
                                  try
                                  {
                                      fb::stream_reader<big_endian> reader(bytes);
                                      auto                          protocol_type = reader.read<uint32_t>();
                                      auto                          protocol_len  = reader.read<uint32_t>();
                                      auto                          offset        = bytes.data() + sizeof(uint32_t) * 2;

                                      promise->set_value(Response::Deserialize(offset));
                                  }
                                  catch (...)
                                  {
                                      promise->set_exception(std::current_exception());
                                  }
                              });

        return promise->task();
    }

public:
    /**
     * @brief      { function_description }
     *
     * @param[in]  route     The route
     * @param[in]  path      The path
     * @param[in]  body      The body
     *
     * @tparam     Request   { description }
     * @tparam     Response  { description }
     *
     * @return     { description_of_the_return_value }
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
     * @brief      { function_description }
     *
     * @param[in]  fd    { parameter_description }
     *
     * @return     { description_of_the_return_value }
     */
    bool connected(uint32_t fd)
    {
        return this->_sockets.template read<bool>([fd](const auto& v) -> bool {
            return v.contains(fd);
        });
    }

private:
    /**
     * @brief      This method must be call by i/o thread.
     *
     * @param      socket  The socket
     * @param      stream  The stream
     *
     * @return     { description_of_the_return_value }
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

                if (this->_deserializer.contains(cmd) == false)
                {
                    fb::logger::warn(std::format("정의되지 않은 프로토콜입니다. [{:#x}]", cmd));
                }
                else if (this->_handler.contains(cmd) == false)
                {
                    fb::logger::warn(std::format("정의되지 않은 핸들러입니다. [{:#x}]", cmd));
                }
                else
                {
                    auto protocol = std::shared_ptr<fb::protocol::header>(co_await this->_deserializer[cmd](reader));
                    auto fd       = socket.fd();
                    this->threads.enqueue(socket,
                                          [this, protocol, &socket, fd, cmd](auto& thread) -> async::task<void> {
                                              try
                                              {
                                                  if (this->connected(fd) == false)
                                                      co_return;

                                                  auto& handler = this->_handler[cmd];
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
     * @brief      { function_description }
     *
     * @param      socket  The socket
     *
     * @return     { description_of_the_return_value }
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
     * @brief      Called when socket received.
     *
     * @param      socket  The socket
     * @param      stream  The stream
     *
     * @return     { description_of_the_return_value }
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
     * @brief      Called when socket closed.
     *
     * @param      socket  The socket
     *
     * @return     { description_of_the_return_value }
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
     * @brief      { function_description }
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
     * @brief      { function_description }
     *
     * @param      socket  The socket
     * @param[in]  ip      { parameter_description }
     * @param[in]  port    The port
     * @param[in]  from    The from
     *
     * @return     { description_of_the_return_value }
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
     * @brief      { function_description }
     *
     * @param      socket  The socket
     * @param[in]  ip      { parameter_description }
     * @param[in]  port    The port
     * @param[in]  from    The from
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<void>
    transfer(fb::socket<T>& socket, const std::string& ip, uint16_t port, fb::protocol::internal::Service from)
    {
        co_await this->transfer(socket, inet_addr(this->ipv4(ip).c_str()), port, from);
    }

public:
    /**
     * @brief      { function_description }
     *
     * @param      socket     The socket
     * @param[in]  ip         { parameter_description }
     * @param[in]  port       The port
     * @param[in]  from       The from
     * @param[in]  parameter  The parameter
     *
     * @return     { description_of_the_return_value }
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
     * @brief      { function_description }
     *
     * @param      socket     The socket
     * @param[in]  ip         { parameter_description }
     * @param[in]  port       The port
     * @param[in]  from       The from
     * @param[in]  parameter  The parameter
     *
     * @return     { description_of_the_return_value }
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
     * @brief      { function_description }
     *
     * @param[in]  cmd   The command
     *
     * @return     { description_of_the_return_value }
     */
    virtual bool decrypt_policy(uint8_t cmd) const
    {
        return true;
    }

    /**
     * @brief      { function_description }
     *
     * @param[in]  socket  The socket
     *
     * @return     { description_of_the_return_value }
     */
    virtual bool assert_tps(const fb::socket<T>& socket) const
    {
        return true;
    }

protected:
    /**
     * @brief      { function_description }
     *
     * @param      socket  The socket
     *
     * @return     { description_of_the_return_value }
     */
    virtual T* handle_accepted(fb::socket<T>& socket) = 0;

protected:
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
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
     * @brief      { function_description }
     *
     * @param      session  The session
     *
     * @return     { description_of_the_return_value }
     */
    virtual async::task<bool> handle_connected(fb::socket<T>& session)
    {
        co_return true;
    }

protected:
    /**
     * @brief      { function_description }
     *
     * @param      session  The session
     *
     * @return     { description_of_the_return_value }
     */
    virtual async::task<bool> handle_disconnected(fb::socket<T>& session)
    {
        co_return true;
    }

protected:
    /**
     * @brief      { function_description }
     *
     * @param[in]  socket  The socket
     *
     * @return     { description_of_the_return_value }
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
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    virtual async::task<void> handle_exit()
    {
        co_return;
    }

protected:
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    virtual uint8_t id() const
    {
        return fb::config<uint8_t>("id");
    }

protected:
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    virtual std::string name() const
    {
        return fb::config<std::string>("name");
    }

protected:
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    virtual fb::protocol::internal::Service service() const = 0;

protected:
    /**
     * @brief      { function_description }
     *
     * @param[in]  <unnamed>    { parameter_description }
     * @param[in]  header       The header
     *
     * @tparam     Class        { description }
     * @tparam     RequestType  { description }
     */
    template <typename Class, typename RequestType>
    void bind(async::task<bool> (Class::*fn)(fb::socket<T>&, const RequestType&),
              uint8_t                                    header,
              const std::chrono::steady_clock::duration& duration = 1s,
              uint32_t                                   limit    = 10)
    {
        this->_deserializer.insert({header, [](auto& reader) -> async::task<fb::protocol::header*> {
                                        auto protocol = new RequestType();
                                        co_await protocol->deserialize(reader);
                                        co_return protocol;
                                    }});

        auto func_bound = std::bind(fn, static_cast<Class*>(this), std::placeholders::_1, std::placeholders::_2);
        auto handler_fn = [func_bound](auto& socket, auto& header) -> async::task<bool> {
            auto protocol = static_cast<RequestType&>(header);
            co_return co_await func_bound(socket, protocol);
        };

        this->_handler.insert({header, handler(handler_fn, duration, limit)});
    }

protected:
    /**
     * @brief      { function_description }
     *
     * @param[in]  route         The route
     * @param[in]  <unnamed>     { parameter_description }
     *
     * @tparam     Class         { description }
     * @tparam     ResponseType  { description }
     */
    template <typename Class, typename ResponseType>
    void bind_amqp(const std::string& route, async::task<void> (Class::*fn)(const ResponseType&))
    {
        if (this->_amqp_handler.contains(route) == false)
            this->_amqp_handler.insert({route, std::unordered_map<uint32_t, amqp_handler_func>{}});

        auto c_func = std::bind(fn, static_cast<Class*>(this), std::placeholders::_1);
        auto cmd    = static_cast<uint32_t>(ResponseType::FlatBufferProtocolType);
        this->_amqp_handler[route].insert({cmd, [c_func](const uint8_t* ptr) -> async::task<void> {
                                               auto protocol = ResponseType::Deserialize(ptr);
                                               co_await c_func(protocol);
                                           }});
    }

    /**
     * @brief      { function_description }
     *
     * @param      queue  The queue
     */
    void bind_amqp(fb::amqp::queue& queue)
    {
        auto& route = queue.route();
        if (this->_amqp_handler.contains(route))
        {
            for (auto& [cmd, fn] : this->_amqp_handler.at(route))
            {
                queue.handler(cmd, fn);
            }
        }
    }

protected:
    /**
     * @brief      { function_description }
     *
     * @param[in]  <unnamed>  { parameter_description }
     *
     * @tparam     Class      { description }
     * @tparam     Request    { description }
     */
    template <typename Class, typename Request>
    void bind(async::task<bool> (Class::*fn)(fb::socket<T>&, const Request&),
              const std::chrono::steady_clock::duration& duration = 1s,
              uint32_t                                   limit    = 10)
    {
        this->bind(fn, Request::header, duration, limit);
    }

public:
    /**
     * @brief      { function_description }
     *
     * @param      socket   The socket
     * @param[in]  stream   The stream
     * @param[in]  encrypt  The encrypt
     * @param[in]  wrap     The wrap
     *
     * @return     { description_of_the_return_value }
     */
    async::task<size_t> send(fb::socket<T>& socket, const fb::stream& stream, bool encrypt = true, bool wrap = true)
    {
        if (stream.empty())
            co_return 0;

        co_return co_await socket.send(stream, encrypt, wrap);
    }

public:
    /**
     * @brief      { function_description }
     *
     * @param      socket    The socket
     * @param[in]  response  The response
     * @param[in]  encrypt   The encrypt
     * @param[in]  wrap      The wrap
     *
     * @return     { description_of_the_return_value }
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
     * @brief      { function_description }
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
            this->amqp_thread_loop();
        }));

        for (auto& thread : threads)
        {
            thread.join();
        }
    }

public:
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    bool running() const
    {
        return this->_running;
    }

protected:
    /**
     * @brief      { function_description }
     *
     * @param[in]  duration  The duration
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<void> sleep(const fb::model::timespan& duration)
    {
        auto thread = this->threads.current();
        if (thread != nullptr)
            co_await thread->sleep(duration);
    }

private:
    async::task<void> disconnect_sockets()
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
    void access_sockets(std::function<void(const socket_container&)> fn)
    {
        this->_sockets.read([fn](const auto& v) {
            fn(v);
        });
    }

public:
    /**
     * @brief      { function_description }
     */
    void exit() override final
    {
        // abstract에 있는 exit와 겹치는 코드가 굉장히 많고
        // 멤버필드의 위치도 애매함. 리팩토링 필요함
        if (this->_running == false)
            return;

        this->_running = false; // 백그라운드 스레드 종료
        this->cancel();         // async_accept 취소
        async::awaitable_get(this->disconnect_sockets());

        for (auto& timer : this->_timers)
        {
            timer->cancel();
        }

        this->threads.exit(); // 로직스레드 종료
        this->close();        // io 스레드 종료

        static_cast<boost::asio::io_context&>(*this).stop();
    }
};

} // namespace fb

#endif // !__FB_ACCEPTOR_H__