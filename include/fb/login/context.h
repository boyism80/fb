#ifndef __FB_LOGIN_H__
#define __FB_LOGIN_H__

#include <fb/login/auth.h>
#include <fb/login/session.h>
#include <fb/login/gateway.h>
#include <zlib.h>
#include <fb/protocol/login.h>
#include <fb/protocol/internal.h>
#include <fb/core/acceptor.h>

namespace fb { namespace login {

class context : public fb::acceptor<fb::login::session>
{
public:
    using unique_session = std::unique_ptr<fb::login::session>;

private:
    fb::protocol::login::response::agreement    _agreement = CP949(fb::config::get()["agreement"].asString(), PLATFORM::Both);
    service::auth                               _auth_service;
    std::vector<unique_session>                 _sessions;
    fb::db::context<session>                    _db;

public:
    context(boost::asio::io_context& context, uint16_t port);
    ~context();

    // override
protected:
    bool                        is_decrypt(uint8_t) const final;
    fb::login::session*         handle_accepted(fb::socket<fb::login::session>&) final;
    bool                        handle_connected(fb::socket<fb::login::session>&) final;
    bool                        handle_disconnected(fb::socket<fb::login::session>&) final;
    void                        handle_internal_connected() final;

public:
    fb::task<bool>              handle_in_shutdown(fb::internal::socket<>&, const fb::protocol::internal::response::shutdown&);

public:
    fb::task<bool>              handle_agreement(fb::socket<fb::login::session>&, const fb::protocol::login::request::agreement&);
    fb::task<bool>              handle_create_account(fb::socket<fb::login::session>&, const fb::protocol::login::request::account::create&);
    fb::task<bool>              handle_account_complete(fb::socket<fb::login::session>&, const fb::protocol::login::request::account::complete&);
    fb::task<bool>              handle_login(fb::socket<fb::login::session>&, const fb::protocol::login::request::login&);
    fb::task<bool>              handle_change_password(fb::socket<fb::login::session>&, const fb::protocol::login::request::account::change_pw&);
};

} }

#endif // !__FB_LOGIN_H__
