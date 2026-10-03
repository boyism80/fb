#include <fb/game/handler/amqp/matchmaking_ticket_removed.h>

#include <fb/config.h>
#include <fb/game/character.h>
#include <fb/game/server.h>

#include <tuple>

using namespace fb::game::handler::amqp;

matchmaking_ticket_removed::matchmaking_ticket_removed(fb::game::server& server) :
    fb::handler::amqp<fb::game::server, fb::protocol::matchmaking::mq::TicketRemoved>(server)
{ }

async::task<void> matchmaking_ticket_removed::handle(const fb::protocol::matchmaking::mq::TicketRemoved& message)
{
    auto match_type = message.match_type;
    auto ticket_id  = message.ticket_id;

    for (auto& member : message.members)
    {
        auto ch = this->server.characters.find(member.character_id);
        if (ch == nullptr)
            continue;

        auto weak    = ch->weak_from_this_as<character>();
        auto builder = this->server.threads.new_builder(weak);
        builder.func = [ch, match_type, ticket_id](auto&) -> async::task<void> {
            // The member already dropped or replaced this ticket on its own.
            if (ch->matchmaker.ticket_id() != ticket_id)
                co_return;

            ch->matchmaker.clear_pending_match_id();
            ch->matchmaker.clear_ticket();

            auto lua = ch->server.lua.open("scripts/interaction.lua", "on_matchmaking_dequeue");
            if (lua)
            {
                lua->pushobject(ch);
                lua->pushinteger(match_type);
                lua->pushinteger(static_cast<lua_Integer>(ticket_id));
                std::ignore = co_await lua->call(3);
            }
            co_return;
        };
        builder.enqueue();
    }

    co_return;
}
