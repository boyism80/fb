#include <fb/gateway/handler/protocol/check_version.h>

using namespace fb::gateway::handler::protocol;

check_version::check_version(fb::gateway::server& server) :
    fb::handler<fb::gateway::server, fb::protocol::gateway::request::version>(server)
{ }

async::task<bool> check_version::handle(fb::socket<fb::gateway::session>&        session,
                                        fb::protocol::gateway::request::version& request)
{
    try
    {
        util::assert_client(request);

        auto encryption = encryption::generate();
        session.encryption(encryption);

        this->server.send(session, response::encryption(encryption, this->server.endpoint_crc()), false);
        co_return true;
    }
    catch (std::exception&)
    {
        co_return false;
    }
}
