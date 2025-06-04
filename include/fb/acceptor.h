#ifndef __FB_ACCEPTOR_H__
#define __FB_ACCEPTOR_H__

#include <ctime>
#include <fb/mutex.h>
#include <fb/protocol/flatbuffer/protocol.h>
#include <fb/protocol/transfer.h>
#include <fb/socket.h>
#include <httplib.h>
#include <iomanip>
#include <fb/amqp.h>
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
    using background_func       = std::function<async::task<void>()>;
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
    std::queue<background_func> _background_queue;
    std::mutex                  _background_queue_mutex;

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
     * @brief      Gets the internal.
     *
     * @param[in]  host     The host
     * @param[in]  path     The path
     * @param[in]  headers  The headers
     *
     * @return     The internal.
     */
    [[nodiscard]] async::task<httplib::Result> get_internal(const std::string& host,
                                                            const std::string& path,
                                                            httplib::Headers   headers)
    {
        headers.insert({"Content-Type", "application/octet-stream"});

        co_return co_await this->background<httplib::Result>([=, this]() -> async::task<httplib::Result> {
            auto client = httplib::Client(host);
            client.set_connection_timeout(5, 0);
            client.set_read_timeout(5, 0);
            client.set_write_timeout(5, 0);
            co_return client.Get(UTF8(path, PLATFORM::Windows), headers);
        });
    }

private:
    /**
     * @brief      Gets the internal.
     *
     * @param[in]  host      The host
     * @param[in]  path      The path
     *
     * @tparam     Response  { description }
     *
     * @return     The internal.
     */
    template <typename Response>
    [[nodiscard]] async::task<Response> get_internal(const std::string& host, const std::string& path)
    {
        auto   headers = httplib::Headers();
        auto&& res     = co_await this->get_internal(host, path, headers);
        if (!res)
            throw std::runtime_error(std::format("cannot request to http server : {}", host));

        if (res->status != 200)
            throw std::runtime_error(std::format("http server response status code {}", res->status));

        auto ptr    = (const uint8_t*)res->body.c_str();
        auto size   = std::stoi(res->get_header_value("Content-Length"));
        auto stream = fb::stream(ptr, size);
        auto reader = fb::stream_reader<big_endian>(stream);

        auto protocol_type = reader.read<uint32_t>();
        auto protocol_size = reader.read<uint32_t>();
        auto offset        = ptr + sizeof(uint32_t) + sizeof(uint32_t);
        co_return Response::Deserialize(offset);
    }

public:
    /**
     * @brief      { function_description }
     *
     * @param[in]  route     The route
     * @param[in]  path      The path
     *
     * @tparam     Response  { description }
     *
     * @return     { description_of_the_return_value }
     */
    template <typename Response>
    [[nodiscard]] async::task<Response> get(const std::string& route, const std::string& path)
    {
        auto& config = fb::config<>(route);
        auto  host   = std::format("http://{}:{}", config["ip"].asCString(), config["port"].asUInt());
        co_return co_await this->get_internal<Response>(host, path);
    }

