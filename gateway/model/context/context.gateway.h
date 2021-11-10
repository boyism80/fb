#ifndef __FB_GATEWAY_H__
#define __FB_GATEWAY_H__

#include "module/acceptor/acceptor.h"
#include "module/config/config.h"
#include "model/session/session.gateway.h"
#include "model/gateway/gateway.h"
#include <service/gateway/service.gateway.h>
#include <zlib.h>
#include <memory>
#include "protocol/gateway.h"
#include "protocol/internal.h"
#include "module/encoding/encoding.h"
#include "module/console/console.h"

namespace fb { namespace gateway {

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

private:
    service::gateway            _gateway_service;

public:
    context(boost::asio::io_context& context, uint16_t port, std::chrono::seconds delay, const INTERNAL_CONNECTION& internal_connection);
    ~context();

private:
    bool                        load_entries();

private:
    fb::ostream                 make_crt_stream(const fb::cryptor& crt);

    // override
public:
    fb::gateway::session*       handle_accepted(fb::socket<fb::gateway::session>& socket);

public:
    bool                        handle_connected(fb::socket<fb::gateway::session>& session);
    bool                        handle_disconnected(fb::socket<fb::gateway::session>& session);

public:
    bool                        handle_check_version(fb::socket<fb::gateway::session>& session, const fb::protocol::gateway::request::assert_version&);
    bool                        handle_entry_list(fb::socket<fb::gateway::session>& session, const fb::protocol::gateway::request::entry_list&);

public:
    bool                        handle_in_shutdown(fb::internal::socket<>&, const fb::protocol::internal::response::shutdown&);
};

} }

#endif // !__FB_GATEWAY_H__
