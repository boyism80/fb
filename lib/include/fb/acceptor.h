#ifndef __FB_ACCEPTOR_H__
#define __FB_ACCEPTOR_H__

#include <async/awaitable_get.h>
#include <async/task.h>
#include <async/task_completion_source.h>
#include <chrono>
#include <ctime>
#include <fb/abstract.h>
#include <fb/logger.h>
#include <fb/model/datetime.h>
#include <fb/mutex.h>
#include <fb/protocol/flatbuffer/protocol.h>
#include <fb/protocol/protocol.h>
#include <fb/redis.h>
#include <fb/socket.h>
#include <fb/stream.h>
#include <functional>
#include <httplib.h>
#include <iomanip>
#include <map>
#include <queue>
#include <zlib.h>

using namespace std::chrono_literals;

namespace fb {

/**
 * @brief      This class describes an acceptor.
 *
 * @tparam     T     { description }
 */
template <typename T>
class acceptor : public fb::context
{
private:
    using handle_func     = std::function<async::task<bool>(fb::socket<T>&, fb::protocol::base::header&)>;
    using deserilze_func  = std::function<async::task<fb::protocol::base::header*>(fb::stream_reader<big_endian>&)>;
    using background_func = std::function<async::task<void>()>;

private:
    std::unordered_map<uint8_t, handle_func>    _handler;
    std::unordered_map<uint8_t, deserilze_func> _deserializer;
    std::mutex                                  _mutex_exit;
    bool                                        _running = false;

protected:
    std::queue<background_func> _background_queue;
    std::mutex                  _background_queue_mutex;

protected:
    fb::redis _redis;
    fb::mutex _mutex;

public:
    fb::socket_container<T> sockets;

protected:
    /**
     * @brief      Constructs a new instance.
     *
     * @param      context  The context
     * @param[in]  port     The port
     */
    acceptor(boost::asio::io_context& context, const std::string& name, uint16_t port) :
        fb::context(context, name, port),
        _redis(*this, fb::config<std::string>("redis:default:ip"), fb::config<uint16_t>("redis:default:port")),
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

private:
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

private:
    /**
     * @brief      This method must be call by i/o thread.
     *
     * @param      socket  The socket
     * @param      stream  The stream
     */
    async::task<void> execute_handler(fb::socket<T>& socket, fb::stream& stream)
    {
        static constexpr uint8_t base_size = sizeof(uint8_t) + sizeof(uint16_t);

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
                if (reader.readable_size() < size)
                {
                    reader.seek(0);
                    co_return;
                }

                auto cmd = reader.read<uint8_t>();
                if (this->decrypt_policy(cmd))
                    size = socket.crt().decrypt(stream, reader.seek() - 1, size);

                reader.flush(); // remove magic code and size

                if (this->_deserializer.contains(cmd) == false)
                {
                    fb::logger::fatal(std::format("정의되지 않은 프로토콜입니다. [{:#x}]", cmd));
                }
                else if (this->_handler.contains(cmd) == false)
                {
                    fb::logger::fatal(std::format("정의되지 않은 핸들러입니다. [{:#x}]", cmd));
                }
                else
                {
                    auto protocol =
                        std::shared_ptr<fb::protocol::base::header>(co_await this->_deserializer[cmd](reader));
                    this->threads.enqueue(
                        socket,                                        // pivot
                        [this, protocol, fd = socket.fd()]() -> bool { // condition
                            if (this->sockets.contains(fd))
                                return true;

                            return false;
                        },
                        [this, cmd, &socket, protocol](auto&) -> async::task<void> { // fn
                            std::ignore = co_await this->_handler[cmd](socket, *protocol.get());
                        },
                        [](auto& error) { // error
                            fb::logger::fatal(error.what());
                        },
                        []() { // success

                        });
                }

                reader.seek(size - sizeof(uint8_t));
                reader.flush(); // remove packet body
            }
        }
        catch (std::exception& e)
        {
            fb::logger::fatal(e.what());
            reader.seek(0);
            reader.clear();
        }
        catch (...)
        {
            fb::logger::fatal("unhandled exception while parse packet");
            reader.seek(0);
            reader.clear();
        }
    }