private:
    /**
     * @brief      Posts an internal.
     *
     * @param[in]  host     The host
     * @param[in]  path     The path
     * @param[in]  headers  The headers
     * @param[in]  bytes    The bytes
     * @param[in]  size     The size
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<httplib::Result> post_internal(const std::string& host,
                                                             const std::string& path,
                                                             httplib::Headers   headers,
                                                             const void*        bytes,
                                                             size_t             size)
    {
        auto promise = std::make_shared<async::task_completion_source<httplib::Result>>();
        auto buffer  = std::vector<uint8_t>(size);
        std::memcpy(buffer.data(), bytes, size);
        co_return co_await this->background<httplib::Result>([=, this]() -> async::task<httplib::Result> {
            auto client = httplib::Client(host);
            client.set_connection_timeout(5, 0);
            client.set_read_timeout(5, 0);
            client.set_write_timeout(5, 0);
            co_return client.Post(UTF8(path, PLATFORM::Windows),
                                  headers,
                                  (const char*)buffer.data(),
                                  buffer.size(),
                                  "application/octet-stream");
        });
    }

private:
    /**
     * @brief      Posts an internal.
     *
     * @param[in]  host      The host
     * @param[in]  path      The path
     * @param[in]  body      The body
     *
     * @tparam     Request   { description }
     * @tparam     Response  { description }
     *
     * @return     { description_of_the_return_value }
     */
    template <typename Request, typename Response>
    [[nodiscard]] async::task<Response> post_internal(const std::string& host,
                                                      const std::string& path,
                                                      const Request&     body)
    {
        auto headers    = httplib::Headers();
        auto serialized = body.Serialize();
        auto stream_req = fb::stream();
        auto writer     = fb::stream_writer<>(stream_req);
        writer.write<uint32_t>(static_cast<uint32_t>(Request::FlatBufferProtocolType));
        writer.write<uint32_t>(serialized.size());
        writer.write((const void*)serialized.data(), serialized.size());

        auto&& res = co_await this->post_internal(host, path, headers, stream_req.data(), stream_req.size());
        if (!res)
            throw std::runtime_error(std::format("cannot request to http server : {}", host));

        if (res->status != 200)
            throw std::runtime_error(std::format("http server response status code {}", res->status));

        auto ptr        = (const uint8_t*)res->body.c_str();
        auto size       = std::stoi(res->get_header_value("Content-Length"));
        auto stream_res = fb::stream(ptr, size);
        auto reader     = fb::stream_reader<>(stream_res);

        auto protocol_type = reader.read<uint32_t>();
        auto protocol_size = reader.read<uint32_t>();
        co_return Response::Deserialize(ptr + sizeof(uint32_t) + sizeof(uint32_t));
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
        co_return co_await this->post_internal<Request, Response>(host, path, body);
    }

public:
    using dispatch_callback = std::function<async::task<void>(void)>;
    /**
     * @brief      { function_description }
     *
     * @param      socket    The socket
     * @param[in]  fn        The function
     * @param[in]  priority  The priority
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<void> dispatch(fb::socket<T>& socket, const dispatch_callback& fn, uint32_t priority = 0)
    {
        auto id     = this->thread_id(socket);
        auto thread = this->threads[id];

        if (thread == nullptr)
            throw std::runtime_error("thread does not exists");

        co_await this->threads[id]->dispatch(fn, 0s, priority);
    }

    /**
     * @brief      { function_description }
     *
     * @param[in]  fd    { parameter_description }
     *
     * @return     { description_of_the_return_value }
     */
    bool connected(uint32_t fd)
    {
        return this->_sockets.template lock<bool>([fd](const auto& v) -> bool {
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
        this->_sockets.lock([fd](auto& v) -> void {
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
                    this->_sockets.lock([fd, &socket](auto& v) -> void {
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

protected:
    /**
     * @brief      { function_description }
     */
    virtual void handle_background()
    {
        while (this->_running || !this->_background_queue.empty())
        {
            background_func func;
            {
                auto _ = std::lock_guard(this->_background_queue_mutex);
                if (this->_background_queue.size() > 0)
                {
                    func = this->_background_queue.front();
                    this->_background_queue.pop();
                }
            }

            if (func)
                async::awaitable_get(func());
            else
                std::this_thread::sleep_for(100ms);
        }
    }

    /**
     * @brief      { function_description }
     *
     * @param[in]  func  The function
     *
     * @tparam     R     { description }
     *
     * @return     { description_of_the_return_value }
     */
    template <typename R>
    [[nodiscard]] async::task<R> background(const std::function<async::task<R>()>& func)
    {
        auto _       = std::lock_guard(this->_background_queue_mutex);
        auto promise = std::make_shared<async::task_completion_source<R>>();
        auto thread  = this->threads.current();
        this->_background_queue.push([this, promise, thread, func]() -> async::task<void> {
            try
            {
                if constexpr (std::is_void_v<R>)
                {
                    co_await func();
                    if (thread != nullptr)
                        co_await thread->switching();
                    promise->set_value();
                }
                else
                {
                    R result = co_await func();
                    if (thread != nullptr)
                        co_await thread->switching();
                    promise->set_value(std::move(result));
                }
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
     * @brief      { function_description }
     */
    void run()
    {
        this->_running = true;
        this->accept();

        auto threads = std::vector<std::thread>();
        for (int i = 0; i < fb::config<uint32_t>("thread:background"); i++)
        {
            threads.push_back(std::thread([this]() {
                this->handle_background();
            }));
        }

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
        this->_sockets.lock([&pairs](const auto& v) -> void {
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
        this->_sockets.lock([fn](auto& v) {
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