#include <fb/game/context.h>

using namespace fb::game;

void context::assert_whisper(const internal_resp::Whisper& response) const
{
    switch (static_cast<ERROR_CODE>(response.error))
    {
    case ERROR_CODE::NONE:
        return;

    case ERROR_CODE::OFFLINE:
        throw std::runtime_error(std::format(_TEXT(MESSAGE_USER_NOT_LOGIN), response.to));

    case ERROR_CODE::DISABLED_WHISPER_TARGET:
        throw std::runtime_error(std::format("{}님은 귓속말 거부 상태입니다.", response.to));

    default:
        throw std::runtime_error(std::format(_TEXT(MESSAGE_UNKNOWN_ERROR_WITH_CODE), response.error));
    }
}

async::task<void> context::whisper(character& sender, std::string receiver_name, std::string message)
{
    if (sender.option(OPTION::WHISPER) == false)
        throw std::runtime_error(_TEXT(MESSAGE_WHISPER_DISABLED_MINE));

    auto sender_weak = sender.weak_from_this_as<character>();
    auto sender_name = sender.name();

    auto receiver = this->characters.find(receiver_name);
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
    }
    else
    {
        auto&& resp =
            co_await this->http.post("internal", "/in-game/whisper", Whisper{sender_name, receiver_name, message});
        co_await this->threads.switching(sender_weak);

        this->on_whisper(resp);
        sender.message(std::format("{}< {}", receiver_name, message), MESSAGE_TYPE::NOTIFY);
    }
}

void context::on_whisper(const internal_resp::Whisper& resp)
{
    this->assert_whisper(resp);
    this->characters.invoke(resp.to, [resp](auto& ch) {
        ch->message(std::format("{}> {}", resp.from, resp.message), MESSAGE_TYPE::NOTIFY);
    });
}