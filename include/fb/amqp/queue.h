#ifndef __AMQP_QUEUE_H__
#define __AMQP_QUEUE_H__

#include <fb/amqp/socket.h>
#include <fb/stream_reader.h>
#include <functional>
#include <async/task.h>

namespace fb::amqp {

class queue
{
public:
    friend class socket;

public:
    using handle_func      = std::function<async::task<void>(const uint8_t*)>;
    using handle_container = std::unordered_map<uint32_t, handle_func>;

private:
    socket&          _owner;
    handle_container _handler;
    amqp_bytes_t     _raw_name;
    std::string      _name;
    amqp_bytes_t     _raw_tag;
    std::string      _tag;

private:
    queue(socket& owner, const amqp_bytes_t& name);
    queue(const queue&) = delete;

public:
    ~queue();

public:
    bool                            bind(const std::string& exchange, const std::string& binding_key);
    const std::string&              name() const;
    const std::string&              consumer_tag() const;
    [[nodiscard]] async::task<void> invoke(const std::vector<uint8_t>& message);

    template <typename R>
    void handler(const std::function<async::task<void>(R&)>& fn)
    {
        auto cmd = (uint32_t)R::FlatBufferProtocolType;
        this->_handler.insert({cmd, [this, fn](const uint8_t* ptr) -> async::task<void> {
                                   auto protocol = R::Deserialize(ptr);
                                   co_await fn(protocol);
                               }});
    }
};

} // namespace fb::amqp

#endif