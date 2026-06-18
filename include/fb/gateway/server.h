#ifndef __FB_GATEWAY_H__
#define __FB_GATEWAY_H__

#include <fb/acceptor.h>
#include <fb/gateway/session.h>
#include <fb/gateway/protocol.h>
#include <fb/protocol/flatbuffer/protocol.h>
#include <fb/log_collector.h>
#include <memory>

REGISTER_RESPONSE(fb::protocol::internal::request::Heartbeat, fb::protocol::internal::response::Heartbeat)

namespace fb::gateway {

using namespace fb::protocol::internal;
using namespace fb::protocol::internal::request;
using namespace fb::protocol::gateway;

namespace internal      = fb::protocol::internal;
namespace internal_resp = fb::protocol::internal::response;

class server : public fb::acceptor<fb::gateway::session>
{
private:
    std::vector<endpoint> _entrypoints;
    fb::stream            _endpoint_bytes;
    uint32_t              _endpoint_crc = 0;
    fb::stream            _connection_welcome;

public:
    fb::log_collector log;

public:
    server(boost::asio::io_context& io_context, uint16_t port);
    server(const server&) = delete;
    server(server&&)      = delete;
    ~server();
    const std::vector<endpoint>& entrypoints() const;
    const fb::stream&            endpoint_bytes() const;
    uint32_t                     endpoint_crc() const;

private:
    [[nodiscard]] async::task<void> load_entries();

private:
    fb::stream make_crt_stream(const fb::encryption& encryption);
    // override

protected:
    bool                            decrypt_policy(uint8_t opcode) const override final;
    void                            on_init_amqp(fb::amqp::socket& amqp) override final;
    [[nodiscard]] async::task<void> on_start() override final;
    [[nodiscard]] async::task<void> on_accepted(fb::socket<fb::gateway::session>& socket) override final;
    [[nodiscard]] async::task<bool> on_connected(fb::socket<fb::gateway::session>& session) override final;
    [[nodiscard]] async::task<bool> on_disconnected(fb::socket<fb::gateway::session>& session) override final;

protected:
    Service service() const override final
    {
        return Service::Gateway;
    }

public:
    async::task<void> update_status();
};

} // namespace fb::gateway

#endif // !__FB_GATEWAY_H__
