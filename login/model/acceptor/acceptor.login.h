#ifndef __FB_LOGIN_H__
#define __FB_LOGIN_H__

#include "service/auth/auth.service.h"
#include "model/session/session.login.h"
#include "model/gateway/gateway.h"
#include <zlib.h>
#include "protocol/login.h"
#include "module/console/console.h"
#include "module/acceptor/acceptor.h"

namespace fb { namespace login {

using namespace fb::protocol;

class acceptor : public fb::acceptor<fb::socket, fb::login::session>
{
private:
    response::login::agreement  _agreement;
    service::auth               _auth_service;

public:
    acceptor(boost::asio::io_context& context, uint16_t port, uint8_t accept_delay);
    ~acceptor();

    // override
public:
    fb::login::session*         handle_accepted(fb::socket<fb::login::session>& socket);

public:
    bool                        handle_connected(fb::socket<fb::login::session>& session);
    bool                        handle_disconnected(fb::socket<fb::login::session>& session);

public:
    bool                        handle_agreement(fb::socket<fb::login::session>& socket, const fb::protocol::request::login::agreement&);
    bool                        handle_create_account(fb::socket<fb::login::session>& socket, const fb::protocol::request::login::account::create&);
    bool                        handle_account_complete(fb::socket<fb::login::session>& socket, const fb::protocol::request::login::account::complete&);
    bool                        handle_login(fb::socket<fb::login::session>& socket, const fb::protocol::request::login::login&);
    bool                        handle_change_password(fb::socket<fb::login::session>& socket, const fb::protocol::request::login::account::change_pw&);
};

} }

#endif // !__FB_LOGIN_H__
