#include <fb/game/server.h>
#include <fb/encoding.h>
#include <json/json.h>

using namespace fb::game;

void server::assert_whisper(const internal_resp::Whisper& response) const
{
    switch (static_cast<ERROR_CODE>(response.error))
    {
    case ERROR_CODE::NONE:
        return;

    case ERROR_CODE::OFFLINE:
        throw std::runtime_error(std::format(_TEXT(MESSAGE_USER_NOT_LOGIN), response.to));

    case ERROR_CODE::DISABLED_WHISPER_TARGET:
        throw std::runtime_error(std::format(_TEXT(MESSAGE_WHISPER_DISABLED_TARGET), response.to));

    default:
        throw std::runtime_error(std::format(_TEXT(MESSAGE_UNKNOWN_ERROR_WITH_CODE), response.error));
    }
}

async::task<void> server::whisper(character& sender, std::string receiver_name, std::string message)
{
    if (sender.option(OPTION::WHISPER) == false)
        throw std::runtime_error(_TEXT(MESSAGE_WHISPER_DISABLED_MINE));

    co_await this->characters.async_write([this, &sender, &receiver_name, &message](auto& characters) -> async::task<void> {
        auto sender_weak = sender.weak_from_this_as<character>();
        auto sender_name = sender.name();
        auto receiver    = characters.find(receiver_name);
        if (receiver != nullptr)
        {
            auto target_weak = receiver->weak_from_this_as<character>();
            co_await this->threads.switching(target_weak);
            auto target_name = receiver->name();

            if (receiver->option(OPTION::WHISPER) == false)
                throw std::runtime_error(std::format(_TEXT(MESSAGE_WHISPER_DISABLED_TARGET), receiver_name));

            receiver->message(std::format("{}> {}", sender_name, message), MESSAGE_TYPE::NOTIFY);

            co_await this->threads.switching(sender_weak);
            sender.message(std::format("{}< {}", target_name, message), MESSAGE_TYPE::NOTIFY);

            // Log whisper event
            auto log_data             = Json::Value();
            log_data["sender_id"]     = static_cast<Json::Int64>(sender.id());
            log_data["sender_name"]   = UTF8(sender_name, PLATFORM::WINDOWS);
            log_data["receiver_id"]   = static_cast<Json::Int64>(receiver->id());
            log_data["receiver_name"] = UTF8(target_name, PLATFORM::WINDOWS);
            log_data["message"]       = UTF8(message, PLATFORM::WINDOWS);
            this->log.write("whisper", log_data);
        }
        else
        {
            auto&& resp = co_await this->http.post("internal", "/in-game/whisper", Whisper{sender_name, receiver_name, message});
            co_await this->threads.switching(sender_weak);

            co_await this->on_whisper(resp);
            sender.message(std::format("{}< {}", receiver_name, message), MESSAGE_TYPE::NOTIFY);

            // Log whisper event (cross-server)
            auto log_data             = Json::Value();
            log_data["sender_id"]     = static_cast<Json::Int64>(sender.id());
            log_data["sender_name"]   = UTF8(sender_name, PLATFORM::WINDOWS);
            log_data["receiver_name"] = UTF8(receiver_name, PLATFORM::WINDOWS);
            log_data["message"]       = UTF8(message, PLATFORM::WINDOWS);
            this->log.write("whisper", log_data);
        }
    });
}

async::task<void> server::on_whisper(const internal_resp::Whisper& resp)
{
    this->assert_whisper(resp);
    co_await this->characters.async_write([this, &resp](auto& characters) -> async::task<void> {
        co_await characters.invoke(resp.to, [resp, this](auto& ch) {
            ch->message(std::format("{}> {}", resp.from, resp.message), MESSAGE_TYPE::NOTIFY);

            // Log whisper event (received)
            auto log_data             = Json::Value();
            log_data["sender_name"]   = UTF8(resp.from, PLATFORM::WINDOWS);
            log_data["receiver_id"]   = static_cast<Json::Int64>(ch->id());
            log_data["receiver_name"] = UTF8(resp.to, PLATFORM::WINDOWS);
            log_data["message"]       = UTF8(resp.message, PLATFORM::WINDOWS);
            this->log.write("whisper", log_data);
        });
    });
}