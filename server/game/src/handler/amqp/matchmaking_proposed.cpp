#include <fb/game/handler/amqp/matchmaking_proposed.h>

#include <fb/config.h>
#include <fb/game/character.h>
#include <fb/game/server.h>

#include <tuple>

using namespace fb::game::handler::amqp;

matchmaking_proposed::matchmaking_proposed(fb::game::server& server) :
    fb::handler::amqp<fb::game::server, fb::protocol::matchmaking::mq::MatchProposed>(server)
{ }

async::task<void> matchmaking_proposed::handle(const fb::protocol::matchmaking::mq::MatchProposed& message)
{
    auto match_id         = message.match_id;
    auto match_type       = message.match_type;
    auto confirm_deadline = message.confirm_deadline;

    for (auto& team : message.teams)
    {
        for (auto& ticket : team.tickets)
        {
            for (auto& member : ticket.members)
            {
                auto ch = this->server.characters.find(member.character_id);
                if (ch == nullptr)
                    continue;

                auto weak    = ch->weak_from_this_as<character>();
                auto builder = this->server.threads.new_builder(weak);
                builder.func = [ch, match_id, match_type, confirm_deadline](auto&) -> async::task<void> {
                    if (ch->matchmaker.queued() == false)
                    {
                        co_await ch->matchmaker.decline(match_id, fb::game::matchmaker::initiator::SERVER);
                        co_return;
                    }

                    ch->matchmaker.set_pending_match_id(match_id);

                    auto lua = ch->server.lua.open("scripts/interaction.lua", "on_matchmaking_proposed");
                    if (lua)
                    {
                        lua->pushobject(ch);
                        lua->pushinteger(static_cast<lua_Integer>(match_id));
                        lua->pushinteger(match_type);
                        lua->pushstring(confirm_deadline.c_str());
                        std::ignore = co_await lua->call(4);
                    }
                    co_return;
                };
                builder.enqueue();
            }
        }
    }

    co_return;
}
