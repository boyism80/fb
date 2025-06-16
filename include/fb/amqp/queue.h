#ifndef __AMQP_QUEUE_H__
#define __AMQP_QUEUE_H__

#include <fb/amqp/socket.h>
#include <fb/stream_reader.h>
#include <functional>
#include <async/task.h>

namespace fb::amqp {

/**
 * @brief      AMQP message queue for inter-service communication.
 *
 *             This class represents a message queue within an AMQP connection.
 *             It provides functionality for binding to exchanges, consuming messages,
 *             and handling different message types with type-safe handlers. Used for
 *             asynchronous communication between different server components.
 */
class queue
{
public:
    friend class socket;

public:
    using handle_func      = std::function<async::task<void>(const uint8_t*)>; ///< Message handler function type
    using handle_container = std::unordered_map<uint32_t, handle_func>;        ///< Container for message handlers

private:
    socket&          _owner;    ///< Reference to the owning AMQP socket
    handle_container _handler;  ///< Map of command types to handler functions
    amqp_bytes_t     _raw_name; ///< Raw AMQP queue name bytes
    std::string      _name;     ///< Queue name as string
    amqp_bytes_t     _raw_tag;  ///< Raw AMQP consumer tag bytes
    std::string      _tag;      ///< Consumer tag as string
    std::string      _route;    ///< Routing key for this queue

private:
    /**
     * @brief      Constructs a new AMQP queue (private, called by socket).
     *
     * @param      owner  The AMQP socket that owns this queue.
     * @param[in]  name   The raw AMQP queue name bytes.
     */
    queue(socket& owner, const amqp_bytes_t& name);

    /**
     * @brief      Copy constructor is deleted to prevent copying.
     */
    queue(const queue&) = delete;

public:
    /**
     * @brief      Destroys the AMQP queue and cleans up resources.
     */
    ~queue();

public:
    bool                            bind(const std::string& exchange, const std::string& route);
    const std::string&              name() const;
    const std::string&              route() const;
    const std::string&              consumer_tag() const;
    [[nodiscard]] async::task<void> invoke(const std::vector<uint8_t>& message);

    template <typename R>
    void handler(const std::function<async::task<void>(R&)>& fn)
    {
        auto cmd = static_cast<uint32_t>(R::FlatBufferProtocolType);
        this->handler(cmd, [this, fn](const uint8_t* ptr) -> async::task<void> {
            auto protocol = R::Deserialize(ptr);
            co_await fn(protocol);
        });
    }

    void handler(uint32_t cmd, const handle_func& fn);
};

} // namespace fb::amqp

#endif