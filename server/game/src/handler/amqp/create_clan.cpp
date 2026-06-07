#include <fb/game/handler/amqp/create_clan.h>
#include <fb/game/server.h>

using namespace fb::game::handler::amqp;

create_clan::create_clan(fb::game::server& server) :
    fb::handler::amqp<fb::game::server, internal_resp::ClanDetails>(server)
{ }

async::task<void> create_clan::handle(const internal_resp::ClanDetails& message)
{
    if (message.host == fb::config<uint32_t>("id"))
        co_return;

    if (static_cast<internal::ClanDetailsAction>(message.action) != internal::ClanDetailsAction::Create)
        co_return;

    auto members = std::unordered_map<std::string, fb::game::clan_member>{};
    for (auto& member : message.members)
    {
        members.insert({
            member.name,
            fb::game::clan_member{member.name, static_cast<fb::game::CLAN_ROLE>(member.role)}
        });
    }

    co_await this->server.clan.on_error(message.error);
    auto title = message.clan.title.has_value() ? message.clan.title.value() : std::string{};
    co_await this->server.clan.on_create(message.clan.id, message.clan.name, title, std::move(members));
}
