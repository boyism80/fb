#ifndef __FB_LOGIN_H__
#define __FB_LOGIN_H__

#include <module/acceptor/acceptor.h>
#include <model/session/session.login.h>
#include <service/auth/auth.service.h>
#include <model/gateway/gateway.h>
#include <zlib/zlib.h>
#include <protocol/login.h>

namespace fb { namespace login {

class acceptor : public fb::acceptor<session>
{
private:
    gateways                    _gateways;
    fb::ostream                 _gateway_stream_cache;
    uint32_t                    _gateway_crc32_cache;
    fb::ostream                 _agreement_stream_cache;

private:
    service::auth               _auth_service;

public:
    acceptor(boost::asio::io_context& context, uint16_t port);
    ~acceptor();

private:
    bool                        load_agreement();

private:
    const fb::ostream           make_message_stream(int type, const char* msg) const;

    // override
public:
    login::session*             handle_alloc_session(fb::socket* socket);

public:
    bool                        handle_connected(fb::login::session& session);
    bool                        handle_disconnected(fb::login::session& session);

public:
    bool                        handle_agreement(fb::login::session& session, const fb::protocol::request::login::agreement&);
    bool                        handle_create_account(fb::login::session& session, const fb::protocol::request::login::account::create&);
    bool                        handle_account_complete(fb::login::session& session, const fb::protocol::request::login::account::complete&);
    bool                        handle_login(fb::login::session& session, const fb::protocol::request::login::login&);
    bool                        handle_change_password(fb::login::session& session, const fb::protocol::request::login::account::change_pw&);
};

} }

#endif // !__FB_LOGIN_H__
