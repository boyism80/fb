#include <fb/game/handler/amqp/matchmaking_proposed.h>

#include <fb/config.h>
#include <fb/game/character.h>
#include <fb/game/server.h>
#include <fb/logger.h>
#include <fb/model/datetime.h>

#include <algorithm>
#include <cstdint>
#include <exception>
#include <tuple>

using namespace fb::game::handler::amqp;

matchmaking_proposed::matchmaking_proposed(fb::game::server& server) :
    fb::handler::amqp<fb::game::server, fb::protocol::matchmaking::mq::MatchProposed>(server)
{ }

async::task<void> matchmaking_proposed::handle(const fb::protocol::matchmaking::mq::MatchProposed& message)
{
    auto match_id        = message.match_id;
    auto match_type      = message.match_type;
    auto confirm_seconds = int64_t{0};
    try
    {
        auto remaining  = fb::model::datetime(message.confirm_deadline) - fb::model::datetime::utc_now();
        confirm_seconds = std::max<int64_t>(0, (remaining.total_milliseconds() + 999) / 1000);
    }
    catch (const std::exception& e)
    {
        fb::logger::warn("matchmaking_proposed invalid confirm deadline {}: {}", message.confirm_deadline, e.what());
    }

    for (auto& team : message.teams)
    {
        for (auto& ticket : team.tickets)
        {
            for (auto& member : ticket.members)
            {
                auto ch = this->server.characters.find(member.character_id);
                if (ch == nullptr)
                    continue;

                auto weak      = ch->weak_from_this_as<character>();
                auto builder   = this->server.threads.new_builder(weak);
                auto ticket_id = ticket.ticket_id;
                builder.func   = [weak, match_id, match_type, confirm_seconds, ticket_id](auto&) -> async::task<void> {
                    auto ch = weak.lock();
                    if (ch == nullptr)
                        co_return;

                    if (ch->matchmaker.queued() == false)
                    {
                        co_await ch->matchmaker.decline(match_id, fb::game::matchmaker::initiator::SERVER);
                        co_return;
                    }

                    auto current = ch->matchmaker.ticket_id();
                    if (current.has_value() && current.value() != ticket_id)
                    {
                        fb::logger::warn(
                            "matchmaking_proposed ignored match {} for ticket {} of character {} holding ticket {}",
                            match_id,
                            ticket_id,
                            ch->id,
                            current.value());
                        co_return;
                    }

                    ch->matchmaker.set_pending_match_id(match_id);

                    auto lua = ch->server.lua.open("scripts/interaction.lua", "on_matchmaking_proposed");
                    if (lua)
                    {
                        lua->pushobject(ch);
                        lua->pushinteger(static_cast<lua_Integer>(match_id));
                        lua->pushinteger(match_type);
                        lua->pushinteger(static_cast<lua_Integer>(confirm_seconds));
                        ch.reset();
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
