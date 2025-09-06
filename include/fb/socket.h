#ifndef __SOCKET_H__
#define __SOCKET_H__

#include <exception>
#include <unordered_map>
#include <deque>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <boost/system/error_code.hpp>
#include <fb/protocol/header.h>
#include <fb/encryption.h>
#include <fb/logger.h>
#include <async/task.h>
#include <async/task_completion_source.h>
#include <async/awaitable_get.h>
#include <fb/async_executor.h>
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

    class rate_limiter
    {
    private:
        struct tracker
        {
            fb::model::datetime last        = fb::model::datetime(); ///< Last reset time for rate limiting window
            uint32_t            transitions = 0;                     ///< Current number of transitions in the window
        };

        tracker                              _global;   ///< Global TPS tracker for all requests
        std::unordered_map<uint8_t, tracker> _commands; ///< Per-command TPS trackers

    public:
        rate_limiter() = default;

        ~rate_limiter() = default;

        // Delete copy constructor and copy assignment operator
        rate_limiter(const rate_limiter&)             = delete;
        rate_limiter& operator= (const rate_limiter&) = delete;

        // Delete move constructor and move assignment operator
        rate_limiter(rate_limiter&&)             = delete;
        rate_limiter& operator= (rate_limiter&&) = delete;

        bool update(uint32_t limit)
        {
            auto elapsed_time = fb::model::datetime() - this->_global.last;
            if (elapsed_time > 1s)
            {
                this->_global.last        = fb::model::datetime();
                this->_global.transitions = 0;
            }

            if (++this->_global.transitions > limit)
                return false;

            return true;
        }

        bool update(uint8_t cmd, const std::chrono::steady_clock::duration& duration, uint32_t limit)
        {
            if (!this->_commands.contains(cmd))
                this->_commands[cmd] = tracker{};

            auto& tracker = this->_commands[cmd];

            auto elapsed_time = fb::model::datetime() - tracker.last;
            if (elapsed_time > duration)
            {
                tracker.last        = fb::model::datetime();
                tracker.transitions = 0;
            }

            if (++tracker.transitions > limit)
                return false;

            return true;
        }
    };

private:
    fb::async_executor& _executor;
    fb::encryption      _encryption;
    handle_read_event   _handle_received;
    handler_event       _handle_closed;
    fb::stream          _stream;

protected:
    std::array<char, MAX_BUFFER_SIZE> _buffer;
    std::shared_ptr<T>                _data;

public:
    rate_limiter limiter;

public:
    socket(fb::async_executor& executor, const handle_read_event& handle_received, const handler_event& handle_closed) :
        boost::asio::ip::tcp::socket(static_cast<boost::asio::io_context&>(executor)),
        _executor(executor),
        _handle_received(handle_received),
        _handle_closed(handle_closed)
    { }

public:
    socket(fb::async_executor& executor, const fb::encryption& encryption, const handle_read_event& handle_received, const handler_event& handle_closed) :
        boost::asio::ip::tcp::socket(static_cast<boost::asio::io_context&>(executor)),
        _executor(executor),
        _handle_received(handle_received),
        _handle_closed(handle_closed),
        _encryption(encryption)
    { }

public:
    ~socket() = default;

protected:
    virtual bool on_encrypt(fb::stream& out)
    {
        return this->_encryption.encrypt(out);
    }

protected:
    virtual bool on_wrap(fb::stream& out)
    {
        return this->_encryption.wrap(out);
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
            boost::asio::async_write(*this, buffer, [promise](const boost::system::error_code& ec, size_t transferred) {
                if (ec)
                    promise->set_exception(std::make_exception_ptr(std::runtime_error(ec.message())));
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
                auto bytes_transferred = co_await this->async_read_some(boost::asio::buffer(this->_buffer), boost::asio::use_awaitable);

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
            if (ec == boost::asio::error::eof)
            {
                // do nothing
            }
            else if (ec == boost::asio::error::operation_aborted)
            {
                fb::logger::debug("socket recv cancelled (possibly shutdown)");
            }
            else if (ec == boost::asio::error::connection_reset)
            {
                fb::logger::debug("client disconnected (connection reset)");
            }
            else
            {
                // fb::logger::debug("recv error: {}", ec.message());
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
    void data(std::shared_ptr<T> value)
    {
        this->_data = value;
    }

public:
    std::shared_ptr<T> data_ptr() const
    {
        return this->_data;
    }

    T* data() const
    {
        if (this->_data == nullptr)
            return nullptr;

        return this->_data.get();
    }

public:
    uint32_t fd() const
    {
        return (uint32_t)const_cast<fb::socket<T>*>(this)->native_handle();
    }

public:
    fb::encryption& encryption()
    {
        return this->_encryption;
    }

public:
    void encryption(const fb::encryption& encryption)
    {
        this->_encryption = encryption;
    }

public:
    void encryption(uint8_t enctype, const uint8_t* enckey)
    {
        this->_encryption = fb::encryption(enctype, enckey);
    }

public:
    virtual fb::thread* thread() const override
    {
        if constexpr (std::is_base_of_v<fb::thread_switchable, T>)
        {
            if (this->_data != nullptr)
                return this->_data->thread();
            else
                return this->_executor.threads.modular(this->fd());
        }
        else
        {
            return this->_executor.threads.modular(this->fd());
        }
    }
};

} // namespace fb

#endif // !__SOCKET_H__
