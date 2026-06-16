#include <fb/logger.h>
#include <fb/outbound_buffer.h>

#include <boost/asio.hpp>
#include <mutex>
#include <unordered_map>
#include <utility>

using namespace fb;

struct outbound_buffer::state
{
    struct slot
    {
        std::shared_ptr<boost::asio::ip::tcp::socket> endpoint;
        fb::stream                                    wire;
    };

    std::unordered_map<uint32_t, slot> pending;
    std::recursive_mutex               mutex;
};

outbound_buffer::outbound_buffer() :
    _state(std::make_shared<state>())
{ }

outbound_buffer::~outbound_buffer() = default;

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

    const auto key = outbound_buffer::endpoint_key(*endpoint);

    std::lock_guard lock(this->_state->mutex);

    auto& slot = this->_state->pending[key];
    if (slot.endpoint == nullptr)
        slot.endpoint = std::move(endpoint);

    slot.wire.insert(slot.wire.end(), wire.begin(), wire.end());
}

void outbound_buffer::flush()
{
    std::lock_guard lock(this->_state->mutex);

    if (this->_state->pending.empty())
        return;

    auto pending = std::move(this->_state->pending);
    this->_state->pending.clear();

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
