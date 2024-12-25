#ifndef __SOCKET_H__
#define __SOCKET_H__

#include <exception>
#include <unordered_map>
#include <deque>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <boost/system/error_code.hpp>
#include <fb/protocol/protocol.h>
#include <fb/cryptor.h>
#include <fb/logger.h>
#include <async/task.h>
#include <async/task_completion_source.h>
#include <async/awaitable_get.h>
#include <fb/abstract.h>
#include <fb/thread.h>

namespace fb {

template <typename T = void*>
class socket : public boost::asio::ip::tcp::socket, public thread_switchable
{
public:
    using handle_read_event = std::function<async::task<void>(fb::socket<T>&, fb::stream&)>;
    using handler_event     = std::function<async::task<void>(fb::socket<T>&)>;

private:
    context&          _context;
    fb::cryptor       _crt;
    handle_read_event _handle_received;
    handler_event     _handle_closed;
    fb::stream        _stream;

protected:
    std::array<char, 256> _buffer;
    T*                    _data;
    std::recursive_mutex  _boost_mutex;

public:
    socket(context& context, const handle_read_event& handle_received, const handler_event& handle_closed) :
        boost::asio::ip::tcp::socket(static_cast<boost::asio::io_context&>(context)),
        _context(context),
        _handle_received(handle_received),
        _handle_closed(handle_closed)
    { }

public:
    socket(context&                 context,
           const fb::cryptor&       crt,
           const handle_read_event& handle_received,
           const handler_event&     handle_closed) :
        boost::asio::ip::tcp::socket(static_cast<boost::asio::io_context&>(context)),
        _context(context),
        _handle_received(handle_received),
        _handle_closed(handle_closed),
        _crt(crt)
    { }

public:
    ~socket()
    {
        auto _ = std::lock_guard(this->_boost_mutex);
        this->close();
    }

protected:
    virtual bool on_encrypt(fb::stream& out)
    {
        return this->_crt.encrypt(out);
    }

protected:
    virtual bool on_wrap(fb::stream& out)
    {
        return this->_crt.wrap(out);
    }

public:
    async::task<size_t> send(const fb::stream& stream, bool encrypt = true, bool wrap = true)
    {
        auto promise = std::make_shared<async::task_completion_source<size_t>>();

        if (stream.empty())
        {
            promise->set_value(0);
            return promise->task();
        }

        auto clone = fb::stream(stream);
        if (encrypt && this->on_encrypt(clone) == false)
        {
            promise->set_exception(std::make_exception_ptr(std::runtime_error("unknown exception while send bytes")));
            return promise->task();
        }

        if (wrap && this->on_wrap(clone) == false)
        {
            promise->set_exception(std::make_exception_ptr(std::runtime_error("unknown exception while send bytes")));
            return promise->task();
        }

        auto buffer = boost::asio::buffer(clone.data(), clone.size());
        {
            auto _ = std::lock_guard(this->_boost_mutex);
            boost::asio::async_write(*this, buffer, [promise](const boost::system::error_code& ec, size_t transferred) {
                if (ec)
                    promise->set_exception(std::make_exception_ptr(std::runtime_error("boost async write failed")));
                else
                    promise->set_value(transferred);
            });
        }

        return promise->task();
    }

public:
    async::task<size_t> send(const fb::protocol::base::header& response, bool encrypt = true, bool wrap = true)
    {
        auto stream = fb::stream();
        auto writer = fb::stream_writer<big_endian>(stream);
        co_await response.serialize(writer);
        co_return co_await this->send(stream, encrypt, wrap);
    }

public:
    boost::asio::awaitable<void> recv()
    {
        try
        {
            while (true)
            {
                auto bytes_transferred =
                    co_await this->async_read_some(boost::asio::buffer(this->_buffer), boost::asio::use_awaitable);

                auto writer = fb::stream_writer<big_endian>(this->_stream);
                writer.write(this->_buffer.data(), bytes_transferred);

                async::awaitable_get(this->_handle_received(*this, this->_stream));
                if (this->is_open() == false)
                    throw std::runtime_error("disconnected");
            }
        }
        catch (boost::system::system_error& e)
        { }
        catch (boost::system::error_code& e)
        {
            auto ec = e.value();
            switch (ec)
            {
            case ENOENT:
                break;

            case ECONNABORTED:
#ifdef _WIN32
            case WSA_OPERATION_ABORTED:
#endif
                break;

            case ECONNRESET:
                fb::logger::info("SYSTEM SHUTDOWN ALERT?");
                break;

            default:
                break;
            }
        }
        catch (std::exception& e)
        {
            fb::logger::fatal(e.what());
        }
        catch (...)
        { }

        std::ignore = this->_handle_closed(*this);
    }

public:
    void data(T* value)
    {
        this->_data = value;
    }

public:
    T* data() const
    {
        return _data;
    }

public:
    std::string IP() const
    {
        return this->remote_endpoint().address().to_string();
    }

public:
    uint32_t fd() const
    {
        return (uint32_t)const_cast<fb::socket<T>*>(this)->native_handle();
    }

public:
    fb::cryptor& crt()
    {
        return this->_crt;
    }

public:
    void crt(const fb::cryptor& crt)
    {
        this->_crt = fb::cryptor(crt);
    }

public:
    void crt(uint8_t enctype, const uint8_t* enckey)
    {
        this->_crt = fb::cryptor(enctype, enckey);
    }

public:
    operator fb::cryptor& ()
    {
        return this->_crt;
    }

public:
    virtual fb::thread* thread() const override
    {
        if constexpr (std::is_base_of_v<fb::thread_switchable, T>)
        {
            if (this->_data != nullptr)
                return this->_data->thread();
            else
                return this->_context.threads.modular(this->fd());
        }
        else
        {
            return this->_context.threads.modular(this->fd());
        }
    }
};

template <typename T>
class acceptor;

template <typename T>
class socket_container
{
public:
    using handler_event = std::function<void(fb::socket<T>&)>;
    using socket_map    = std::unordered_map<uint32_t, std::unique_ptr<fb::socket<T>>>;

public:
    friend class acceptor<T>;

private:
    std::recursive_mutex _mutex;
    socket_map           _sockets;
    handler_event        _on_enter;
    handler_event        _on_leave;

public:
    socket_container()                        = default;
    socket_container(const socket_container&) = delete;
    socket_container(socket_container&&)      = delete;
    ~socket_container()                       = default;

public:
    size_t size()
    {
        return this->_sockets.size();
    }

