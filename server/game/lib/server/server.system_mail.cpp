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

        // Get current system mail IDs before update
        auto old_mail_ids = this->_system_mails.read([](const std::vector<system_mail>& mails) {
            std::set<uint32_t> ids;
            for (const auto& mail : mails)
                ids.insert(mail.id);
            return ids;
        });

        // Convert DTOs to DAOs and update system mails in memory with thread-safe access
        std::set<uint32_t> new_mail_ids;
        this->_system_mails.write([&resp, &new_mail_ids](auto& mails) {
            mails.clear();
            mails.reserve(resp.mails.size());
            std::transform(resp.mails.begin(), resp.mails.end(), std::back_inserter(mails), [&new_mail_ids](const internal::SystemMail& dto) {
                new_mail_ids.insert(dto.id);
                return system_mail(dto);
            });
        });

        // Find newly added system mail IDs
        std::vector<uint32_t> added_mail_ids;
        for (auto id : new_mail_ids)
        {
            if (old_mail_ids.find(id) == old_mail_ids.end())
                added_mail_ids.push_back(id);
        }

        // Add new system mails to all logged-in characters
        if (!added_mail_ids.empty())
        {
            // Get system mails map for quick lookup
            auto system_mails_map = this->_system_mails.read([](const std::vector<system_mail>& mails) {
                std::map<uint32_t, const system_mail*> map;
                for (const auto& mail : mails)
                    map[mail.id] = &mail;
                return map;
            });

            co_await this->characters.foreach_async([added_mail_ids, system_mails_map](auto& ch) -> async::task<void> {
                for (auto mail_id : added_mail_ids)
                {
                    auto mail_it = system_mails_map.find(mail_id);
                    if (mail_it != system_mails_map.end())
                    {
                        const auto& mail = *mail_it->second;
                        ch->mail_box.add_system_mail_user(mail_id, mail.expire_date.has_value() ? std::make_optional(mail.expire_date.value().to_string()) : std::nullopt);
                    }
                }
                co_return;
            });
        }
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
