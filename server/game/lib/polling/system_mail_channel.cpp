#include <fb/game/polling/system_mail_channel.h>
#include <fb/game/server.h>
#include <fb/game/system_mail.h>
#include <fb/logger.h>

using namespace fb::game;

system_mail_channel::system_mail_channel(server& owner) :
    _owner(owner)
{ }

async::task<void> system_mail_channel::fetch()
{
    try
    {
        auto&& resp = co_await this->_owner.http.get<fb::protocol::internal::response::GetSystemMails>("internal", "/mail/system");
        if (resp.error != 0)
            co_return;

        auto now = fb::model::datetime();

        this->_data.write([&resp, now](auto& mails) {
            mails.clear();
            mails.reserve(resp.mails.size());
            for (const auto& dto : resp.mails)
            {
                auto mail = system_mail(dto);
                if (mail.expire_date.has_value() && mail.expire_date.value() < now)
                    continue;

                mails.push_back(std::move(mail));
            }
        });
    }
    catch (const std::exception& e)
    {
        fb::logger::fatal("Failed to fetch system mails: {}", e.what());
    }
    co_return;
}

void system_mail_channel::read(std::function<void(const std::vector<system_mail>&)> fn)
{
    this->_data.read(fn);
}

async::task<void> system_mail_channel::read_async(std::function<async::task<void>(const std::vector<system_mail>&)> fn)
{
    co_await this->_data.async_read(fn);
}

