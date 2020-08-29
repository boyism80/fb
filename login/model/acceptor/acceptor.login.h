#ifndef __FB_LOGIN_H__
#define __FB_LOGIN_H__

#include <module/acceptor/acceptor.h>
#include <model/session/session.login.h>
#include <service/auth/auth.service.h>
#include <model/gateway/gateway.h>
#include <zlib/zlib.h>
#include <protocol/login.h>

namespace fb { namespace login {

using namespace fb::protocol;

class acceptor : public fb::acceptor<session>
{
private:
    response::login::agreement  _agreement;
    service::auth               _auth_service;

public:
    acceptor(boost::asio::io_context& context, uint16_t port, uint8_t accept_delay);
    ~acceptor();

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
