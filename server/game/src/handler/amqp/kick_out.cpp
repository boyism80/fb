#include <fb/game/handler/amqp/kick_out.h>

#include <fb/game/server.h>

using namespace fb::game::handler::amqp;

kick_out::kick_out(fb::game::server& server) :
    fb::handler::amqp<fb::game::server, internal_resp::KickOut>(server)
{ }

async::task<void> kick_out::handle(const internal_resp::KickOut& message)
{
    auto pending = this->server.pending_logins.read([&message](const std::unordered_multiset<std::string>& names) {
        return names.contains(message.name);
    });

    if (this->server.characters.find(message.name) != nullptr)
    {
        this->server.characters.on_kick_out(message);
    }
    else if (pending == false)
    {
        try
        {
            auto resp = co_await this->server.http.post(
                "internal",
                "/in-game/logout",
                internal_reqs::Logout{message.world, message.name, fb::config<uint8_t>("id")});
            if (resp.success == false)
                fb::logger::fatal("Character {} logout after kick-out failed: {}",
                                  message.name,
                                  resp.reason.value_or(""));
        }
        catch (std::exception& e)
        {
            fb::logger::fatal("Character {} logout after kick-out failed: {}", message.name, e.what());
        }
    }
    co_return;
}
