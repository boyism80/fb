#ifndef __FB_AMQP_H__
#define __FB_AMQP_H__

#ifdef _WIN32
#include <WinSock2.h>
#else
#include <sys/time.h>
#endif
#include <async/awaitable_then.h>
#include <async/task.h>
#include <fb/stream.h>
#include <functional>
#include <memory>
#include <rabbitmq-c/amqp.h>
#include <rabbitmq-c/tcp_socket.h>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace fb { namespace amqp {

class queue;

class socket
{
private:
    amqp_socket_t*                      _socket = nullptr;
    amqp_connection_state_t             _conn   = nullptr;
    std::vector<std::unique_ptr<queue>> _queues;

public:
    socket();
    ~socket();

public:
    bool   connect(const std::string& hostname, uint16_t port, const std::string& id, const std::string& pw, const std::string& vhost);
    queue& declare_queue();
    operator amqp_connection_state_t ();
    bool select(const timeval* timeout = nullptr);
};

class queue
{
public:
    friend class socket;

public:
    using handle_func = std::function<async::task<void>(const uint8_t*)>;

private:
    socket&                                   _owner;
    std::unordered_map<uint32_t, handle_func> _handler;
    amqp_bytes_t                              _raw_name;
    std::string                               _name;
    amqp_bytes_t                              _raw_tag;
    std::string                               _tag;

private:
    queue(socket& owner, const amqp_bytes_t& name);
    queue(const queue&) = delete;

public:
    ~queue();

public:
    bool               bind(const std::string& exchange, const std::string& binding_key);
    const std::string& name() const;
    const std::string& consumer_tag() const;
    async::task<void>  invoke(const std::vector<uint8_t>& message);

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

}} // namespace fb::amqp

#endif