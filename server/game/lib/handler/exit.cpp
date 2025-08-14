#include <fb/game/handler/exit.h>
#include <fb/game/server.h>

using namespace fb::game::handler;

exit::exit(fb::game::server& server) :
    fb::handler<fb::game::server, fb::protocol::game::request::exit>(server)
{ }

async::task<bool> exit::handle(fb::socket<character>& session, fb::protocol::game::request::exit& request)
{
    auto ch = session.data();
    if (ch->inited() == false)
        co_return true;

    std::ignore = this->server.transfer(session,
                                        fb::config<std::string>("login:ip"),
                                        fb::config<uint16_t>("login:port"),
                                        internal::Service::Game);
    co_return true;
}
