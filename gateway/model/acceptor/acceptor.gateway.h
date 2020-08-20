#ifndef __FB_GATEWAY_H__
#define __FB_GATEWAY_H__

#include <module/acceptor/acceptor.h>
#include <module/config/config.h>
#include <model/session/session.gateway.h>
#include <model/gateway/gateway.h>
#include <service/gateway/service.gateway.h>
#include <zlib/zlib.h>
#include <protocol/gateway.h>

namespace fb { namespace gateway {

class acceptor : public fb::acceptor<session>
{
private:
    host_entries                _entries;
    fb::ostream                 _entry_stream_cache;
    uint32_t                    _entry_crc32_cache;

private:
    service::gateway            _gateway_service;

public:
    acceptor(boost::asio::io_context& context, uint16_t port);
    ~acceptor();

private:
    bool                        load_entries();

private:
    fb::ostream                 make_crt_stream(const fb::cryptor& crt);

    // override
public:
    fb::gateway::session*       handle_alloc_session(fb::socket* socket);

public:
    bool                        handle_connected(fb::gateway::session& session);
    bool                        handle_disconnected(fb::gateway::session& session);

public:
    bool                        handle_check_version(fb::gateway::session& session, const fb::protocol::request::gateway::assert_version&);
    bool                        handle_entry_list(fb::gateway::session& session, const fb::protocol::request::gateway::entry_list&);
};

} }

#endif // !__FB_GATEWAY_H__
