#include <fb/login/handler/protocol/keep_alive.h>

#include <fb/logger.h>

#include <cstdint>

namespace login_reqs = fb::protocol::login::request;
using namespace fb::login::handler::protocol;

template <fb::protocol::CLIENT_VERSION V>
keep_alive<V>::keep_alive(fb::login::server& server) :
    fb::handler::protocol<fb::login::server, login_reqs::keep_alive<V>>(server)
{ }

template <fb::protocol::CLIENT_VERSION V>
async::task<bool> keep_alive<V>::handle(fb::socket<fb::login::session>& session, login_reqs::keep_alive<V>& request)
{
    fb::logger::info("login keep-alive 0x71 from {}:{} (client version {})",
                     session.ip(),
                     session.port(),
                     static_cast<uint32_t>(V));
    co_return true;
}

template class keep_alive<fb::protocol::CLIENT_VERSION::v550>;
template class keep_alive<fb::protocol::CLIENT_VERSION::v565>;
template class keep_alive<fb::protocol::CLIENT_VERSION::v651>;
