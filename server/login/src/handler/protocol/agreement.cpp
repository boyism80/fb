#include <fb/login/handler/protocol/agreement.h>

using namespace fb::login::handler::protocol;

agreement::agreement(fb::login::server& server) :
    fb::handler::protocol<fb::login::server, fb::protocol::login::request::agreement>(server)
{ }

async::task<bool> agreement::handle(fb::socket<fb::login::session>&          session,
                                    fb::protocol::login::request::agreement& request)
{
    try
    {
        if (encryption::validate(request.enc_type, request.enc_key, request.enc_key_size) == false)
            throw std::exception();

        session.encryption(request.enc_type, request.enc_key);
        session.send(this->server.agreement());
        co_return true;
    }
    catch (std::exception&)
    {
        co_return false;
    }
}
