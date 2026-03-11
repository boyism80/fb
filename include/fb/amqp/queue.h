#ifndef __AMQP_QUEUE_H__
#define __AMQP_QUEUE_H__

#include <fb/amqp/socket.h>
#include <fb/stream_reader.h>
#include <functional>
#include <async/task.h>
#include <fb/thread_container.h>

namespace fb::amqp {

class queue
{
public:
    friend class socket;

public:
    using handle_func      = std::function<async::task<void>(const uint8_t*)>;
    using handle_container = std::unordered_map<uint32_t, handle_func>;

private:
    socket&               _owner;
    handle_container      _handler;
    amqp_bytes_t          _raw_name;
    std::string           _name;
    amqp_bytes_t          _raw_tag;
    std::string           _tag;
    std::string           _route;
    fb::thread_container& _threads;

private:
    queue(socket& owner, const amqp_bytes_t& name, fb::thread_container& threads);
    queue(const queue&) = delete;

public:
    ~queue();

public:
    bool                            bind(std::string_view exchange, std::string_view route);
    const std::string&              name() const;
    const std::string&              route() const;
    const std::string&              consumer_tag() const;
    [[nodiscard]] async::task<void> invoke(const std::vector<uint8_t>& message);
    void                            invoke_async(const std::vector<uint8_t>& message);

    template <typename R>
    void handler(std::function<async::task<void>(R&)> fn)
    {
        auto opcode = static_cast<uint32_t>(R::FlatBufferProtocolType);
        this->handler(opcode, [this, fn](const uint8_t* ptr) -> async::task<void> {
            auto protocol = R::Deserialize(ptr);
            co_await fn(protocol);
        });
    }

    void handler(uint32_t opcode, handle_func&& fn);
};

} // namespace fb::amqp

#endif