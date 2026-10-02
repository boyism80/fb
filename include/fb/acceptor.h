#ifndef __FB_ACCEPTOR_H__
#define __FB_ACCEPTOR_H__

#include <async/awaitable_get.h>
#include <fb/amqp_handler_registry.h>
#include <fb/asio_task.h>
#include <fb/context.h>
#include <fb/execution_context.h>
#include <fb/http_client.h>
#include <fb/lua.h>
#include <fb/mutex.h>
#include <fb/protocol/client_version.h>
#include <fb/protocol/flatbuffer/protocol.h>
#include <fb/protocol/transfer.h>
#include <fb/protocol_handler_registry.h>
#include <fb/socket.h>

#include <boost/stacktrace.hpp>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <format>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

namespace fb {

using namespace std::chrono_literals;

template <typename T>
class acceptor : public fb::async_executor, public boost::asio::ip::tcp::acceptor
{
public:
    using socket_container      = std::unordered_map<uint32_t, std::shared_ptr<fb::socket<T>>>;
    using socket_container_sync = fb::synchronized<socket_container>;
    using boost_timers          = std::vector<std::shared_ptr<boost::asio::deadline_timer>>;
    using session_type          = fb::socket<T>;

    struct handler
    {
        fb::protocol_handler_registry<T> protocol;
        fb::amqp_handler_registry<T>     amqp;

        explicit handler(fb::acceptor<T>& owner) :
            protocol(owner),
            amqp(owner)
        { }

        // Delete copy constructor and assignment operator
        handler(const handler&)             = delete;
        handler& operator= (const handler&) = delete;
    };

