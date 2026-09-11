#include <fb/game/handler/amqp/matchmaking_ready.h>
#include <fb/config.h>
#include <fb/game/character.h>
#include <fb/game/server.h>

using namespace fb::game::handler::amqp;

matchmaking_ready::matchmaking_ready(fb::game::server& server) :
    fb::handler::amqp<fb::game::server, fb::protocol::matchmaking::mq::Ready>(server)
{ }

async::task<void> matchmaking_ready::handle(const fb::protocol::matchmaking::mq::Ready& message)
{
    auto match_id   = message.match_id;
    auto match_type = message.match_type;

    for (uint32_t team_index = 0; team_index < static_cast<uint32_t>(message.teams.size()); ++team_index)
    {
        auto team_id = team_index + 1;
        for (auto& registry : message.teams[team_index].registries)
        {
            for (auto& entry : registry.entries)
            {
                auto ch = this->server.characters.find(entry.character_id);
                if (ch == nullptr)
                    continue;

                auto weak    = ch->weak_from_this_as<character>();
                auto builder = this->server.threads.new_builder(weak);
                builder.func = [ch, match_id, match_type, team_id](auto&) -> async::task<void> {
                    ch->matchmaker.clear_pending_match_id_if(match_id);
                    ch->matchmaker.clear_enrollment();

                    auto lua = ch->server.lua.open("scripts/interaction.lua", "on_matchmaking_ready");
                    if (lua)
                    {
                        lua->pushobject(ch);
                        lua->pushstring(match_id.c_str());
                        lua->pushinteger(match_type);
                        lua->pushinteger(team_id);
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
