#ifndef __FB_TCP_SOCKET_H__
#define __FB_TCP_SOCKET_H__

#include <fb/stream.h>

#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>

#include <cstddef>
#include <deque>
#include <functional>
#include <memory>
#include <mutex>

namespace fb {

// Serializes writes so that only one async_write is in flight per socket.
// Overlapping async_write calls may interleave bytes on partial writes.
class tcp_socket : public boost::asio::ip::tcp::socket
{
public:
    using write_handler = std::function<void(const boost::system::error_code&, size_t)>;

private:
    struct pending_write
    {
        std::shared_ptr<fb::stream> wire;
        write_handler               handler;
    };

    std::mutex                _write_mutex;
    std::deque<pending_write> _writes;

public:
    explicit tcp_socket(boost::asio::io_context& context);
    ~tcp_socket() = default;

    tcp_socket(const tcp_socket&)             = delete;
    tcp_socket& operator= (const tcp_socket&) = delete;

public:
    // Takes shared ownership so the socket outlives every in-flight async_write.
    static void write(const std::shared_ptr<tcp_socket>& socket, fb::stream wire, write_handler handler = nullptr);

private:
    static void write_front(const std::shared_ptr<tcp_socket>& socket);
};

} // namespace fb

#endif // !__FB_TCP_SOCKET_H__
