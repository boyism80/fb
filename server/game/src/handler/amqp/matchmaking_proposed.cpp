#include <fb/game/handler/amqp/matchmaking_proposed.h>
#include <fb/config.h>
#include <fb/game/character.h>
#include <fb/game/server.h>

using namespace fb::game::handler::amqp;

matchmaking_proposed::matchmaking_proposed(fb::game::server& server) :
    fb::handler::amqp<fb::game::server, fb::protocol::matchmaking::mq::Proposed>(server)
{ }

async::task<void> matchmaking_proposed::handle(const fb::protocol::matchmaking::mq::Proposed& message)
{
    auto world            = fb::config<uint32_t>("world");
    auto match_id         = message.match_id;
    auto match_type       = message.match_type;
    auto confirm_deadline = message.confirm_deadline;

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
                builder.func = [ch, match_id, match_type, confirm_deadline](auto&) -> async::task<void> {
                    if (ch->matchmaker.enrolled())
                    {
                        ch->matchmaker.set_pending_match_id(match_id);

                        auto lua = ch->server.lua.open("scripts/interaction.lua", "on_matchmaking_proposed");
                        if (lua)
                        {
                            lua->pushobject(ch);
                            lua->pushstring(match_id.c_str());
                            lua->pushinteger(match_type);
                            lua->pushstring(confirm_deadline.c_str());
                            std::ignore = lua->call(4);
                        }
                    }
                    else
                    {
                        co_await ch->matchmaker.decline_queue(match_id, true);
                    }
                    co_return;
                };
                builder.enqueue();
            }
        }
    }

    co_return;
}
