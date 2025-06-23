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

/**
 * @brief      High-performance TCP socket wrapper with encryption and rate limiting.
 *
 *             This template class extends boost::asio TCP socket functionality with
 *             built-in encryption/decryption support, automatic rate limiting (TPS),
 *             and thread-safe operations. It provides asynchronous I/O operations
 *             with customizable event handlers for received data and connection closure.
 *             The socket integrates with the game server's threading system and
 *             supports protocol-aware data streaming with automatic packet wrapping.
 *
 * @tparam     T     The type of session data associated with this socket connection.
 *                   This allows each socket to carry custom state/context information.
 */
template <typename T = void*>
class socket : public boost::asio::ip::tcp::socket, public thread_switchable
{
public:
    static constexpr uint32_t MAX_BUFFER_SIZE = 256;

public:
    /**
     * @brief      The type of the read event handler.
     */
    using handle_read_event = std::function<async::task<void>(fb::socket<T>&, fb::stream&)>;
    /**
     * @brief      The type of the event handler.
     */
    using handler_event = std::function<async::task<void>(fb::socket<T>&)>;

    /**
     * @brief      Rate limiter for managing TPS (Transactions Per Second) limits.
     *
     *             This nested class handles both global socket TPS and per-command TPS limiting
     *             using a unified tracking structure. It provides overloaded update() methods
     *             for different use cases and tracks request counts with timestamps to enforce
     *             rate limits effectively. This class is non-copyable and non-movable to ensure
     *             data integrity.
     */
    class rate_limiter
    {
    private:
        /**
         * @brief      Structure for tracking TPS measurements per command.
         */
        struct tracker
        {
            fb::model::datetime last        = fb::model::datetime(); ///< Last reset time for rate limiting window
            uint32_t            transitions = 0;                     ///< Current number of transitions in the window
        };

        tracker                              _global;   ///< Global TPS tracker for all requests
        std::unordered_map<uint8_t, tracker> _commands; ///< Per-command TPS trackers

    public:
        /**
         * @brief      Default constructor.
         */
        rate_limiter() = default;

        /**
         * @brief      Destructor.
         */
        ~rate_limiter() = default;

        // Delete copy constructor and copy assignment operator
        rate_limiter(const rate_limiter&)             = delete;
        rate_limiter& operator= (const rate_limiter&) = delete;

        // Delete move constructor and move assignment operator
        rate_limiter(rate_limiter&&)             = delete;
        rate_limiter& operator= (rate_limiter&&) = delete;

        /**
         * @brief      Updates and checks the global TPS limit for all requests.
         *
         * @param[in]  limit  The maximum number of requests allowed per second.
         *
         * @return     True if within limits, false if rate limited.
         */
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

