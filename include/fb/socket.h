#ifndef __SOCKET_H__
#define __SOCKET_H__

#include <exception>
#include <unordered_map>
#include <deque>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <boost/system/error_code.hpp>
#include <fb/protocol/header.h>
#include <fb/crypto.h>
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
    static constexpr uint32_t MAX_BUFFER_SIZE = 256;

public:
    using handle_read_event = std::function<async::task<void>(fb::socket<T>&, fb::stream&)>;
    using handler_event     = std::function<async::task<void>(fb::socket<T>&)>;

private:
    context&            _context;
    fb::crypto          _crypto;
    handle_read_event   _handle_received;
    handler_event       _handle_closed;
    fb::stream          _stream;
    uint32_t            _tps = 0;
    fb::model::datetime _last_tps_time;

protected:
    std::array<char, MAX_BUFFER_SIZE> _buffer;
    T*                                _data;
    std::recursive_mutex              _boost_mutex;

public:
    socket(context& context, const handle_read_event& handle_received, const handler_event& handle_closed) :
        boost::asio::ip::tcp::socket(static_cast<boost::asio::io_context&>(context)),
        _context(context),
        _handle_received(handle_received),
        _handle_closed(handle_closed)
    { }

public:
    socket(context&                 context,
           const fb::crypto&        crt,
           const handle_read_event& handle_received,
           const handler_event&     handle_closed) :
        boost::asio::ip::tcp::socket(static_cast<boost::asio::io_context&>(context)),
        _context(context),
        _handle_received(handle_received),
        _handle_closed(handle_closed),
        _crypto(crt)
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
        return this->_crypto.encrypt(out);
    }

protected:
    virtual bool on_wrap(fb::stream& out)
    {
        return this->_crypto.wrap(out);
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
    std::string ip() const
    {
        return this->remote_endpoint().address().to_string();
    }

    uint32_t ip_raw() const
    {
        try
        {
            auto addr = this->remote_endpoint().address();
            if (addr.is_v4())
                return addr.to_v4().to_uint();
            else
                return 0;
        }
        catch (...)
        {
            return 0;
        }
    }

    uint16_t port() const
    {
        return this->remote_endpoint().port();
    }

public:
    async::task<size_t> send(const fb::protocol::header& response, bool encrypt = true, bool wrap = true)
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
        {
            auto ec = e.code();

            if (ec == boost::asio::error::operation_aborted)
            {
                fb::logger::info("socket recv cancelled (possibly shutdown)");
            }
            else if (ec == boost::asio::error::connection_reset)
            {
                fb::logger::info("client disconnected (connection reset)");
            }
            else
            {
                fb::logger::fatal("recv error: {}", ec.message());
            }
        }
        catch (std::exception& e)
        {
            fb::logger::fatal(e.what());
        }
        catch (...)
        {
            fb::logger::fatal("unknown error in socket::recv");
        }

        try
        {
            std::ignore = this->_handle_closed(*this);
        }
        catch (std::exception& e)
        {
            fb::logger::fatal("handle_closed exception: {}", e.what());
        }
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
    uint32_t fd() const
    {
        return (uint32_t)const_cast<fb::socket<T>*>(this)->native_handle();
    }

public:
    fb::crypto& crt()
    {
        return this->_crypto;
    }

public:
    void crt(const fb::crypto& crt)
    {
        this->_crypto = fb::crypto(crt);
    }

public:
    void crt(uint8_t enctype, const uint8_t* enckey)
    {
        this->_crypto = fb::crypto(enctype, enckey);
    }

public:
    bool update_tps(uint32_t limit)
    {
        auto elapsed_time = fb::model::datetime() - this->_last_tps_time;
        if (elapsed_time.total_milliseconds() > 1000)
        {
            this->_last_tps_time = fb::model::datetime();
            this->_tps           = 0;
        }

        if (++this->_tps > limit)
            return false;

        return true;
    }

public:
    operator fb::crypto& ()
    {
        return this->_crypto;
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

} // namespace fb

#endif // !__SOCKET_H__
