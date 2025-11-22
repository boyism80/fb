#include <fb/game/server.h>
#include <algorithm>

using namespace fb::game;

async::task<void> server::fetch_system_mails()
{
    try
    {
        auto&& resp = co_await this->http.post("internal", "/mail/system/get", GetSystemMails{});
        if (resp.error != 0)
            co_return;

        auto now = fb::model::datetime();

        this->_system_mails.write([&resp, now](auto& mails) {
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
    catch (std::exception& e)
    {
        fb::logger::fatal("Failed to fetch system mails: {}", e.what());
    }
    co_return;
}

std::vector<system_mail> server::get_system_mails() const
{
    return this->_system_mails.read([](const std::vector<system_mail>& mails) {
        return mails;
    });
}
