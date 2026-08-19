#include <fb/login/handler/protocol/meta_dat.h>
#include <fb/login/protocol/name_list.h>
#include <fb/logger.h>
#include <tuple>

namespace login_reqs  = fb::protocol::login::request;
namespace login_resps = fb::protocol::login::response;
using namespace fb::login::handler::protocol;

template <fb::protocol::CLIENT_VERSION V>
meta_dat<V>::meta_dat(fb::login::server& server) :
    fb::handler::protocol<fb::login::server, login_reqs::meta_dat<V>>(server)
{ }

template <fb::protocol::CLIENT_VERSION V>
async::task<bool> meta_dat<V>::handle(fb::socket<fb::login::session>& session, login_reqs::meta_dat<V>& request)
{
    if constexpr (V == fb::protocol::CLIENT_VERSION::v651)
    {
        if (request.name.empty() == false)
        {
            fb::logger::info("meta_dat miss name={} (no body asset)", request.name);
            co_return true;
        }

        auto entry = std::vector<std::pair<std::string, uint32_t>>{
            {"Collections", 0x6863AD43u},
            {"GroupNames",  0x31D8776Cu}
        };
        std::ignore = session.send(login_resps::name_list<V>(1, entry.size(), {}, 0, {}, entry));
    }

    co_return true;
}

template class meta_dat<fb::protocol::CLIENT_VERSION::v550>;
template class meta_dat<fb::protocol::CLIENT_VERSION::v565>;
template class meta_dat<fb::protocol::CLIENT_VERSION::v651>;
