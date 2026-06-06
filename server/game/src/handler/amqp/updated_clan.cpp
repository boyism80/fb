#include <fb/game/handler/amqp/updated_clan.h>
#include <fb/game/server.h>

using namespace fb::game::handler::amqp;

updated_clan::updated_clan(fb::game::server& server) :
    fb::handler::amqp<fb::game::server, internal_resp::UpdatedClan>(server)
{ }

async::task<void> updated_clan::handle(const internal_resp::UpdatedClan& message)
{
    if (message.host == fb::config<uint32_t>("id"))
        co_return;

    co_await this->server.clans.on_error(message.error);

    switch (message.action)
    {
    case internal::ClanActionType::Join:
    {
        auto role = message.new_role.has_value() ? static_cast<fb::game::CLAN_ROLE>(message.new_role.value())
                                                 : fb::game::CLAN_ROLE::MATE;
        co_await this->server.clans.on_join(
            message.clan_id,
            message.new_member.has_value() ? std::optional(message.new_member.value().name) : std::nullopt,
            role);
        break;
    }

    case internal::ClanActionType::Leave:
        co_await this->server.clans.on_leave(
            message.clan_id,
            message.deleted_member.has_value() ? std::optional(message.deleted_member.value().name) : std::nullopt);
        break;

    case internal::ClanActionType::Kick:
        co_await this->server.clans.on_kick(
            message.clan_id,
            message.deleted_member.has_value() ? std::optional(message.deleted_member.value().name) : std::nullopt);
        break;

    case internal::ClanActionType::ChangeRole:
        co_await this->server.clans.on_change_role(
            message.clan_id,
            message.target.has_value() ? std::optional(message.target.value().uid) : std::nullopt,
            message.target.has_value() ? std::optional(message.target.value().name) : std::nullopt,
            message.old_role,
            message.new_role);
        break;

    case internal::ClanActionType::SetTitle:
        co_await this->server.clans.on_set_title(message.clan_id, message.new_title);
        break;

    default:
        break;
    }
}
