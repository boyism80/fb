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

void character::container::broadcast(std::string_view message, MESSAGE_TYPE type)
{
    auto message_str = std::string(message);
    this->foreach_enqueue([message_str, type](auto& ch) -> async::task<void> {
        ch->message(message_str, type);
        co_return;
    });
}

async::task<void> character::container::broadcast(std::string_view message,
                                                  MESSAGE_TYPE     type,
                                                  BROADCAST_TYPE   broadcast_type)
{
    auto message_str = std::string(message);
    if (broadcast_type == BROADCAST_TYPE::GLOBAL)
    {
        auto   world = fb::config<uint32_t>("world");
        auto&& resp  = co_await this->_server.http.post(
            "internal",
            "/in-game/broadcast",
            internal_reqs::Broadcast{world, fb::config<uint32_t>("id"), message_str, static_cast<uint8_t>(type)});
        co_await this->on_broadcast(resp);
    }
    else
    {
        auto guard = this->_server.characters.enter_write();
        guard.value().broadcast(message_str, type);
    }

    co_return;
}

async::task<void> character::container::on_broadcast(const internal_resp::Broadcast& resp)
{
    this->broadcast(resp.message, static_cast<MESSAGE_TYPE>(resp.type));
    co_return;
}

void character::container::update_time(uint8_t hours)
{
    this->foreach_enqueue([hours](auto& ch) -> async::task<void> {
        ch->update_time(hours);
        co_return;
    });
}

void character::container::send(const fb::stream& stream, bool encrypt)
{
    this->foreach_enqueue([stream, encrypt](auto& ch) -> async::task<void> {
        std::ignore = ch->send(stream, encrypt);
        co_return;
    });
}

character::container::online_snapshot_t character::container::online_users() const
{
    auto users = online_snapshot_t{};
    users.reserve(this->_from_uid.size());

    for (auto it = this->cbegin(); it != this->cend(); ++it)
    {
        const auto& ch = it->second;
        if (ch == nullptr)
            continue;

        users.emplace(it->first, ch->created_date());
    }

    return users;
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

async::task<void> character::container::on_whisper(const internal_resp::Whisper& resp) const
{
    this->assert_whisper(resp.error, resp.to);
    co_await this->invoke(resp.to, [resp, this](auto& ch) {
        ch->message(std::format("{}> {}", resp.from, resp.message), MESSAGE_TYPE::NOTIFY);

        auto log_data             = Json::Value();
        log_data["sender_name"]   = UTF8(resp.from, PLATFORM::WINDOWS);
        log_data["receiver_id"]   = static_cast<Json::Int64>(ch->id);
        log_data["receiver_name"] = UTF8(resp.to, PLATFORM::WINDOWS);
        log_data["message"]       = UTF8(resp.message, PLATFORM::WINDOWS);
        this->_server.log.write("whisper", log_data);
    });
}

std::string character::container::build_ban_message(std::string_view                  reason,
                                                    const std::optional<std::string>& expire_date)
{
    auto ban_message = std::string(_TEXT(MESSAGE_ACCOUNT_BANNED));
    if (!reason.empty())
        ban_message += std::format(_TEXT(MESSAGE_ACCOUNT_BAN_REASON), reason);

    if (expire_date.has_value())
        ban_message += std::format(_TEXT(MESSAGE_ACCOUNT_BAN_EXPIRE_DATE), expire_date.value());
    else
        ban_message += _TEXT(MESSAGE_ACCOUNT_BAN_PERMANENT);

    return ban_message;
}

async::task<void> character::container::on_ban(const internal_resp::Ban& message)
{
    auto ch = this->find(message.name);
    if (ch == nullptr)
        co_return;

    auto weak = ch->weak_from_this_as<character>();
    co_await this->_server.threads.switching(weak);
    ch = weak.lock();
    if (ch == nullptr)
        co_return;

    ch->message(build_ban_message(message.reason, message.expire_date), MESSAGE_TYPE::POPUP);

    co_await ch->thread()->sleep(1s);

    ch = weak.lock();
    if (ch == nullptr)
        co_return;

    auto socket_ptr = ch->socket_ptr();
    if (socket_ptr == nullptr)
        co_return;

    socket_ptr->close();
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
