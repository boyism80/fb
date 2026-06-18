#include <fb/gateway/handler/protocol/version.h>
#include <fb/config.h>
#include <fb/model/model.h>

using namespace fb::gateway::handler::protocol;

namespace gateway_resp = fb::protocol::gateway::response;

version::version(fb::gateway::server& server) :
    fb::handler::protocol<fb::gateway::server, gateway_reqs::version>(server)
{ }

async::task<bool> version::handle(fb::socket<fb::gateway::session>& session, gateway_reqs::version& request)
{
    try
    {
        if (request.v != fb::config<uint16_t>("client:version"))
            throw std::runtime_error(_TEXT(MESSAGE_CLIENT_VERSION_MISMATCH));

        if (request.nation != fb::config<uint8_t>("client:nation"))
            throw std::runtime_error(_TEXT(MESSAGE_CLIENT_NATION_INVALID));

        auto encryption = fb::encryption::generate();
        session.encryption(encryption);

        this->server.send(session, gateway_resp::encryption(encryption, this->server.endpoint_crc()), false);
        co_return true;
    }
    catch (std::exception&)
    {
        co_return false;
    }
}
