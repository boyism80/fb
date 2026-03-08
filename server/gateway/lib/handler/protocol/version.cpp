#include <fb/gateway/handler/protocol/version.h>
#include <fb/gateway/util.h>

using namespace fb::gateway::handler::protocol;
using namespace fb::gateway::util;

namespace gateway_resp = fb::protocol::gateway::response;

version::version(fb::gateway::server& server) :
    fb::handler::protocol<fb::gateway::server, gateway_reqs::version>(server)
{ }

async::task<bool> version::handle(fb::socket<fb::gateway::session>& session, gateway_reqs::version& request)
{
    try
    {
        assert_client(request);

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