private:
    /**
     * @brief      { function_description }
     */
    void accept()
    {
        auto callback_received = [this](fb::socket<T>& socket, fb::stream& stream) -> async::task<void> {
            try
            {
                co_await this->execute_handler(socket, stream);
            }
            catch (std::exception& e)
            {
                fb::logger::fatal(e.what());
            }
        };

        auto callback_closed = [this](fb::socket<T>& socket) -> async::task<void> {
            try
            {
                if (socket.data() == nullptr)
                    co_return;

                auto& casted = static_cast<fb::socket<T>&>(socket);
                co_await this->threads.switching(casted);
                std::ignore = co_await this->handle_disconnected(casted);
                this->sockets.erase(casted);
            }
            catch (std::exception& e)
            {
                fb::logger::fatal(e.what());
            }
        };

        auto socket = std::make_unique<fb::socket<T>>(*this, callback_received, callback_closed);
        auto ptr    = socket.get();
        this->async_accept(*ptr, [this, socket = std::move(socket), ptr](boost::system::error_code error) mutable {
            try
            {
                if (error)
                    throw std::runtime_error(error.message());

                if (this->_running == false)
                    throw std::runtime_error("cannot accept socket. acceptor is cleaning now.");

                ptr->data(this->handle_accepted(*ptr));

                this->sockets.push(std::move(socket));
                async::awaitable_get(this->handle_connected(*ptr));

                boost::asio::co_spawn(*this, ptr->recv(), boost::asio::detached);
                this->accept();
            }
            catch (std::exception& e)
            {
                fb::logger::fatal(e.what());
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
            writer.write<uint8_t>(cryptor::KEY_SIZE);
            writer.write(crt.key(), cryptor::KEY_SIZE);
            writer.write<uint8_t>(static_cast<uint8_t>(from));
        }

        auto stream = fb::stream();
        {
            auto writer = fb::stream_writer<big_endian>(stream);
            co_await fb::protocol::response::transfer(ip, port, params).serialize(writer);
        }

        crt.wrap(stream);
        co_await socket.send(stream, false, false);
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
        co_await this->transfer(socket, inet_addr(ip.c_str()), port, from);
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
            writer.write<uint8_t>(cryptor::KEY_SIZE);
            writer.write(crt.key(), cryptor::KEY_SIZE);
            writer.write<uint8_t>(static_cast<uint8_t>(from));
            writer.write<fb::stream>(parameter);
        }

        auto stream = fb::stream();
        {
            auto writer = fb::stream_writer<big_endian>(stream);
            co_await fb::protocol::response::transfer(ip, port, header).serialize(writer);
        }

        crt.wrap(stream);
        co_await socket.send(stream, false, false);
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
        co_await this->transfer(socket, inet_addr(ip.c_str()), port, from, parameter);
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
     * @param[in]  cmd   The command
     * @param[in]  fn    The function
     *
     * @tparam     R     { description }
     */
    template <typename R>
    void bind(int cmd, const std::function<async::task<bool>(fb::socket<T>&, const R&)>& fn)
    { }

protected:
    /**
     * @brief      { function_description }
     *
     * @param[in]  <unnamed>  { parameter_description }
     * @param[in]  header     The header
     *
     * @tparam     Class      { description }
     * @tparam     Request    { description }
     */
    template <typename Class, typename Request>
    void bind(async::task<bool> (Class::*fn)(fb::socket<T>&, const Request&), uint8_t header)
    {
        this->_deserializer.insert({header, [](auto& reader) -> async::task<fb::protocol::base::header*> {
                                        auto protocol = new Request();
                                        co_await protocol->deserialize(reader);
                                        co_return protocol;
                                    }});

        auto func_bound = std::bind(fn, static_cast<Class*>(this), std::placeholders::_1, std::placeholders::_2);
        this->_handler.insert({header, [func_bound](auto& socket, auto& header) -> async::task<bool> {
                                   auto protocol = static_cast<Request&>(header);
                                   co_return co_await func_bound(socket, protocol);
                               }});
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
    void bind(async::task<bool> (Class::*fn)(fb::socket<T>&, const Request&))
    {
        this->bind(fn, Request::header);
    }

protected:
    /**
     * @brief      { function_description }
     *
     * @param[in]  fn        The function
     * @param[in]  duration  The duration
     */
    template <typename Class>
    void bind_timer(async::task<void> (Class::*fn)(void), const std::chrono::steady_clock::duration& duration)
    {
        auto cfunc = std::bind(fn, static_cast<Class*>(this));
        auto timer = std::make_shared<boost::asio::deadline_timer>(this->_boost_context, boost::posix_time::seconds(1));
        auto callback_ptr = std::make_shared<std::function<void(const boost::system::error_code&)>>();
        auto callback     = [=](const boost::system::error_code&) {
            async::awaitable_then(cfunc(), [timer, callback_ptr, duration](async::awaitable_result<void> result) {
                timer->expires_at(timer->expires_at() +
                                  boost::posix_time::milliseconds(
                                      std::chrono::duration_cast<std::chrono::milliseconds>(duration).count()));
                timer->async_wait(*callback_ptr.get());
            });
        };
        *callback_ptr = callback;
        timer->async_wait(*callback_ptr.get());
    }

public:
    /**
     * @brief      { function_description }
     *
     * @param      socket   The socket
     * @param[in]  stream   The stream
     * @param[in]  encrypt  The encrypt
     * @param[in]  wrap     The wrap
     */
    [[nodiscard]] async::task<void>
    send(fb::socket<T>& socket, const fb::stream& stream, bool encrypt = true, bool wrap = true)
    {
        if (stream.empty())
            co_return;

        co_await socket.send(stream, encrypt, wrap);
    }

public:
    /**
     * @brief      { function_description }
     *
     * @param      socket    The socket
     * @param[in]  response  The response
     * @param[in]  encrypt   The encrypt
     * @param[in]  wrap      The wrap
     */
    [[nodiscard]] async::task<void>
    send(fb::socket<T>& socket, const fb::protocol::base::header& response, bool encrypt = true, bool wrap = true)
    {
        auto stream = fb::stream();
        auto writer = fb::stream_writer<big_endian>(stream);
        co_await response.serialize(writer);
        if (stream.empty())
            co_return;

        co_await socket.send(stream, encrypt, wrap);
    }

protected:
    /**
     * @brief      { function_description }
     */
    virtual void handle_background()
    {
        while (this->_running)
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

public:
    /**
     * @brief      { function_description }
     */
    void exit()
    {
        auto _ = std::lock_guard(this->_mutex_exit);

        if (this->_running == false)
            return;

        this->threads.exit();

        async::awaitable_get(this->handle_exit());
        this->cancel();
        this->sockets.close();
        this->_running = false;
    }
};

} // namespace fb

#endif // !__FB_ACCEPTOR_H__