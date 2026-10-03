#include <fb/login/handler/protocol/keep_alive.h>

#include <fb/logger.h>
#include <fb/model/model.h>

#include <cstdint>

namespace internal_reqs = fb::protocol::internal::request;
using namespace fb::login::handler::protocol;

template <fb::protocol::CLIENT_VERSION V>
keep_alive<V>::keep_alive(fb::login::server& server) :
    fb::handler::protocol<fb::login::server, login_reqs::keep_alive<V>>(server)
{ }

template <fb::protocol::CLIENT_VERSION V>
async::task<bool> keep_alive<V>::handle(fb::socket<fb::login::session>& session, login_reqs::keep_alive<V>& request)
{
    auto session_data = session.data();
    if (session_data != nullptr && !session_data->pending_name.empty())
    {
        auto weak         = session.weak_from_this_as<fb::socket<fb::login::session>>();
        auto world        = fb::config<uint32_t>("world");
        auto pending_name = session_data->pending_name;
        std::ignore       = co_await this->server.http.post("internal",
                                                      "/account/name-keepalive",
                                                      internal_reqs::ReserveName{world, pending_name});
        co_await this->server.threads.switching(weak);
    }
    co_return true;
}

template class keep_alive<fb::protocol::CLIENT_VERSION::v550>;
template class keep_alive<fb::protocol::CLIENT_VERSION::v565>;
template class keep_alive<fb::protocol::CLIENT_VERSION::v651>;
