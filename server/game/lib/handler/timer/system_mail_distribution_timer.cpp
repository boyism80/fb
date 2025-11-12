#include <fb/game/handler/timer/system_mail_distribution_timer.h>
#include <fb/game/server.h>
#include <algorithm>

namespace fb::game::handler::timer {

system_mail_distribution_timer::system_mail_distribution_timer(fb::game::server& server) :
    fb::handler::timer<fb::game::server>(server)
{ }

async::task<void> system_mail_distribution_timer::handle(const fb::model::datetime& now, std::thread::id id)
{
    auto thread = this->server.threads.at(id);
    auto params = thread->template data<thread_params>();

    // Get all active system mails from memory (thread-safe read)
    auto system_mails = this->server.get_system_mails();
    if (system_mails.empty())
        co_return;

    // Create a map of system mail ID to system mail for quick lookup
    auto system_mail_map = std::unordered_map<uint32_t, const system_mail*>();
    for (const auto& mail : system_mails)
    {
        // Check expiration
        if (mail.expire_date.has_value() && mail.expire_date.value() < now)
            continue;

        system_mail_map[mail.id] = &mail;
    }

    if (system_mail_map.empty())
        co_return;

    // Collect all characters in this thread
    auto characters_to_process = std::vector<std::shared_ptr<fb::game::character>>();
    for (auto& [_, map] : params->maps)
    {
        if (map->active == false)
            continue;

        for (auto& [fd, obj] : map->objects)
        {
            if (obj->is(OBJECT_TYPE::CHARACTER) == false)
                continue;

            auto ch = std::static_pointer_cast<fb::game::character>(obj);
            if (ch->inited() == false)
                continue;

            characters_to_process.push_back(ch);
        }
    }

    if (characters_to_process.empty())
        co_return;

    // Limit the number of users to process per execution
    auto process_count = std::min(characters_to_process.size(), static_cast<size_t>(PROCESS_LIMIT));

    // Process each character
    for (size_t i = 0; i < process_count; i++)
    {
        auto ch = characters_to_process[i];
        if (ch == nullptr)
            continue;

        try
        {
            // Get unreceived system mail IDs from character's in-memory collection (no HTTP request)
            auto        unreceived_mail_ids = std::vector<uint32_t>();
            const auto& system_mail_users   = ch->mail_box.get_system_mail_users();
            for (const auto& [mail_id, smu] : system_mail_users)
            {
                // Check if mail is not yet sent (read flag tracks sent status)
                if (!smu.read && system_mail_map.find(mail_id) != system_mail_map.end())
                    unreceived_mail_ids.push_back(mail_id);
            }

            if (unreceived_mail_ids.empty())
                continue;

            // Send each unreceived system mail using existing send_mail API
            for (auto mail_id : unreceived_mail_ids)
            {
                try
                {
                    auto mail_it = system_mail_map.find(mail_id);
                    if (mail_it == system_mail_map.end())
                        continue;

                    const auto& mail = *mail_it->second;

                    // Send system mail directly with sender "System"
                    auto&& resp =
                        co_await this->server.http.post("internal", "/mail/write", WriteMail{"System", ch->name(), mail.title, mail.contents, fb::config<uint32_t>("id")});

                    if (resp.error == 0)
                    {
                        // Mark as read (sent) after successful send
                        ch->mail_box.update_system_mail_user_read(mail_id, true);
                    }
                }
                catch (...)
                {
                    // Silently handle errors for individual mail sends
                }
            }
        }
        catch (...)
        {
            // Silently handle errors for individual users
        }
    }

    co_return;
}

} // namespace fb::game::handler::timer
