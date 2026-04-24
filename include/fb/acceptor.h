#ifndef __FB_ACCEPTOR_H__
#define __FB_ACCEPTOR_H__

#include <ctime>
#include <fb/mutex.h>
#include <fb/protocol/flatbuffer/protocol.h>
#include <fb/protocol/transfer.h>
#include <fb/protocol_handler_registry.h>
#include <fb/amqp_handler_registry.h>
#include <fb/http_client.h>
#include <fb/socket.h>
#include <iomanip>
#include <mutex>
#include <boost/stacktrace.hpp>

namespace fb {

using namespace std::chrono_literals;

template <typename T>
class acceptor : public fb::async_executor, public boost::asio::ip::tcp::acceptor
{
public:
    using socket_container      = std::unordered_map<uint32_t, std::shared_ptr<fb::socket<T>>>;
    using socket_container_lock = fb::locker<socket_container>;
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

    handler         handler;
    fb::http_client http;

private:
    boost_timers        _timers;
    mutable std::mutex  _now_mutex;
    fb::model::timespan _now_offset;

protected:
    socket_container_lock _sockets;

protected:
    acceptor(boost::asio::io_context& context, std::string_view name, uint16_t port, size_t http_max_concurrent = 500) :
        fb::async_executor(context, name, config<uint32_t>("thread:logic")),
        boost::asio::ip::tcp::acceptor(context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
        handler(*this),
        http(*this, http_max_concurrent)
    {
        static auto flag = std::once_flag{};
        std::call_once(flag, [port] {
            console::puts("Listen port : {}", port);
        });
    }

public:
    virtual ~acceptor()
    {
        this->exit();
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

                auto opcode = reader.read<uint8_t>();
                if (this->decrypt_policy(opcode))
                    size = socket.encryption().decrypt(stream, reader.seek() - 1, size);

                reader.flush(); // remove magic code and size

                socket.update_last_packet_time();

                if (!this->handler.protocol.has_deserializer(opcode))
                {
                    fb::logger::warn(std::format("Undefined protocol. [{:#x}]", opcode));
                }
                else if (!this->handler.protocol.has_handler(opcode))
                {
                    fb::logger::warn(std::format("Undefined handler. [{:#x}]", opcode));
                }
                else
                {
                    auto protocol = co_await this->handler.protocol.get_deserializer(opcode)(reader);
                    auto fd       = socket.fd();
                    auto weak     = socket.template weak_from_this_as<fb::socket<T>>();
                    this->threads.enqueue(weak, [this, protocol, weak, fd, opcode](auto& thread) -> async::task<void> {
                        try
                        {
                            if (weak.expired())
                                co_return;

                            auto shared = weak.lock();
                            if (shared == nullptr)
                                co_return;

                            auto  socket  = shared.get();
                            auto& handler = this->handler.protocol.get_handler(opcode);
                            // Check both global socket TPS and per-command TPS limits
                            // If either limit is exceeded, ignore the packet
                            if (this->assert_tps(*socket) &&
                                !socket->limiter.update(opcode, handler.duration, handler.limit))
                                co_return;

                            [[maybe_unused]]
                            volatile auto holder  = protocol;
                            auto          success = co_await handler.fn(*socket, *protocol.get());
                            if (success == false)
                                socket->close();
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
    async::task<void> erase(fb::socket<T>& socket)
    {
        try
        {
            std::ignore = co_await this->on_disconnected(socket);
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

    void accept()
    {
        auto socket_ptr = std::make_shared<fb::socket<T>>(*this,
                                                          std::bind_front(&acceptor::on_socket_received, this),
                                                          std::bind_front(&acceptor::on_socket_closed, this));
        this->async_accept(*socket_ptr, [this, socket_ptr](boost::system::error_code error) mutable {
            try
            {
                if (error)
                    throw std::runtime_error(error.message());

                if (this->_running == false)
                    throw std::runtime_error("cannot accept socket. acceptor is cleaning now.");

                async::awaitable_get(this->on_accepted(*socket_ptr));
                socket_ptr->set_option(boost::asio::ip::tcp::no_delay(false));

                {
                    auto fd = socket_ptr->fd();
                    this->_sockets.write([fd, &socket_ptr](auto& v) -> void {
                        if (v.contains(fd))
                        {
                            fb::logger::warn(std::format("socket already exists. fd: {}", fd));
                            v.erase(fd); // remove old socket if exists
                        }

                        v.insert({fd, socket_ptr});
                    });
                }

                async::awaitable_get(this->on_connected(*socket_ptr));

                boost::asio::co_spawn(*this, socket_ptr->recv(), boost::asio::detached);
                this->accept();
            }
            catch (std::exception& e)
            {
                fb::logger::fatal("acceptor::accept: error={}\n{}",
                                  e.what(),
                                  boost::stacktrace::to_string(boost::stacktrace::stacktrace()));
                socket_ptr->close();
            }
        });
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
    virtual bool decrypt_policy(uint8_t opcode) const
    {
        return true;
    }

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
        lua::build<lua::luable>();
        lua::build<fb::thread, lua::luable>();
        lua::build<fb::thread_switchable, lua::luable>();
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
    fb::thread* thread(const fb::socket<T>& socket) const
    {
        auto count = this->threads.count();
        if (count == 0)
            return 0xFF;

        return this->threads[this->thread_id(socket) % count];
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
        co_await response.serialize(writer);
        if (stream.empty())
            co_return 0;

        co_return co_await socket.send(stream, encrypt, wrap);
    }

public:
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

        async::awaitable_get(this->on_start());

        threads.push_back(std::thread([this]() {
            this->handler.amqp.on_initialize = [this](fb::amqp::socket& amqp) {
                this->on_init_amqp(amqp);
            };

            this->handler.amqp.thread_loop();
        }));

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
                    std::ignore = co_await this->on_disconnected(*socket);
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
    void exit() override final
    {
        if (this->_running == false)
            return;

        this->_running = false;
        this->cancel();
        async::awaitable_get(this->on_exit());
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