#include <fb/game/handler/amqp/whisper.h>
#include <fb/game/character.h>
#include <fb/game/server.h>
#include <fb/encoding.h>
#include <json/json.h>
#include <format>

using namespace fb::game::handler::amqp;

whisper::whisper(fb::game::server& server) :
    fb::handler::amqp<fb::game::server, internal_resp::Whisper>(server)
{ }

async::task<void> whisper::handle(const internal_resp::Whisper& message)
{
    if (message.host == fb::config<uint16_t>("id"))
        co_return;

    fb::game::character::container::assert_whisper(message.error, message.to);

    auto ch = this->server.characters.find(message.to);
    if (ch == nullptr)
        co_return;

    auto weak   = ch->weak_from_this_as<fb::game::character>();
    auto before = this->server.threads.current();
    co_await this->server.threads.switching(weak);

    ch = weak.lock();
    if (ch != nullptr)
    {
        co_await ch->message(std::format("{}> {}", message.from, message.message), fb::game::MESSAGE_TYPE::NOTIFY);

        auto log_data             = Json::Value();
        log_data["sender_name"]   = UTF8(message.from, PLATFORM::WINDOWS);
        log_data["receiver_id"]   = static_cast<Json::Int64>(ch->id);
        log_data["receiver_name"] = UTF8(message.to, PLATFORM::WINDOWS);
        log_data["message"]       = UTF8(message.message, PLATFORM::WINDOWS);
        this->server.log.write("whisper", log_data);
    }

    if (before != nullptr)
        co_await before->switching();
}
