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
class acceptor : public icontext
{
private:
    using handle_func     = std::function<async::task<bool>(fb::socket<T>&, const std::function<void()>&)>;
    using background_func = std::function<async::task<void>()>;

private:
    std::map<uint8_t, handle_func> _handler;
    fb::threads                    _threads;
    std::mutex                     _mutex_exit;
    bool                           _running = false;

protected:
    std::queue<background_func> _background_queue;
    std::mutex                  _background_queue_mutex;

protected:
    boost::asio::io_context& _context;
    fb::redis                _redis;
    fb::mutex                _mutex;

public:
    fb::socket_container<T> sockets;

protected:
    /**
     * @brief      Constructs a new instance.
     *
     * @param      context  The context
     * @param[in]  port     The port
     */
    acceptor(boost::asio::io_context& context, uint16_t port) :
        icontext(context, port),
        _context(context),
        _threads(context),
        _redis(*this,
               fb::config::get()["redis"]["default"]["ip"].asString(),
               fb::config::get()["redis"]["default"]["port"].asUInt()),
        _mutex(*this)
    {
        this->accept();
    }

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
    async::task<httplib::Result> get_internal(const std::string& host,
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
    async::task<Response> get_internal(const std::string& host, const std::string& path)
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
    async::task<Response> get(const std::string& route, const std::string& path)
    {
        auto& config = fb::config::get();
        auto  host   = std::format("http://{}:{}", config[route]["ip"].asCString(), config[route]["port"].asUInt());
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
    async::task<httplib::Result> post_internal(const std::string& host,
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
    async::task<Response> post_internal(const std::string& host, const std::string& path, const Request& body)
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
    async::task<Response> post(const std::string& route, const std::string& path, const Request& body)
    {
        auto& config = fb::config::get();
        auto  host   = std::format("http://{}:{}", config[route]["ip"].asCString(), config[route]["port"].asUInt());
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
    async::task<void> dispatch(fb::socket<T>& socket, const dispatch_callback& fn, uint32_t priority = 0)
    {
        auto id     = this->thread_id(socket);
        auto thread = this->_threads[id];

        if (thread == nullptr)
            throw std::runtime_error("thread does not exists");

        co_await this->_threads[id]->dispatch(fn, 0s, priority);
    }

public:
    /**
     * @brief      { function_description }
     *
     * @param      socket    The socket
     * @param[in]  priority  The priority
     *
     * @return     { description_of_the_return_value }
     */
    async::task<void> dispatch(fb::socket<T>& socket, uint32_t priority = 0)
    {
        auto id     = this->thread_id(socket);
        auto thread = this->_threads[id];

        if (thread != nullptr)
            co_await this->_threads[id]->dispatch(priority);
    }

private:
    /**
     * @brief      { function_description }
     *
     * @param      socket     The socket
     * @param      reader  The stream to read data from
     *
     * @return     { description_of_the_return_value }
     */
    async::task<bool> execute_bound_handler(fb::socket<T>& socket, fb::stream& stream)
    {
        static constexpr uint8_t base_size = sizeof(uint8_t) + sizeof(uint16_t);
        auto                     reader    = fb::stream_reader<big_endian>(stream);
        while (true)
        {
            try
            {
                if (reader.readable_size() < base_size)
                    break;

                // Read base head and check it is 0xAA
                auto head = reader.read<uint8_t>();
                if (head != 0xAA)
                    throw std::exception();

                auto size = reader.read<uint16_t>();
                if (reader.readable_size() < size)
                    break;

                auto cmd = reader.read<uint8_t>();
                if (this->decrypt_policy(cmd))
                    size = socket.crt().decrypt(stream, reader.seek() - 1, size);

                // Call function that matched by command byte
                if (this->_handler.contains(cmd) == false)
                {
                    fb::logger::warn("정의되지 않은 요청입니다. [{:#x}]", cmd);
                    reader.seek(base_size + size);
                    reader.flush();
                    continue;
                }

                reader.flush(); // remove magic code and size
                auto before = this->thread_id(socket);
                auto result = co_await this->_handler[cmd](socket, [&reader, size] {
                    reader.seek(size - sizeof(uint8_t));
                    reader.flush();
                });
                auto after  = this->thread_id(socket);

                // 콜백 조건이 만족하지 못하는 경우 즉시 종료
                if (before != after)
                    co_return true;
            }
            catch (std::exception& e)
            {
                fb::logger::fatal(e.what());
                reader.clear();
                break;
            }
            catch (...)
            {
                reader.clear();
                break;
            }
        }

        reader.seek(0);
        co_return false;
    }

private:
    /**
     * @brief      { function_description }
     *
     * @param      socket  The socket
     *
     * @return     { description_of_the_return_value }
     */
    async::task<void> handle_work(fb::socket<T>& socket)
    {
        if (this->_running == false)
            co_return;

        auto switched = co_await socket.template stream<async::task<bool>>(
            [this, &socket](fb::stream& stream) -> async::task<bool> {
                co_return co_await this->execute_bound_handler(socket, stream);
            });

        if (switched == false)
            co_return;

        co_await this->dispatch(socket);
        co_await this->handle_work(socket);
    }

private:
    /**
     * @brief      { function_description }
     */
    void accept()
    {
        auto callback_received = [this](fb::socket<T>& socket) -> async::task<void> {
            if (this->_running == false)
                throw std::runtime_error("acceptor closed");

            auto& casted = static_cast<fb::socket<T>&>(socket);
            auto  id     = this->thread_id(casted);

            if (id == 0xFF)
            {
                co_await this->handle_work(casted);
            }
            else
            {
                co_await this->dispatch(socket);
                co_await this->handle_work(casted);
            }
        };

        auto callback_closed = [this](fb::socket<T>& socket) -> async::task<void> {
            if (socket.data() == nullptr)
                co_return;

            auto& casted = static_cast<fb::socket<T>&>(socket);
            co_await this->dispatch(casted);
            co_await this->handle_disconnected(casted);
            this->sockets.erase(casted);
        };

        auto socket = std::make_unique<fb::socket<T>>(this->_context, callback_received, callback_closed);
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
            catch (std::exception& /*e*/)
            {
                // std::cout << e.what() << std::endl;
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
     */
    void transfer(fb::socket<T>& socket, uint32_t ip, uint16_t port, fb::protocol::internal::services from)
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
            fb::protocol::response::transfer(ip, port, params).serialize(writer);
        }

        crt.wrap(stream);
        socket.send(stream, false, false);
    }

public:
    /**
     * @brief      { function_description }
     *
     * @param      socket  The socket
     * @param[in]  ip      { parameter_description }
     * @param[in]  port    The port
     * @param[in]  from    The from
     */
    void transfer(fb::socket<T>& socket, const std::string& ip, uint16_t port, fb::protocol::internal::services from)
    {
        this->transfer(socket, inet_addr(ip.c_str()), port, from);
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
     */
    void transfer(fb::socket<T>&                   socket,
                  uint32_t                         ip,
                  uint16_t                         port,
                  fb::protocol::internal::services from,
                  const fb::stream&                parameter)
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
            fb::protocol::response::transfer(ip, port, header).serialize(writer);
        }

        crt.wrap(stream);
        socket.send(stream, false, false);
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
     */
    void transfer(fb::socket<T>&                   socket,
                  const std::string&               ip,
                  uint16_t                         port,
                  fb::protocol::internal::services from,
                  const fb::stream&                parameter)
    {
        this->transfer(socket, inet_addr(ip.c_str()), port, from, parameter);
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
    virtual uint32_t thread_id(const fb::socket<T>& socket) const
    {
        return socket.fd();
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
        auto count = this->_threads.count();
        if (count == 0)
            return 0xFF;

        return this->_threads[this->thread_id(socket) % count];
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
        return (uint8_t)fb::config::get()["id"].asUInt();
    }

protected:
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    virtual std::string name() const
    {
        return fb::config::get()["name"].asString();
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
        auto bound_func = std::bind(fn, static_cast<Class*>(this), std::placeholders::_1, std::placeholders::_2);
        this->_handler.insert(
            {header, [this, bound_func](fb::socket<T>& socket, const std::function<void()>& callback) {
                 return socket.template stream<async::task<bool>>(
                     [this, &bound_func, &socket, &callback](fb::stream& stream) {
                         auto protocol = Request();
                         auto reader   = fb::stream_reader<big_endian>(stream);
                         protocol.deserialize(reader);
                         callback();
                         return bound_func(socket, protocol);
                     });
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
    void bind_timer(const std::function<async::task<void>()>& fn, const std::chrono::steady_clock::duration& duration)
    {
        auto timer = std::make_shared<boost::asio::deadline_timer>(this->_context, boost::posix_time::seconds(1));
        auto callback_ptr = std::make_shared<std::function<void(const boost::system::error_code&)>>();
        auto callback     = [=](const boost::system::error_code&) {
            async::awaitable_then(fn(), [timer, callback_ptr, duration](async::awaitable_result<void> result) {
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
    void send(fb::socket<T>& socket, const fb::stream& stream, bool encrypt = true, bool wrap = true)
    {
        if (stream.empty())
            return;

        socket.send(stream, encrypt, wrap);
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
    void send(fb::socket<T>& socket, const fb::protocol::base::header& response, bool encrypt = true, bool wrap = true)
    {
        auto stream = fb::stream();
        auto writer = fb::stream_writer<big_endian>(stream);
        response.serialize(writer);
        if (stream.empty())
            return;

        socket.send(stream, encrypt, wrap);
    }

public:
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    fb::threads& threads()
    {
        return this->_threads;
    }

public:
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    const fb::threads& threads() const
    {
        return this->_threads;
    }

public:
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    fb::thread* current_thread()
    {
        return this->_threads.current();
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
    async::task<R> background(const std::function<async::task<R>()>& func)
    {
        auto _       = std::lock_guard(this->_background_queue_mutex);
        auto promise = std::make_shared<async::task_completion_source<R>>();
        auto thread  = this->current_thread();
        this->_background_queue.push([this, promise, thread, func]() -> async::task<void> {
            try
            {
                if constexpr (std::is_void_v<R>)
                {
                    co_await func();
                    if (thread != nullptr)
                        co_await thread->dispatch();
                    promise->set_value();
                }
                else
                {
                    R result = co_await func();
                    if (thread != nullptr)
                        co_await thread->dispatch();
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
        auto& config  = fb::config::get();
        auto  threads = std::vector<std::thread>();

        this->_running = true;
        for (int i = 0; i < config["thread"]["io"].asUInt(); i++)
        {
            threads.push_back(std::thread([this]() {
                this->_context.run();
            }));
        }

        for (int i = 0; i < config["thread"]["background"].asUInt(); i++)
        {
            threads.push_back(std::thread([this]() {
                this->handle_background();
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
    async::task<void> sleep(const fb::model::timespan& duration)
    {
        auto thread = this->_threads.current();
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

        this->_threads.exit();

        async::awaitable_get(this->handle_exit());
        this->cancel();
        this->sockets.close();
        this->_running = false;
    }

public:
    inline operator boost::asio::io_context& () const
    {
        return this->_context;
    }
};

} // namespace fb

#endif // !__FB_ACCEPTOR_H__