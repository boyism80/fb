#ifndef __FB_GATEWAY_H__
#define __FB_GATEWAY_H__

#include <fb/core/acceptor.h>
#include <fb/core/config.h>
#include <fb/gateway/session.h>
#include <fb/gateway/util.h>
#include <zlib.h>
#include <memory>
#include <fb/protocol/gateway.h>
#include <fb/protocol/internal.h>
#include <fb/core/encoding.h>

namespace fb { namespace gateway {

using namespace fb::protocol::gateway;

class context : public fb::acceptor<fb::gateway::session>
{
public:
    using unique_session = std::unique_ptr<fb::gateway::session>;

private:
    std::vector<entry>          _entrypoints;
    fb::ostream                 _entry_stream_cache;
    uint32_t                    _entry_crc32_cache = 0;
    fb::ostream                 _connection_cache;
    std::vector<unique_session> _sessions;

public:
    context(boost::asio::io_context& context, uint16_t port, std::chrono::seconds delay);
    ~context();

private:
    bool                        load_entries();

private:
    fb::ostream                 make_crt_stream(const fb::cryptor& crt);

    // override
protected:
    fb::gateway::session*       handle_accepted(fb::socket<fb::gateway::session>& socket) final;
    bool                        handle_connected(fb::socket<fb::gateway::session>& session) final;
    bool                        handle_disconnected(fb::socket<fb::gateway::session>& session) final;
    void                        handle_internal_connected() final;

public:
    bool                        handle_check_version(fb::socket<fb::gateway::session>& session, const fb::protocol::gateway::request::assert_version&);
    bool                        handle_entry_list(fb::socket<fb::gateway::session>& session, const fb::protocol::gateway::request::entry_list&);

public:
    bool                        handle_in_shutdown(fb::internal::socket<>&, const fb::protocol::internal::response::shutdown&);
};

} }

#endif // !__FB_GATEWAY_H__
