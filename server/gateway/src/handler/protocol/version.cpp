#include <fb/gateway/handler/protocol/version.h>
#include <fb/config.h>
#include <fb/model/model.h>

using namespace fb::gateway::handler::protocol;

namespace gateway_resp = fb::protocol::gateway::response;

namespace {

bool is_version_allowed(uint16_t packed)
{
    const auto& node = fb::config_node("client:versions");
    if (node.isArray() == false || node.size() == 0)
        return false;

    for (const auto& item : node)
    {
        if (static_cast<uint16_t>(item.asUInt()) == packed)
            return true;
    }
    return false;
}

} // namespace

version::version(fb::gateway::server& server) :
    fb::handler::protocol<fb::gateway::server, gateway_reqs::version>(server)
{ }

async::task<bool> version::handle(fb::socket<fb::gateway::session>& session, gateway_reqs::version& request)
{
    try
    {
        if (is_version_allowed(request.v) == false)
            throw std::runtime_error(_TEXT(MESSAGE_CLIENT_VERSION_MISMATCH));

        if (request.nation != fb::config<uint8_t>("client:nation"))
            throw std::runtime_error(_TEXT(MESSAGE_CLIENT_NATION_INVALID));

        auto* data = session.data();
        if (data != nullptr)
            data->client_version = request.client_version;

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