    void on_enter(const handler_event& handler)
    {
        auto _          = std::lock_guard(this->_mutex);
        this->_on_enter = handler;
    }

    void on_leave(const handler_event& handler)
    {
        auto _          = std::lock_guard(this->_mutex);
        this->_on_leave = handler;
    }

private:
    void push(std::unique_ptr<fb::socket<T>>&& session)
    {
        auto _ = std::lock_guard(this->_mutex);

        auto fd = session->fd();
        this->_sockets.insert({fd, std::move(session)});
        if (this->_on_enter)
            this->_on_enter(*this->_sockets[fd]);
    }

    void erase(uint32_t fd)
    {
        auto _ = std::lock_guard(this->_mutex);

        if (this->_sockets.contains(fd) == false)
            return;

        auto ptr = std::move(this->_sockets[fd]);
        this->_sockets.erase(fd);

        if (this->_on_leave)
            this->_on_leave(*ptr);
    }

    void erase(fb::socket<T>& session)
    {
        this->erase(session.fd());
    }

    bool empty()
    {
        auto _ = std::lock_guard(this->_mutex);

        return this->_sockets.empty();
    }

public:
    bool contains(uint32_t fd)
    {
        auto _ = std::lock_guard(this->_mutex);

        return this->_sockets.contains(fd);
    }

    async::task<void> each(const std::function<async::task<void>(fb::socket<T>&)> fn)
    {
        auto _ = std::lock_guard(this->_mutex);
        for (auto& [fd, socket] : this->_sockets)
        {
            co_await fn(*socket);
        }
    }

    fb::socket<T>* find(const std::function<bool(fb::socket<T>&)> fn)
    {
        auto _ = std::lock_guard(this->_mutex);
        for (auto& [fd, socket] : this->_sockets)
        {
            if (fn(*socket))
                return socket.get();
        }

        return nullptr;
    }
    void close()
    {
        auto _ = std::lock_guard(this->_mutex);

        // auto empty = this->_sockets.empty();
        // if(empty)
        //     return;

        // for (auto& [k, v] : *this)
        //{
        //     try
        //     {
        //         v->close();
        //     }
        //     catch(std::exception& e)
        //     {
        //         std::cout << e.what() << std::endl;
        //     }
        // }

        this->_sockets.clear();
    }

public:
    fb::socket<T>* operator[] (uint32_t fd)
    {
        auto _ = std::lock_guard(this->_mutex);

        const auto& found = this->_sockets.find(fd);
        if (found == this->_sockets.cend())
            return nullptr;

        return found->second.get();
    }
};

} // namespace fb

#endif // !__SOCKET_H__
