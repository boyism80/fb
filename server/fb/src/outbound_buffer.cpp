#include <fb/logger.h>
#include <fb/outbound_buffer.h>

#include <boost/asio.hpp>
#include <utility>

using namespace fb;

uint32_t outbound_buffer::endpoint_key(boost::asio::ip::tcp::socket& endpoint)
{
    return static_cast<uint32_t>(endpoint.native_handle());
}

void outbound_buffer::write(std::shared_ptr<boost::asio::ip::tcp::socket> endpoint, std::shared_ptr<fb::stream> wire)
{
    boost::asio::async_write(*endpoint,
                             boost::asio::buffer(wire->data(), wire->size()),
                             [endpoint, wire](const boost::system::error_code& ec, std::size_t transferred) {
                                 if (ec)
                                 {
                                     fb::logger::debug("outbound_buffer flush failed: {}", ec.message());
                                     return;
                                 }

                                 std::ignore = transferred;
                             });
}

void outbound_buffer::append(std::shared_ptr<boost::asio::ip::tcp::socket> endpoint, fb::stream wire)
{
    if (endpoint == nullptr || wire.empty())
        return;

    if (endpoint->is_open() == false)
        return;

    const auto key  = outbound_buffer::endpoint_key(*endpoint);
    auto&      slot = this->_pending[key];

    if (slot.endpoint == nullptr)
        slot.endpoint = std::move(endpoint);

    slot.wire.insert(slot.wire.end(), wire.begin(), wire.end());
}

void outbound_buffer::flush()
{
    if (this->_pending.empty())
        return;

    auto pending = std::move(this->_pending);
    this->_pending.clear();

    for (auto& [key, slot] : pending)
    {
        std::ignore = key;

        if (slot.wire.empty())
            continue;

        if (slot.endpoint == nullptr || slot.endpoint->is_open() == false)
            continue;

        auto wire = std::make_shared<fb::stream>(std::move(slot.wire));
        this->write(slot.endpoint, std::move(wire));
    }
}
