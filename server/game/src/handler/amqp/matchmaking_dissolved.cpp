#include <fb/game/handler/amqp/matchmaking_dissolved.h>
#include <fb/config.h>
#include <fb/game/character.h>
#include <fb/game/server.h>

using namespace fb::game::handler::amqp;

matchmaking_dissolved::matchmaking_dissolved(fb::game::server& server) :
    fb::handler::amqp<fb::game::server, fb::protocol::matchmaking::mq::Dissolved>(server)
{ }

async::task<void> matchmaking_dissolved::handle(const fb::protocol::matchmaking::mq::Dissolved& message)
{
    auto world      = fb::config<uint32_t>("world");
    auto match_id   = message.match_id;
    auto match_type = message.match_type;
    auto reason     = message.reason;

    auto  guard      = this->server.characters.enter_write();
    auto& characters = guard.value();
    for (auto& outcome : message.registry_outcomes)
    {
        for (auto& entry : outcome.entries)
        {
            if (entry.world != world)
                continue;

            auto ch = characters.find(entry.character_id);
            if (ch == nullptr)
                continue;

            auto weak    = ch->weak_from_this_as<character>();
            auto builder = this->server.threads.new_builder(weak);
            builder.func =
                [ch, match_id, match_type, reason, outcome_value = outcome.outcome](auto&) -> async::task<void> {
                ch->matchmaker.clear_pending_match_id_if(match_id);
                if (outcome_value == 0)
                    ch->matchmaker.clear_enrollment();

                auto lua = ch->server.lua.new_ctx_guard("scripts/interaction.lua", "on_matchmaking_dissolved");
                if (lua)
                {
                    lua->pushobject(ch);
                    lua->pushstring(match_id.c_str());
                    lua->pushinteger(match_type);
                    lua->pushinteger(reason);
                    lua->pushinteger(outcome_value);
                    std::ignore = lua->call(5);
                }
                co_return;
            };
            builder.enqueue();
        }
    }

    co_return;
}
