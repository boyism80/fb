#include <fb/tcp_socket.h>

#include <cstddef>
#include <memory>
#include <mutex>
#include <utility>
#include <vector>

using namespace fb;

tcp_socket::tcp_socket(boost::asio::io_context& context) :
    boost::asio::ip::tcp::socket(context)
{ }

void tcp_socket::write(const std::shared_ptr<tcp_socket>& socket, fb::stream wire, write_handler handler)
{
    if (wire.empty())
    {
        if (handler != nullptr)
            handler(boost::system::error_code(), 0);
        return;
    }

    auto overflow = false;
    {
        auto lock = std::lock_guard(socket->_write_mutex);
        if (socket->_write_bytes + wire.size() > MAX_PENDING_WRITE_BYTES)
        {
            overflow = true;
        }
        else
        {
            socket->_write_bytes += wire.size();
            socket->_writes.push_back(
                pending_write{.wire = std::make_shared<fb::stream>(std::move(wire)), .handler = std::move(handler)});

            if (socket->_writes.size() == 1)
                tcp_socket::write_front(socket);
        }
    }

    if (overflow)
    {
        // Close on the socket's executor; the in-flight write then fails and drops the rest of the queue.
        boost::asio::post(socket->get_executor(), [socket]() {
            auto ec = boost::system::error_code();
            socket->close(ec);
        });

        if (handler != nullptr)
            handler(boost::asio::error::no_buffer_space, 0);
    }
}

// Requires socket->_write_mutex to be held by the caller.
void tcp_socket::write_front(const std::shared_ptr<tcp_socket>& socket)
{
    auto wire = socket->_writes.front().wire;
    boost::asio::async_write(*socket,
                             boost::asio::buffer(wire->data(), wire->size()),
                             [socket, wire](const boost::system::error_code& ec, size_t transferred) {
                                 auto completed = write_handler();
                                 auto dropped   = std::vector<write_handler>();
                                 {
                                     auto lock = std::lock_guard(socket->_write_mutex);
                                     completed = std::move(socket->_writes.front().handler);
                                     socket->_writes.pop_front();
                                     socket->_write_bytes -= wire->size();

                                     if (ec)
                                     {
                                         for (auto& pending : socket->_writes)
                                         {
                                             dropped.push_back(std::move(pending.handler));
                                         }
                                         socket->_writes.clear();
                                         socket->_write_bytes = 0;
                                     }
                                     else if (socket->_writes.empty() == false)
                                     {
                                         tcp_socket::write_front(socket);
                                     }
                                 }

                                 // Handlers run outside the lock because they may call write() again.
                                 if (completed != nullptr)
                                     completed(ec, transferred);

                                 for (auto& handler : dropped)
                                 {
                                     if (handler != nullptr)
                                         handler(ec, 0);
                                 }
                             });
}
