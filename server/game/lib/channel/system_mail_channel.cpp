#include <fb/game/channel/system_mail_channel.h>
#include <fb/game/server.h>
#include <fb/game/system_mail.h>
#include <fb/logger.h>
#include <algorithm>

using namespace fb::game;

system_mail_channel::system_mail_channel(server& owner) :
    _owner(owner)
{ }

async::task<void> system_mail_channel::fetch()
{
    namespace internal_resp = fb::protocol::internal::response;
    try
    {
        auto   section = fb::config<std::string>("section");
        auto&& resp    = co_await this->_owner.http.get<internal_resp::GetSystemMails>(
            "internal", std::format("/mail/system/{}", section));
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

        // Notify all connected characters about system mail updates
        this->_owner.characters.write([](auto& characters) {
            characters.foreach_enqueue([](auto& ch) -> async::task<void> {
                co_await ch->process_system_mails();
            });
        });
    }
    catch (const std::exception& e)
    {
        fb::logger::fatal("Failed to fetch system mails: {}", e.what());
    }
    co_return;
}

async::task<void> system_mail_channel::append(const system_mail& mail)
{
    try
    {
        auto now = fb::model::datetime();

        // Skip if expired
        if (mail.expire_date.has_value() && mail.expire_date.value() < now)
            co_return;

        this->_data.write([&mail](auto& mails) {
            // Check for duplicate by id
            auto it = std::find_if(mails.begin(), mails.end(), [&mail](const auto& existing_mail) {
                return existing_mail.id == mail.id;
            });

            if (it == mails.end())
            {
                mails.push_back(mail);
            }
        });

        // Notify all connected characters about new system mail
        this->_owner.characters.write([](auto& characters) {
            characters.foreach_enqueue([](auto& ch) -> async::task<void> {
                co_await ch->process_system_mails();
            });
        });
    }
    catch (const std::exception& e)
    {
        fb::logger::fatal("Failed to append system mail: {}", e.what());
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




