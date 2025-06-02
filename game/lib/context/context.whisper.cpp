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

async::task<void> context::whisper(character& from, std::string to, std::string message)
{
    if (from.option(OPTION::WHISPER) == false)
        throw std::runtime_error(_TEXT(MESSAGE_WHISPER_DISABLED_MINE));

    auto target = this->_shard[to]->names.template lock<character*>([&to](auto& names) -> character* {
        if (names.contains(to) == false)
            return nullptr;

        return names.at(to);
    });

    auto from_name = from.name();
    if (target != nullptr)
    {
        co_await this->switch_thread(*target);
        if (target->option(OPTION::WHISPER) == false)
            throw std::runtime_error(std::format(_TEXT(MESSAGE_WHISPER_DISABLED_TARGET), to));

        target->message(std::format("{}> {}", from_name, message), MESSAGE_TYPE::NOTIFY);

        co_await this->switch_thread(from);
        from.message(std::format("{}< {}", target->name(), message), MESSAGE_TYPE::NOTIFY);
    }
    else
    {
        auto&& resp = co_await this->post<internal_reqs::Whisper, internal_resp::Whisper>(
            "internal",
            "/in-game/whisper",
            internal_reqs::Whisper{from_name, to, message});
        co_await this->switch_thread(from);

        this->on_whisper(resp);
        from.message(std::format("{}< {}", to, message), MESSAGE_TYPE::NOTIFY);
    }
}

void context::on_whisper(const internal_resp::Whisper& resp)
{
    this->assert_whisper(resp);
    this->foreach_ch(resp.to, [resp](auto& ch) {
        ch.message(std::format("{}> {}", resp.from, resp.message), MESSAGE_TYPE::NOTIFY);
    });
}