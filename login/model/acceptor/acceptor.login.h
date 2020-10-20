#ifndef __FB_LOGIN_H__
#define __FB_LOGIN_H__

#include "service/auth/auth.service.h"
#include "model/session/session.login.h"
#include "model/gateway/gateway.h"
#include <zlib.h>
#include "protocol/login.h"
#include "protocol/internal.h"
#include "module/console/console.h"
#include "module/acceptor/acceptor.h"

namespace fb { namespace login {

class acceptor : public fb::acceptor<fb::login::session>
{
private:
    fb::protocol::login::response::agreement  _agreement;
    service::auth               _auth_service;

public:
    acceptor(boost::asio::io_context& context, uint16_t port, uint8_t accept_delay, const INTERNAL_CONNECTION& internal_connection);
    ~acceptor();

    // override
public:
    fb::login::session*         handle_accepted(fb::socket<fb::login::session>&);

public:
    bool                        handle_connected(fb::socket<fb::login::session>&);
    bool                        handle_disconnected(fb::socket<fb::login::session>&);

public:
    bool                        handle_private_login(fb::internal::socket<>&, const fb::protocol::internal::response::login&);

public:
    bool                        handle_agreement(fb::socket<fb::login::session>&, const fb::protocol::login::request::agreement&);
    bool                        handle_create_account(fb::socket<fb::login::session>&, const fb::protocol::login::request::account::create&);
    bool                        handle_account_complete(fb::socket<fb::login::session>&, const fb::protocol::login::request::account::complete&);
    bool                        handle_login(fb::socket<fb::login::session>&, const fb::protocol::login::request::login&);
    bool                        handle_change_password(fb::socket<fb::login::session>&, const fb::protocol::login::request::account::change_pw&);
};

} }

#endif // !__FB_LOGIN_H__
