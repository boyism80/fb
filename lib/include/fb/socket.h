#ifndef __SOCKET_H__
#define __SOCKET_H__

#include <exception>
#include <map>
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

namespace fb {

template <typename T = void*>
class socket : public boost::asio::ip::tcp::socket
{
public:
    using handler_event       = std::function<async::task<void>(fb::socket<T>&)>;
    using boost_send_callback = std::function<void(const boost::system::error_code&, size_t)>;

private:
    fb::cryptor   _crt;
    handler_event _handle_received;
    handler_event _handle_closed;
    uint32_t      _fd = 0xFFFFFFFF;
    fb::stream    _stream;

protected:
    std::array<char, 256> _buffer;
    T*                    _data;
    std::recursive_mutex  _boost_mutex;

public:
    socket(boost::asio::io_context& context, const handler_event& handle_received, const handler_event& handle_closed) :
        boost::asio::ip::tcp::socket(context),
        _handle_received(handle_received),
        _handle_closed(handle_closed)
    { }

public:
    socket(boost::asio::io_context& context,
           const fb::cryptor&       crt,
           const handler_event&     handle_received,
           const handler_event&     handle_closed) :
        boost::asio::ip::tcp::socket(context),
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
    void send(const fb::stream& stream, bool encrypt = true, bool wrap = true)
    {
        static auto empty_fn = [](const boost::system::error_code ec, size_t size) {

        };
        this->send(stream, encrypt, wrap, empty_fn);
    }

public:
    void send(const fb::stream& stream, bool encrypt, bool wrap, const boost_send_callback& callback)
    {
        if (stream.empty())
            return;

        auto clone = fb::stream(stream);
        if (encrypt && this->on_encrypt(clone) == false)
            return;

        if (wrap && this->on_wrap(clone) == false)
            return;

        auto buffer = boost::asio::buffer(clone.data(), clone.size());
        {
            auto _ = std::lock_guard(this->_boost_mutex);
            boost::asio::async_write(*this, buffer, callback);
        }
    }

public:
    void send(const fb::protocol::base::header& response, bool encrypt = true, bool wrap = true)
    {
        static auto empty_fn = [](const boost::system::error_code&, size_t) {
        };
        this->send(response, encrypt, wrap, empty_fn);
    }

public:
    void send(const fb::protocol::base::header& response, bool encrypt, bool wrap, const boost_send_callback& callback)
    {
        auto stream = fb::stream();
        auto writer = fb::stream_writer<big_endian>(stream);
        response.serialize(writer);
        this->send(stream, encrypt, wrap, callback);
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
                this->stream<void>([this, bytes_transferred](fb::stream& stream) {
                    auto writer = fb::stream_writer<big_endian>(stream);
                    writer.write(this->_buffer.data(), bytes_transferred);
                });

                async::awaitable_get(this->_handle_received(*this));
                if (this->is_open() == false)
                    throw std::runtime_error("disconnected");
            }
        }
        catch (std::exception& e)
        {
            fb::logger::fatal(e.what());
        }
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
        catch (...)
        { }

        async::awaitable_get(this->_handle_closed(*this));
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
    template <typename R = void>
    R stream(const std::function<R(fb::stream& stream)>& func)
    {
        if constexpr (std::is_void_v<T>)
        {
            func(this->_stream);
        }
        else
        {
            return func(this->_stream);
        }
    }
};

template <typename T>
class acceptor;

template <typename T>
class socket_container : private std::map<uint32_t, std::unique_ptr<fb::socket<T>>>
{
private:
    std::mutex mutex;

public:
    friend class acceptor<T>;

public:
    socket_container() = default;
    ~socket_container()
    {
        auto _ = std::lock_guard(this->mutex);
    }

public:
    using std::map<uint32_t, std::unique_ptr<fb::socket<T>>>::begin;
    using std::map<uint32_t, std::unique_ptr<fb::socket<T>>>::end;
    using std::map<uint32_t, std::unique_ptr<fb::socket<T>>>::size;

private:
    void push(std::unique_ptr<fb::socket<T>>&& session)
    {
        auto _ = std::lock_guard(this->mutex);

        auto fd = session->fd();
        std::map<uint32_t, std::unique_ptr<fb::socket<T>>>::insert(
            std::pair<uint32_t, std::unique_ptr<fb::socket<T>>>(fd, std::move(session)));
    }
    void erase(fb::socket<T>& session)
    {
        auto _ = std::lock_guard(this->mutex);

        std::map<uint32_t, std::unique_ptr<fb::socket<T>>>::erase(session.fd());
    }

    void erase(uint32_t fd)
    {
        auto _ = std::lock_guard(this->mutex);

        std::map<uint32_t, std::unique_ptr<fb::socket<T>>>::erase(fd);
    }
    bool empty()
    {
        auto _ = std::lock_guard(this->mutex);

        return std::map<uint32_t, std::unique_ptr<fb::socket<T>>>::empty();
    }

public:
    bool contains(uint32_t fd)
    {
        auto _ = std::lock_guard(this->mutex);

        return std::map<uint32_t, std::unique_ptr<fb::socket<T>>>::contains(fd);
    }

    void each(const std::function<void(fb::socket<T>&)> fn)
    {
        auto _ = std::lock_guard(this->mutex);
        for (auto& [fd, socket] : *this)
        {
            fn(*socket);
        }
    }

    fb::socket<T>* find(const std::function<bool(fb::socket<T>&)> fn)
    {
        auto _ = std::lock_guard(this->mutex);
        for (auto& [fd, socket] : *this)
        {
            if (fn(*socket))
                return socket.get();
        }

        return nullptr;
    }
    void close()
    {
        auto _ = std::lock_guard(this->mutex);

        // auto empty = std::map<uint32_t, std::unique_ptr<fb::socket<T>>>::empty();
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

        std::map<uint32_t, std::unique_ptr<fb::socket<T>>>::clear();
    }

public:
    fb::socket<T>* operator[] (uint32_t fd)
    {
        auto _ = std::lock_guard(this->mutex);

        const auto& found = std::map<uint32_t, std::unique_ptr<fb::socket<T>>>::find(fd);
        if (found == this->cend())
            return nullptr;

        return found->second.get();
    }
};

} // namespace fb

#endif // !__SOCKET_H__
