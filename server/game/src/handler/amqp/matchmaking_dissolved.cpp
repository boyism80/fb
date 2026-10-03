#include <fb/game/handler/amqp/matchmaking_dissolved.h>

#include <fb/config.h>
#include <fb/game/character.h>
#include <fb/game/server.h>

#include <tuple>

using namespace fb::game::handler::amqp;

matchmaking_dissolved::matchmaking_dissolved(fb::game::server& server) :
    fb::handler::amqp<fb::game::server, fb::protocol::matchmaking::mq::MatchDissolved>(server)
{ }

async::task<void> matchmaking_dissolved::handle(const fb::protocol::matchmaking::mq::MatchDissolved& message)
{
    auto match_id   = message.match_id;
    auto match_type = message.match_type;
    auto reason     = message.reason;

    for (auto& outcome : message.ticket_outcomes)
    {
        for (auto& member : outcome.members)
        {
            auto ch = this->server.characters.find(member.character_id);
            if (ch == nullptr)
                continue;

            auto weak    = ch->weak_from_this_as<character>();
            auto builder = this->server.threads.new_builder(weak);
            builder.func =
                [ch, match_id, match_type, reason, outcome_value = outcome.outcome](auto&) -> async::task<void> {
                ch->matchmaker.clear_pending_match_id_if(match_id);
                if (outcome_value == 0)
                    ch->matchmaker.clear_ticket();

                auto lua = ch->server.lua.open("scripts/interaction.lua", "on_matchmaking_dissolved");
                if (lua)
                {
                    lua->pushobject(ch);
                    lua->pushinteger(static_cast<lua_Integer>(match_id));
                    lua->pushinteger(match_type);
                    lua->pushinteger(reason);
                    lua->pushinteger(outcome_value);
                    std::ignore = co_await lua->call(5);
                }
                co_return;
            };
            builder.enqueue();
        }
    }

    co_return;
}
