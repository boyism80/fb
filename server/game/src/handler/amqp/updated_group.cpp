#include <fb/game/handler/amqp/updated_group.h>
#include <fb/game/server.h>

using namespace fb::game::handler::amqp;

updated_group::updated_group(fb::game::server& server) :
    fb::handler::amqp<fb::game::server, internal_resp::UpdatedGroup>(server)
{ }

async::task<void> updated_group::handle(const internal_resp::UpdatedGroup& message)
{
    if (message.host == fb::config<uint32_t>("id"))
        co_return;

    auto target = std::string{};
    if (message.target.has_value())
        target = message.target.value().name;

    std::optional<std::string> new_member = std::nullopt;
    if (message.new_member.has_value())
        new_member = message.new_member.value().name;

    std::optional<std::string> deleted_member = std::nullopt;
    if (message.deleted_member.has_value())
        deleted_member = message.deleted_member.value().name;

    co_await this->server.groups.on_error(message.error, target);
    switch (message.action)
    {
    case fb::protocol::internal::GroupActionType::Enter:
        co_await this->server.groups.on_enter(std::move(target), message.group_id, std::move(new_member));
        break;

    case fb::protocol::internal::GroupActionType::Leave:
        co_await this->server.groups.on_leave(std::move(target), message.group_id, std::move(deleted_member));
        break;

    case fb::protocol::internal::GroupActionType::Kick:
        co_await this->server.groups.on_kick(std::move(target), message.group_id, std::move(deleted_member));
        break;

    default:
        break;
    }
}
