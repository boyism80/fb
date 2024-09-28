#ifndef __FB_AMQP_H__
#define __FB_AMQP_H__

extern "C" 
{
#include <rabbitmq-c/amqp.h>
#include <rabbitmq-c/tcp_socket.h>
}
#include <string>
#include <vector>
#include <stdexcept>
#include <memory>
#include <functional>
#include <async/task.h>
#include <async/awaitable_then.h>
#ifdef _WIN32
#include <WinSock2.h>
#else
#include <sys/time.h>
#endif

namespace fb { namespace amqp {

class queue;

class socket
{
private:
    amqp_socket_t*                              _socket = nullptr;
    amqp_connection_state_t                     _conn = nullptr;
    std::vector<std::unique_ptr<queue>>         _queues;

public:
    socket();
    ~socket();

public:
    bool                                        connect(const std::string& hostname, uint16_t port, const std::string& id, const std::string& pw, const std::string& vhost);
    queue&                                      declare_queue();
    operator                                    amqp_connection_state_t();
    bool                                        select(const timeval* timeout = nullptr);
};


class queue
{
public:
    using func_type = std::function<async::task<void>(const std::vector<uint8_t>&)>;

    friend class socket;

private:
    socket&                                     _owner;
    amqp_bytes_t                                _raw_name;
    std::string                                 _name;
    amqp_bytes_t                                _raw_tag;
    std::string                                 _tag;
    func_type                                   _func;

private:
    queue(socket& owner, const amqp_bytes_t& name);
    queue(const queue&) = delete;
public:
    ~queue();

public:
    bool                                        bind(const std::string& exchange, const std::string& binding_key, const func_type& func);
    const std::string&                          name() const;
    const std::string&                          consumer_tag() const;
    async::task<void>                           invoke(const std::vector<uint8_t>& message);
};

} }

#endif