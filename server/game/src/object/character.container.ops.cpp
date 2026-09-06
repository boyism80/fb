#include <fb/game/character.h>
#include <fb/game/server.h>
#include <fb/encoding.h>
#include <fb/config.h>
#include <fb/protocol/flatbuffer/protocol.h>
#include <macro.h>
#include <json/json.h>
#include <format>

using namespace fb::game;
using namespace std::chrono_literals;
namespace game_resp     = fb::protocol::game::response;
namespace internal_resp = fb::protocol::internal::response;
namespace internal_reqs = fb::protocol::internal::request;

void character::container::broadcast(std::string_view message, MESSAGE_TYPE type, bool respect_roar)
{
    auto message_str = std::string(message);
    this->foreach_enqueue([message_str, type, respect_roar](auto& ch) -> async::task<void> {
        if (respect_roar && ch->option(OPTION::ROAR) == false)
            co_return;

        ch->message(message_str, type);
        co_return;
    });
}

async::task<void> character::container::broadcast(std::string_view message,
                                                  MESSAGE_TYPE     type,
                                                  BROADCAST_TYPE   broadcast_type,
                                                  bool             respect_roar)
{
    auto message_str = std::string(message);
    if (broadcast_type == BROADCAST_TYPE::GLOBAL)
    {
        auto   world = fb::config<std::optional<uint32_t>>("world");
        auto&& resp  = co_await this->_server.http.post(
            "internal",
            "/in-game/broadcast",
            internal_reqs::Broadcast{world, fb::config<uint32_t>("id"), message_str, static_cast<uint8_t>(type)});
        // Cross-host GLOBAL has no respect_roar flag; ROAR filtering is for WORLD (local) use.
        co_await this->on_broadcast(resp);
    }
    else
    {
        this->broadcast(message_str, type, respect_roar);
    }

    co_return;
}

async::task<void> character::container::on_broadcast(const internal_resp::Broadcast& resp)
{
    this->broadcast(resp.message, static_cast<MESSAGE_TYPE>(resp.type));
    co_return;
}

void character::container::update_time(uint8_t hours, uint8_t minutes)
{
    this->foreach_enqueue([hours, minutes](auto& ch) -> async::task<void> {
        ch->update_time(hours, minutes);
        co_return;
    });
}

void character::container::send(const fb::stream& stream, bool encrypt)
{
    this->foreach_enqueue([stream, encrypt](auto& ch) -> async::task<void> {
        ch->send(stream, encrypt);
        co_return;
    });
}

void character::container::assert_whisper(uint32_t error, std::string_view to)
{
    switch (static_cast<ERROR_CODE>(error))
    {
    case ERROR_CODE::NONE:
        return;

    case ERROR_CODE::OFFLINE:
        throw std::runtime_error(std::format(_TEXT(MESSAGE_USER_NOT_LOGIN), to));

    case ERROR_CODE::DISABLED_WHISPER_TARGET:
        throw std::runtime_error(std::format(_TEXT(MESSAGE_WHISPER_DISABLED_TARGET), to));

    default:
        throw std::runtime_error(std::format(_TEXT(MESSAGE_UNKNOWN_ERROR_WITH_CODE), error));
    }
}

std::string character::container::build_ban_message(std::string_view                  reason,
                                                    const std::optional<std::string>& expire_date)
{
    auto ban_message = std::string(_TEXT(MESSAGE_ACCOUNT_BANNED));
    if (!reason.empty())
    {
        ban_message += '\n';
        ban_message += std::format(_TEXT(MESSAGE_ACCOUNT_BAN_REASON), reason);
    }

    ban_message += '\n';
    if (expire_date.has_value())
        ban_message += std::format(_TEXT(MESSAGE_ACCOUNT_BAN_EXPIRE_DATE), expire_date.value());
    else
        ban_message += _TEXT(MESSAGE_ACCOUNT_BAN_PERMANENT);

    return ban_message;
}

void character::container::on_kick_out(const internal_resp::KickOut& message)
{
    auto ch = this->find(message.name);
    if (ch == nullptr)
        return;

    auto socket_ptr = ch->socket_ptr();
    if (socket_ptr != nullptr)
        socket_ptr->close();
}

void character::container::on_start_maintenance(const internal_resp::StartMaintenance& message)
{
    this->foreach_enqueue([message = message.message](auto& ch) -> async::task<void> {
        if (ch->role() >= ROLE::ADMIN)
            co_return;

        auto socket_ptr = ch->socket_ptr();
        if (socket_ptr == nullptr)
            co_return;

        ch->message(message, MESSAGE_TYPE::STATE);
        socket_ptr->close();
    });
}
