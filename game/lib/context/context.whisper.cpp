#include <fb/game/context.h>

using namespace fb::game;

void context::assert_whisper(const internal_resp::Whisper& response) const
{
    switch (static_cast<ERROR_CODE>(response.error))
    {
    case ERROR_CODE::NONE:
        return;

    case ERROR_CODE::OFFLINE:
        throw std::runtime_error(std::format("{}님은 바람의나라에 없습니다.", response.to));

    case ERROR_CODE::DISABLED_WHISPER_TARGET:
        throw std::runtime_error(std::format("{}님은 귓속말 거부 상태입니다.", response.to));

    default:
        throw std::runtime_error(std::format(_TEXT(MESSAGE_UNKNOWN_ERROR), response.error));
    }
}

async::task<void> context::whisper(character& from, std::string to, std::string message)
{
    if (from.option(OPTION::WHISPER) == false)
        throw std::runtime_error("당신은 귓속말 거부 상태입니다.");

    auto target = this->_shard[to]->names.template lock<character*>([&to](auto& names) -> character* {
        if (names.contains(to) == false)
            return nullptr;

        return names.at(to);
    });

    auto fd = from.fd();
    if (target != nullptr)
    {
        auto thread = from.thread();

        co_await target->thread()->switching();
        if (target->option(OPTION::WHISPER) == false)
            throw std::runtime_error(std::format("{}님은 귓속말 거부 상태입니다.", to));

        target->message(std::format("{}> {}", from.name(), message), MESSAGE_TYPE::NOTIFY);

        co_await thread->switching();
        if (this->assert_socket(fd) == false)
            co_return;

        from.message(std::format("{}< {}", target->name(), message), MESSAGE_TYPE::NOTIFY);
    }
    else
    {
        auto&& resp = co_await this->post<internal_reqs::Whisper, internal_resp::Whisper>(
            "internal",
            "/in-game/whisper",
            internal_reqs::Whisper{from.name(), to, message});

        if (this->assert_socket(fd) == false)
            co_return;

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