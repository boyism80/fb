#include <fb/game/handler/protocol/exit.h>
#include <fb/game/server.h>

using namespace fb::game::handler::protocol;

namespace game_reqs = fb::protocol::game::request;

exit::exit(fb::game::server& server) :
    fb::handler::protocol<fb::game::server, game_reqs::exit>(server)
{ }

async::task<bool> exit::handle(fb::socket<character>& session, game_reqs::exit& request)
{
    auto ch = session.data();
    if (ch->inited() == false)
        co_return true;

    co_await this->server.transfer(session,
                                   fb::config<std::string_view>("login:ip"),
                                   fb::config<uint16_t>("login:port"),
                                   internal::Service::Game);
    co_return true;
}
