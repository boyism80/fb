#include <fb/tcp_socket.h>

#include <boost/asio/error.hpp>

#include <cstddef>
#include <memory>
#include <mutex>
#include <utility>
#include <vector>

using namespace fb;

tcp_socket::tcp_socket(boost::asio::io_context& context) :
    boost::asio::ip::tcp::socket(context),
    _writes(std::make_shared<write_queue>())
{
    this->_writes->owner = this;
}

tcp_socket::~tcp_socket()
{
    auto lock            = std::lock_guard(this->_writes->mutex);
    this->_writes->owner = nullptr;
}

void tcp_socket::write(fb::stream wire, write_handler handler)
{
    if (wire.empty())
    {
        if (handler != nullptr)
            handler(boost::system::error_code(), 0);
        return;
    }

    auto lock = std::lock_guard(this->_writes->mutex);
    this->_writes->entries.push_back(
        write_queue::entry{.wire = std::make_shared<fb::stream>(std::move(wire)), .handler = std::move(handler)});

    if (this->_writes->entries.size() == 1)
        tcp_socket::write_front(this->_writes);
}

// Requires queue->mutex to be held by the caller.
void tcp_socket::write_front(const std::shared_ptr<write_queue>& queue)
{
    auto wire = queue->entries.front().wire;
    boost::asio::async_write(*queue->owner,
                             boost::asio::buffer(wire->data(), wire->size()),
                             [queue, wire](const boost::system::error_code& ec, size_t transferred) {
                                 auto completed = write_handler();
                                 auto dropped   = std::vector<write_handler>();
                                 {
                                     auto lock = std::lock_guard(queue->mutex);
                                     completed = std::move(queue->entries.front().handler);
                                     queue->entries.pop_front();

                                     if (ec || queue->owner == nullptr)
                                     {
                                         for (auto& entry : queue->entries)
                                         {
                                             dropped.push_back(std::move(entry.handler));
                                         }
                                         queue->entries.clear();
                                     }
                                     else if (queue->entries.empty() == false)
                                     {
                                         tcp_socket::write_front(queue);
                                     }
                                 }

                                 // Handlers run outside the lock because they may call write() again.
                                 if (completed != nullptr)
                                     completed(ec, transferred);

                                 auto dropped_ec =
                                     ec ? ec : boost::system::error_code(boost::asio::error::operation_aborted);
                                 for (auto& handler : dropped)
                                 {
                                     if (handler != nullptr)
                                         handler(dropped_ec, 0);
                                 }
                             });
}