    handler               handler;
    fb::http_client       http;
    fb::lua::context_pool lua;

private:
    static constexpr auto ACCEPT_RETRY_DELAY = std::chrono::milliseconds(100);
    static constexpr auto LOGIN_TIMEOUT      = std::chrono::seconds(30);
    static constexpr auto ACCEPT_LOG_EVERY   = uint32_t(100);
    static constexpr auto DISCONNECT_TIMEOUT = std::chrono::seconds(30);

private:
    boost_timers            _timers;
    mutable std::mutex      _now_mutex;
    fb::model::timespan     _now_offset;
    uint32_t                _accept_failures = 0;
    std::atomic<uint32_t>   _disconnecting   = 0;
    std::mutex              _exit_mutex;
    std::condition_variable _exit_cv;
    bool                    _exit_requested = false;

protected:
    socket_container_sync _sockets;

protected:
    acceptor(boost::asio::io_context& context, std::string_view name, uint16_t port, size_t http_max_concurrent = 128) :
        fb::async_executor(context, name, config<uint32_t>("thread:logic")),
        boost::asio::ip::tcp::acceptor(context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
        handler(*this),
        http(*this, http_max_concurrent),
        lua(static_cast<fb::async_executor&>(*this))
    {
        static auto flag = std::once_flag{};
        std::call_once(flag, [port] {
            console::puts("Listen port : {}", port);
        });
    }

public:
    virtual ~acceptor()
    {
        // Reached with _running only when run() aborted; derived overrides such as on_exit() are already gone.
        if (this->_running == false)
            return;

        this->_running = false;
        this->cancel();
        this->threads.exit();
        this->close();
        static_cast<boost::asio::io_context&>(*this).stop();
    }

protected:
    virtual void on_init_amqp(fb::amqp::socket& amqp) = 0;

public:
    bool connected(uint32_t fd)
    {
        return this->_sockets.template read<bool>([fd](const auto& v) -> bool {
            return v.contains(fd);
        });
    }

private:
    async::task<void> execute_handler(fb::socket<T>& socket, fb::stream& stream)
    {
        static constexpr uint8_t  base_size = sizeof(uint8_t) + sizeof(uint16_t);
        static constexpr uint32_t MAX_TPS   = 100;

        auto reader = fb::stream_reader<big_endian>(stream);
        auto opcode = std::optional<uint8_t>{};
        try
        {
            while (!stream.empty())
            {
                opcode.reset();

                if (socket.is_open() == false)
                    break;

                if (reader.readable_size() < base_size)
                    co_return;

                auto head = reader.read<uint8_t>();
                if (head != 0xAA)
                    throw std::runtime_error("magic code mismatch");

                // size covers the opcode byte, so zero is never valid.
                auto size = reader.read<uint16_t>();
                if (size < sizeof(uint8_t) || size > fb::socket<T>::MAX_BUFFER_SIZE)
                    throw std::runtime_error("packet size mismatch");

                if (reader.readable_size() < size)
                    break;

                if (this->assert_tps(socket) && socket.limiter.update(MAX_TPS) == false)
                    throw std::runtime_error("tps limit exceeded");

                opcode = reader.read<uint8_t>();
                if (this->handler.protocol.should_decrypt(*opcode))
                    size = socket.encryption().decrypt(stream, reader.seek() - 1, size);

                reader.flush(); // remove magic code and size

                socket.update_last_packet_time();

                // Session missing / version not established → deserialize as v550.
                // A logic thread may release the session at any time; keep it alive while reading the version.
                auto session_data   = socket.data_ptr();
                auto client_version = fb::protocol::client_version_or_default(session_data.get());

                if (!this->handler.protocol.has_opcode(*opcode))
                {
                    fb::logger::warn(std::format("Undefined protocol. [{:#x}]", *opcode));
                }
                else if (!this->handler.protocol.has_entry(*opcode, client_version))
                {
                    fb::logger::warn(std::format("Undefined handler. [{:#x}] version={}",
                                                 *opcode,
                                                 fb::protocol::to_string(client_version)));
                }
                else
                {
                    // Deserialize from a copy of this packet's body so a parser cannot read into the next packet.
                    auto body           = fb::stream(stream.data(), size - sizeof(uint8_t));
                    auto body_reader    = fb::stream_reader<big_endian>(body);
                    auto protocol       = this->handler.protocol.get_deserializer(*opcode, client_version)(body_reader);
                    auto await_dispatch = this->handler.protocol.get_handler(*opcode, client_version).await_dispatch;
                    auto fd             = socket.fd();
                    auto weak           = socket.template weak_from_this_as<fb::socket<T>>();
                    auto builder        = this->threads.new_builder(weak);
                    auto frame          = execution_context::create();
                    frame->slot(context::local::slot_id(), context{.transaction_id = mint_transaction_id()});
                    builder.context = execution_context::token(std::move(frame));
                    builder.func    = [this, protocol, weak, fd, opcode = *opcode, client_version](
                                       auto& thread) -> async::task<void> {
                        try
                        {
                            if (weak.expired())
                                co_return;

                            auto shared = weak.lock();
                            if (shared == nullptr)
                                co_return;

                            auto  socket  = shared.get();
                            auto& handler = this->handler.protocol.get_handler(opcode, client_version);
                            // Check both global socket TPS and per-command TPS limits
                            // If either limit is exceeded, ignore the packet
                            if (this->assert_tps(*socket) &&
                                !socket->limiter.update(opcode, handler.duration, handler.limit))
                                co_return;

                            [[maybe_unused]]
                            volatile auto holder = protocol;
                            // Handlers keep raw session.data() across co_await; on_disconnected may reset it meanwhile.
                            [[maybe_unused]]
                            auto session_data = socket->data_ptr();
                            auto success      = co_await handler.fn(*socket, *protocol.get());
                            if (success == false)
                                socket->close();
                        }
                        catch (std::exception& e)
                        {
                            fb::logger::fatal("{} [{:#x}]", e.what(), opcode);
                        }
                        catch (...)
                        {
                            fb::logger::fatal("unhandled exception [{:#x}]", opcode);
                        }
                    };

                    // Default: enqueue so the receive loop keeps parsing. Opt in to
                    // await_dispatch on handlers that must finish before the next packet.
                    if (await_dispatch)
                        co_await builder.dispatch();
                    else
                        builder.enqueue();
                }

                reader.seek(size - sizeof(uint8_t));
                reader.flush(); // remove packet body
            }

            reader.seek(0);
        }
        catch (std::exception& e)
        {
            if (opcode.has_value())
                fb::logger::fatal("{} [{:#x}]", e.what(), opcode.value());
            else
                fb::logger::fatal(e.what());
            socket.close();
        }
        catch (...)
        {
            if (opcode.has_value())
                fb::logger::fatal("unhandled exception while parse packet [{:#x}]", opcode.value());
            else
                fb::logger::fatal("unhandled exception while parse packet");
            socket.close();
        }
    }

private:
    async::task<void> erase(fb::socket<T>& socket)
    {
        // Counted before the mark so exit() never sees a skipped socket with a zero count.
        this->_disconnecting++;
        if (socket.mark_disconnected())
        {
            try
            {
                std::ignore = co_await this->on_disconnected(socket);
            }
            catch (std::exception& e)
            {
                fb::logger::fatal(e.what());
            }
            catch (...)
            {
                fb::logger::fatal("on_disconnected: unknown exception");
            }
        }
        this->_disconnecting--;

        if (socket.is_open())
            socket.close();

        auto fd = socket.fd();
        {
            auto  guard = this->_sockets.enter_write();
            auto& v     = guard.value();
            if (auto it = v.find(fd); it != v.end() && it->second.get() == &socket)
                v.erase(it);
        }
    }

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

    async::task<void> on_socket_closed(fb::socket<T>& socket)
    {
        auto socket_ptr = socket.template shared_from_this_as<fb::socket<T>>();

        if (socket_ptr->data() != nullptr)
        {
            try
            {
                auto weak = socket_ptr->template weak_from_this_as<fb::socket<T>>();
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
        }

        try
        {
            co_await this->erase(*socket_ptr);
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

    boost::asio::awaitable<void> serve(std::shared_ptr<fb::socket<T>> socket_ptr)
    {
        try
        {
            co_await fb::async_await_task(this->on_accepted(*socket_ptr), boost::asio::use_awaitable);
            socket_ptr->set_option(boost::asio::ip::tcp::no_delay(false));

            {
                auto  fd    = socket_ptr->fd();
                auto  guard = this->_sockets.enter_write();
                auto& v     = guard.value();
                if (auto it = v.find(fd); it != v.end())
                {
                    if (it->second.get() != socket_ptr.get())
                    {
                        fb::logger::warn(std::format("socket already exists. fd: {}", fd));
                        v.erase(it);
                    }
                }

                v.insert_or_assign(fd, socket_ptr);
            }

            co_await fb::async_await_task(this->on_connected(*socket_ptr), boost::asio::use_awaitable);

            // Every server binds session data on its first packet; drop connections that never send one.
            auto& context     = static_cast<boost::asio::io_context&>(*this);
            auto  login_timer = std::make_shared<boost::asio::steady_timer>(context, LOGIN_TIMEOUT);
            auto  weak        = std::weak_ptr<fb::socket<T>>(socket_ptr);
            login_timer->async_wait([login_timer, weak](const boost::system::error_code& ec) {
                if (ec)
                    return;

                auto socket = weak.lock();
                if (socket == nullptr || socket->data() != nullptr || socket->is_open() == false)
                    return;

                fb::logger::warn("acceptor::serve: login timeout. fd: {}", socket->fd());
                socket->close();
            });

            co_await socket_ptr->recv();
        }
        catch (std::exception& e)
        {
            fb::logger::fatal("acceptor::serve: error={}\n{}",
                              e.what(),
                              boost::stacktrace::to_string(boost::stacktrace::stacktrace()));
            socket_ptr->close();
        }
        catch (...)
        {
            fb::logger::fatal("acceptor::serve: unknown error");
            socket_ptr->close();
        }
    }

    void accept()
    {
        auto socket_ptr = std::make_shared<fb::socket<T>>(*this,
                                                          std::bind_front(&acceptor::on_socket_received, this),
                                                          std::bind_front(&acceptor::on_socket_closed, this));
        this->async_accept(*socket_ptr, [this, socket_ptr](boost::system::error_code error) mutable {
            if (this->_running == false)
            {
                socket_ptr->close();
                return;
            }

            if (error)
            {
                // Keep listening after transient failures such as fd exhaustion; the delay avoids a busy loop.
                if (this->_accept_failures % ACCEPT_LOG_EVERY == 0)
                    fb::logger::fatal("acceptor::accept: error={} (consecutive failures: {})",
                                      error.message(),
                                      this->_accept_failures + 1);
                this->_accept_failures++;
                socket_ptr->close();

                auto& context = static_cast<boost::asio::io_context&>(*this);
                auto  timer   = std::make_shared<boost::asio::steady_timer>(context, ACCEPT_RETRY_DELAY);
                timer->async_wait([this, timer](const boost::system::error_code& ec) {
                    if (ec || this->_running == false)
                        return;

                    this->accept();
                });
            }
            else
            {
                this->_accept_failures = 0;
                try
                {
                    // Drive cpp-async handshake on an Asio awaitable without blocking the IO thread.
                    boost::asio::co_spawn(*this, this->serve(socket_ptr), boost::asio::detached);
                }
                catch (std::exception& e)
                {
                    fb::logger::fatal("acceptor::accept: error={}\n{}",
                                      e.what(),
                                      boost::stacktrace::to_string(boost::stacktrace::stacktrace()));
                    socket_ptr->close();
                }

                this->accept();
            }
        });
    }

private:
    static uint16_t transfer_client_version(fb::socket<T>& socket)
    {
        auto data = socket.data_ptr();
        if (data == nullptr)
            return static_cast<uint16_t>(fb::protocol::CLIENT_VERSION::v550);

        if constexpr (requires { data->client_version(); })
        {
            return static_cast<uint16_t>(data->client_version());
        }
        else if constexpr (requires { data->client_version; })
        {
            return static_cast<uint16_t>(data->client_version);
        }
        else
        {
            return static_cast<uint16_t>(fb::protocol::CLIENT_VERSION::v550);
        }
    }

public:
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
            writer.write<uint16_t>(transfer_client_version(socket));
        }

        auto stream = fb::stream();
        {
            auto writer = fb::stream_writer<big_endian>(stream);
            fb::protocol::response::transfer(ip, port, params).serialize(writer);
        }

        std::ignore = co_await socket.send(stream, false, true);
    }

public:
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
            writer.write<uint16_t>(transfer_client_version(socket));
            writer.write<fb::stream>(parameter);
        }

        auto stream = fb::stream();
        {
            auto writer = fb::stream_writer<big_endian>(stream);
            fb::protocol::response::transfer(ip, port, header).serialize(writer);
        }

        std::ignore = co_await socket.send(stream, false, true);
    }

public:
    [[nodiscard]] async::task<void>
    transfer(fb::socket<T>& socket, std::string_view ip, uint16_t port, fb::protocol::internal::Service from)
    {
        co_await this->transfer(socket, inet_addr(this->ipv4(ip).c_str()), port, from);
    }

public:
    [[nodiscard]] async::task<void> transfer(fb::socket<T>&                  socket,
                                             std::string_view                ip,
                                             uint16_t                        port,
                                             fb::protocol::internal::Service from,
                                             const fb::stream&               parameter)
    {
        co_await this->transfer(socket, inet_addr(this->ipv4(ip).c_str()), port, from, parameter);
    }

protected:
    virtual bool assert_tps(const fb::socket<T>& socket) const
    {
        return true;
    }

protected:
    virtual async::task<void> on_accepted(fb::socket<T>& socket)
    {
        co_return;
    }

protected:
    virtual async::task<void> on_start()
    {
        for (auto& [_, root] : this->lua)
        {
            co_await root->switching();
            root->build<lua::luable>();
            root->build<fb::thread, lua::luable>();
            root->build<fb::thread_switchable, lua::luable>();
        }
        co_return;
    }

protected:
    virtual async::task<bool> on_connected(fb::socket<T>& session)
    {
        co_return true;
    }

protected:
    virtual async::task<bool> on_disconnected(fb::socket<T>& session)
    {
        co_return true;
    }

protected:
    virtual async::task<void> on_exit()
    {
        co_return;
    }

protected:
    virtual uint8_t id() const
    {
        return fb::config<uint8_t>("id");
    }

protected:
    virtual std::string name() const
    {
        return fb::config<std::string>("name");
    }

protected:
    virtual fb::protocol::internal::Service service() const = 0;

public:
    async::task<size_t> send(fb::socket<T>& socket, const fb::stream& stream, bool encrypt = true, bool wrap = true)
    {
        if (stream.empty())
            co_return 0;

        co_return co_await socket.send(stream, encrypt, wrap);
    }

public:
    async::task<size_t>
    send(fb::socket<T>& socket, const fb::protocol::header& response, bool encrypt = true, bool wrap = true)
    {
        auto stream = fb::stream();
        auto writer = fb::stream_writer<big_endian>(stream);
        response.serialize(writer);
        if (stream.empty())
            co_return 0;

        co_return co_await socket.send(stream, encrypt, wrap);
    }

public:
    void run()
    {
        this->_running = true;
        // Sync boundary between main thread and async-cpp (startup loaders / Lua init).
        async::awaitable_get(this->on_start());
        this->accept();

        auto threads = std::vector<std::thread>();
        for (int i = 0; i < fb::config<uint32_t>("thread:io"); i++)
        {
            threads.push_back(std::thread([this]() {
                this->io_context.run();
            }));
        }

        threads.push_back(std::thread([this]() {
            this->handler.amqp.on_initialize = [this](fb::amqp::socket& amqp) {
                this->on_init_amqp(amqp);
            };

            this->handler.amqp.thread_loop();
        }));

        // Shutdown blocks until disconnects and saves finish, which need the IO and logic threads to keep running.
        // Run it here on the main thread, which is neither, instead of on whichever thread called exit().
        {
            auto lock = std::unique_lock(this->_exit_mutex);
            this->_exit_cv.wait(lock, [this] {
                return this->_exit_requested;
            });
        }
        this->shutdown();

        for (auto& thread : threads)
        {
            thread.join();
        }
    }

public:
    bool running() const
    {
        return this->_running;
    }

public:
    [[nodiscard]] async::task<void> sleep(const fb::model::timespan& duration)
    {
        auto thread = this->threads.current();
        if (thread != nullptr)
            co_await thread->sleep(duration);
    }

public:
    fb::model::datetime now() const
    {
        auto lock = std::lock_guard<std::mutex>(this->_now_mutex);
        return fb::model::datetime() + this->_now_offset;
    }

public:
    void now(const fb::model::datetime& value)
    {
        auto current      = fb::model::datetime();
        auto lock         = std::lock_guard<std::mutex>(this->_now_mutex);
        this->_now_offset = value - current;
    }

public:
    fb::model::timespan now_offset() const
    {
        auto lock = std::lock_guard<std::mutex>(this->_now_mutex);
        return this->_now_offset;
    }

public:
    void reset_now_offset()
    {
        auto lock         = std::lock_guard<std::mutex>(this->_now_mutex);
        this->_now_offset = fb::model::timespan();
    }

private:
    [[nodiscard]] async::task<void> disconnect_sockets()
    {
        auto pairs = std::unordered_map<fb::thread*, std::vector<std::shared_ptr<fb::socket<T>>>>();
        {
            auto guard = this->_sockets.enter_read();
            for (auto& [fd, socket] : guard.value())
            {
                std::ignore = fd;
                auto thread = socket->thread();
                if (thread != nullptr)
                    pairs[thread].push_back(socket);
            }
        }

        for (auto& [thread, sockets] : pairs)
        {
            co_await thread->switching();
            for (auto& socket : sockets)
            {
                if (socket->mark_disconnected())
                {
                    try
                    {
                        std::ignore = co_await this->on_disconnected(*socket);
                    }
                    catch (std::exception& e)
                    {
                        fb::logger::fatal(e.what());
                        std::cerr << boost::stacktrace::stacktrace() << std::endl;
                    }
                }
                socket->close();
            }
        }
    }

public:
    void access_sockets(std::function<void(const socket_container&)> fn)
    {
        auto guard = this->_sockets.enter_read();
        fn(guard.value());
    }

public:
    // Only requests shutdown, so IO threads (signals) and logic threads (AMQP) can call it.
    // run() performs the shutdown on the main thread.
    void exit() override final
    {
        {
            auto lock             = std::lock_guard(this->_exit_mutex);
            this->_exit_requested = true;
        }
        this->_exit_cv.notify_all();
    }

private:
    void shutdown()
    {
        if (this->_running == false)
            return;

        this->_running = false;
        this->cancel();
        // Sync boundary between main/shutdown thread and async-cpp (save, drain queues).
        // A failure here must not skip the teardown below, or run() would destroy joinable threads.
        try
        {
            async::awaitable_get(this->on_exit());
        }
        catch (std::exception& e)
        {
            fb::logger::fatal("acceptor::shutdown: on_exit failed: {}", e.what());
        }

        try
        {
            async::awaitable_get(this->disconnect_sockets());
        }
        catch (std::exception& e)
        {
            fb::logger::fatal("acceptor::shutdown: disconnect_sockets failed: {}", e.what());
        }

        // disconnect_sockets() skips sockets whose on_disconnected already started in erase();
        // logic threads must keep running until those finish (e.g. character save).
        auto deadline = std::chrono::steady_clock::now() + DISCONNECT_TIMEOUT;
        while (this->_disconnecting.load() > 0 && std::chrono::steady_clock::now() < deadline)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        if (this->_disconnecting.load() > 0)
            fb::logger::warn("acceptor::shutdown: {} disconnects still running after timeout",
                             this->_disconnecting.load());

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