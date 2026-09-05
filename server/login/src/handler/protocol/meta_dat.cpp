#include <fb/login/handler/protocol/meta_dat.h>
#include <fb/login/protocol/name_list.h>
#include <tuple>

namespace login_reqs  = fb::protocol::login::request;
namespace login_resps = fb::protocol::login::response;
using namespace fb::login::handler::protocol;

template <fb::protocol::CLIENT_VERSION V>
meta_dat<V>::meta_dat(fb::login::server& server) :
    fb::handler::protocol<fb::login::server, login_reqs::meta_dat<V>, 1000, 10, true>(server)
{ }

template <fb::protocol::CLIENT_VERSION V>
async::task<bool> meta_dat<V>::handle(fb::socket<fb::login::session>& session, login_reqs::meta_dat<V>& request)
{
    if constexpr (V == fb::protocol::CLIENT_VERSION::v651)
    {
        if (request.name.empty() == false)
        {
            auto entry = this->server.meta.find(request.name);
            if (entry == nullptr)
                co_return true;

            std::ignore = session.send(login_resps::name_list<V>(0, 0, entry->name, entry->crc, entry->compressed, {}));
            co_return true;
        }

        auto list = this->server.meta.crc_list();
        std::ignore =
            session.send(login_resps::name_list<V>(1, static_cast<uint16_t>(list.size()), {}, 0, {}, std::move(list)));
    }

    co_return true;
}

template class meta_dat<fb::protocol::CLIENT_VERSION::v550>;
template class meta_dat<fb::protocol::CLIENT_VERSION::v565>;
template class meta_dat<fb::protocol::CLIENT_VERSION::v651>;
