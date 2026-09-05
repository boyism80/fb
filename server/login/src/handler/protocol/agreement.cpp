#include <fb/login/handler/protocol/agreement.h>

using namespace fb::login::handler::protocol;

template <fb::protocol::CLIENT_VERSION V>
agreement<V>::agreement(fb::login::server& server) :
    fb::handler::protocol<fb::login::server, login_reqs::agreement<V>, 1000, 10, true>(server)
{ }

template <fb::protocol::CLIENT_VERSION V>
async::task<bool> agreement<V>::handle(fb::socket<fb::login::session>& session, login_reqs::agreement<V>& request)
{
    try
    {
        if (encryption::validate(request.enc_type, request.enc_key, request.enc_key_size) == false)
            throw std::exception();

        session.encryption(request.enc_type, request.enc_key);

        // Establish const session version after bootstrap deserialize (always v550 layout).
        session.data(std::make_shared<fb::login::session>(request.client_version, request.ui_mode));

        session.send(this->server.agreement());
        co_return true;
    }
    catch (std::exception&)
    {
        co_return false;
    }
}

template class agreement<fb::protocol::CLIENT_VERSION::v550>;
template class agreement<fb::protocol::CLIENT_VERSION::v565>;
template class agreement<fb::protocol::CLIENT_VERSION::v651>;
