#include <fb/outbound_buffer.h>

#include <fb/logger.h>

#include <boost/asio.hpp>

#include <cstddef>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <utility>

using namespace fb;

struct outbound_buffer::state
{
    struct slot
    {
        std::shared_ptr<fb::tcp_socket> endpoint;
        fb::stream                      wire;
    };

    // Keyed by socket address: the slot holds a shared_ptr, so the address cannot be reused while pending.
    std::unordered_map<fb::tcp_socket*, slot> pending;
    std::recursive_mutex                      mutex;
};

outbound_buffer::outbound_buffer() :
    _state(std::make_shared<state>())
{ }

outbound_buffer::~outbound_buffer() = default;

void outbound_buffer::append(std::shared_ptr<fb::tcp_socket> endpoint, fb::stream wire)
{
    if (endpoint == nullptr || wire.empty())
        return;

    if (endpoint->is_open() == false)
        return;

    std::lock_guard lock(this->_state->mutex);

    auto& slot = this->_state->pending[endpoint.get()];
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

    for (auto& [_, slot] : pending)
    {
        if (slot.wire.empty())
            continue;

        if (slot.endpoint == nullptr || slot.endpoint->is_open() == false)
            continue;

        fb::tcp_socket::write(slot.endpoint, std::move(slot.wire), [](const boost::system::error_code& ec, size_t) {
            if (ec)
                fb::logger::debug("outbound_buffer flush failed: {}", ec.message());
        });
    }
}
