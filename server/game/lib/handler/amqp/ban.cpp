#include <fb/game/handler/amqp/ban.h>
#include <fb/game/server.h>
#include <fb/game/character.h>
#include <fb/socket.h>
#include <format>

using namespace fb::game::handler::amqp;
using namespace fb::model::enum_value;

ban::ban(fb::game::server& server) :
    fb::handler::amqp<fb::game::server, internal_resp::Ban>(server)
{ }

std::string ban::build_ban_message(const std::string& reason, const std::optional<std::string>& expire_date)
{
    auto ban_message = std::string("계정이 정지되었습니다.");
    if (!reason.empty())
    {
        ban_message += std::format("\n사유: {}", reason);
    }
    if (expire_date.has_value())
    {
        ban_message += std::format("\n만료일: {}", expire_date.value());
    }
    else
    {
        ban_message += "\n만료일: 영구정지";
    }
    return ban_message;
}

async::task<void> ban::handle(const internal_resp::Ban& message)
{
    auto ch = this->server.characters.read([&message](const auto& container) {
        return container.find(message.name);
    });

    if (ch == nullptr)
        co_return;

    // Switch to character's thread, show ban message, then disconnect
    auto weak = ch->weak_from_this_as<character>();
    co_await this->server.threads.switching(weak);

    ch->message(build_ban_message(message.reason, message.expire_date), MESSAGE_TYPE::POPUP);

    co_await ch->thread()->sleep(1s);
    ch->socket()->close();
}
