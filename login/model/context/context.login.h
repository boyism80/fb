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

class context : public fb::acceptor<fb::login::session>
{
public:
    using unique_session = std::unique_ptr<fb::login::session>;

private:
    fb::protocol::login::response::agreement    _agreement = CP949(fb::config::get()["agreement"].asString(), PLATFORM::Both);
    service::auth                               _auth_service;
    std::vector<unique_session>                 _sessions;

public:
    context(boost::asio::io_context& context, uint16_t port, std::chrono::seconds delay, const INTERNAL_CONNECTION& internal_connection);
    ~context();

    // override
public:
    fb::login::session*         handle_accepted(fb::socket<fb::login::session>&);

public:
    bool                        handle_connected(fb::socket<fb::login::session>&);
    bool                        handle_disconnected(fb::socket<fb::login::session>&);

public:
    bool                        handle_in_transfer(fb::internal::socket<>&, const fb::protocol::internal::response::transfer&);
    bool                        handle_in_shutdown(fb::internal::socket<>&, const fb::protocol::internal::response::shutdown&);

public:
    bool                        handle_agreement(fb::socket<fb::login::session>&, const fb::protocol::login::request::agreement&);
    bool                        handle_create_account(fb::socket<fb::login::session>&, const fb::protocol::login::request::account::create&);
    bool                        handle_account_complete(fb::socket<fb::login::session>&, const fb::protocol::login::request::account::complete&);
    bool                        handle_login(fb::socket<fb::login::session>&, const fb::protocol::login::request::login&);
    bool                        handle_change_password(fb::socket<fb::login::session>&, const fb::protocol::login::request::account::change_pw&);
};

} }

#endif // !__FB_LOGIN_H__
