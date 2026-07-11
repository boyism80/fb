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
    auto world      = fb::config<uint32_t>("world");
    auto match_id   = message.match_id;
    auto match_type = message.match_type;

    auto  guard      = this->server.characters.enter_write();
    auto& characters = guard.value();
    for (auto& team : message.teams)
    {
        for (auto& registry : team.registries)
        {
            for (auto& entry : registry.entries)
            {
                if (entry.world != world)
                    continue;

                auto ch = characters.find(entry.character_id);
                if (ch == nullptr)
                    continue;

                auto weak    = ch->weak_from_this_as<character>();
                auto builder = this->server.threads.new_builder(weak);
                builder.func = [ch, match_id, match_type](auto&) -> async::task<void> {
                    ch->matchmaker.clear_pending_match_id_if(match_id);
                    ch->matchmaker.clear_enrollment();

                    auto lua = ch->server.lua.open("scripts/interaction.lua", "on_matchmaking_ready");
                    if (lua)
                    {
                        lua->pushobject(ch);
                        lua->pushstring(match_id.c_str());
                        lua->pushinteger(match_type);
                        std::ignore = lua->call(3);
                    }
                    co_return;
                };
                builder.enqueue();
            }
        }
    }

    co_return;
}