        /**
         * @brief      Updates and checks the TPS limit for a specific command.
         *
         * @param[in]  cmd       The command byte to track.
         * @param[in]  duration  The time window for rate limiting.
         * @param[in]  limit     The maximum transitions allowed in the window.
         *
         * @return     True if within limits, false if rate limited.
         */
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
    context&          _context;
    fb::crypto        _crypto;
    handle_read_event _handle_received;
    handler_event     _handle_closed;
    fb::stream        _stream;

protected:
    std::array<char, MAX_BUFFER_SIZE> _buffer;
    std::shared_ptr<T>                _data;

public:
    /**
     * @brief      Rate limiter instance for this socket.
     *
     *             Provides direct access to rate limiting functionality with two update methods:
     *             - update(limit): Global TPS limiting for all requests
     *             - update(cmd, duration, limit): Per-command TPS limiting with custom windows
     *
     *             This instance is unique per socket and cannot be copied or moved.
     */
    rate_limiter limiter;

public:
    /**
     * @brief      Constructs a new instance.
     *
     * @param[in]  context  The context.
     * @param[in]  handle_received  The read event handler.
     * @param[in]  handle_closed  The closed event handler.
     */
    socket(context& context, const handle_read_event& handle_received, const handler_event& handle_closed) :
        boost::asio::ip::tcp::socket(static_cast<boost::asio::io_context&>(context)),
        _context(context),
        _handle_received(handle_received),
        _handle_closed(handle_closed)
    { }

public:
    /**
     * @brief      Constructs a new instance.
     *
     * @param[in]  context  The context.
     * @param[in]  crt  The crypto.
     * @param[in]  handle_received  The read event handler.
     * @param[in]  handle_closed  The closed event handler.
     */
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
    /**
     * @brief      Destroys the socket.
     */
    ~socket() = default;

protected:
    /**
     * @brief      Encrypts the stream.
     *
     * @param[in]  out  The stream to encrypt.
     *
     * @return     True if the stream was encrypted successfully, false otherwise.
     */
    virtual bool on_encrypt(fb::stream& out)
    {
        return this->_crypto.encrypt(out);
    }

protected:
    /**
     * @brief      Wraps the stream.
     *
     * @param[in]  out  The stream to wrap.
     *
     * @return     True if the stream was wrapped successfully, false otherwise.
     */
    virtual bool on_wrap(fb::stream& out)
    {
        return this->_crypto.wrap(out);
    }

public:
    /**
     * @brief      Sends a stream.
     *
     * @param[in]  stream  The stream to send.
     * @param[in]  encrypt  Whether to encrypt the stream.
     * @param[in]  wrap  Whether to wrap the stream.
     *
     * @return     A task that will complete when the stream is sent.
     */
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
                    promise->set_exception(std::make_exception_ptr(std::runtime_error("boost async write failed")));
                else
                    promise->set_value(transferred);
            });
        }

        return promise->task();
    }

public:
    /**
     * @brief      Gets the IP address of the socket.
     *
     * @return     The IP address of the socket.
     */
    std::string ip() const
    {
        return this->remote_endpoint().address().to_string();
    }

    /**
     * @brief      Gets the IP address of the socket as a raw integer.
     *
     * @return     The IP address of the socket as a raw integer.
     */
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

    /**
     * @brief      Gets the port of the socket.
     *
     * @return     The port of the socket.
     */
    uint16_t port() const
    {
        return this->remote_endpoint().port();
    }

public:
    /**
     * @brief      Sends a protocol header.
     *
     * @param[in]  response  The response to send.
     * @param[in]  encrypt  Whether to encrypt the response.
     * @param[in]  wrap  Whether to wrap the response.
     *
     * @return     A task that will complete when the header is sent.
     */
    async::task<size_t> send(const fb::protocol::header& response, bool encrypt = true, bool wrap = true)
    {
        auto stream = fb::stream();
        auto writer = fb::stream_writer<big_endian>(stream);
        co_await response.serialize(writer);
        co_return co_await this->send(stream, encrypt, wrap);
    }

public:
    /**
     * @brief      Receives data from the socket.
     *
     * @return     A task that will complete when the data is received.
     */
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
    /**
     * @brief      Sets the data.
     *
     * @param[in]  value  The data.
     */
    void data(std::shared_ptr<T> value)
    {
        this->_data = value;
    }

public:
    /**
     * @brief      Gets the data.
     *
     * @return     The data.
     */
    std::shared_ptr<T> data() const
    {
        return this->_data;
    }

public:
    /**
     * @brief      Gets the file descriptor of the socket.
     *
     * @return     The file descriptor of the socket.
     */
    uint32_t fd() const
    {
        return (uint32_t)const_cast<fb::socket<T>*>(this)->native_handle();
    }

public:
    /**
     * @brief      Gets the crypto.
     *
     * @return     The crypto.
     */
    fb::crypto& crt()
    {
        return this->_crypto;
    }

public:
    /**
     * @brief      Sets the crypto.
     *
     * @param[in]  crt  The crypto.
     */
    void crt(const fb::crypto& crt)
    {
        this->_crypto = fb::crypto(crt);
    }

public:
    /**
     * @brief      Sets the crypto.
     *
     * @param[in]  enctype  The encryption type.
     * @param[in]  enckey  The encryption key.
     */
    void crt(uint8_t enctype, const uint8_t* enckey)
    {
        this->_crypto = fb::crypto(enctype, enckey);
    }

public:
    /**
     * @brief      Gets the thread associated with the socket.
     *
     * @return     The thread associated with the socket.
     */
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
