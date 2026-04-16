#include <fb/game/handler/protocol/update_option.h>
#include <fb/game/server.h>

using namespace fb::game::handler::protocol;

namespace game_reqs = fb::protocol::game::request;

update_option::update_option(fb::game::server& server) :
    fb::handler::protocol<fb::game::server, game_reqs::update_option>(server)
{ }

async::task<bool> update_option::handle(fb::socket<character>& session, game_reqs::update_option& request)
{
    auto ch = session.data();
    if (ch->inited() == false)
        co_return true;

    auto weak   = ch->weak_from_this_as<character>();
    auto option = OPTION(request.option);
    switch (option)
    {
    case OPTION::EXTENSION:
        if (request.ride)
        {
            if (ch->state() == STATE::RIDING)
                ch->unride();
            else
                ch->ride();
        }
        else
        {
            ch->update_option();
        }
        break;

    default:
        auto next = !ch->option(option);

        // If group option is being turned OFF, handle group leave/destroy before API call
        if (option == OPTION::GROUP && !next)
        {
            // Group option turned OFF - leave or destroy group only if currently in a group
            auto group_id = ch->group_id();
            if (group_id.has_value())
            {
                // Currently in a group - use handle_group_action to handle both master (destroy) and member (leave)
                // cases Note: handle_group_action checks OPTION::GROUP, but at this point it's still true
                try
                {
                    co_await this->server.handle_group_action(*ch, ch->name());
                }
                catch (std::exception& e)
                {
                    fb::logger::warn("update_option: handle_group_action failed: {}", e.what());
                }
            }
        }

        auto   world = fb::config<uint32_t>("world");
        auto&& resp  = co_await this->server.http.post(
            "internal",
            "/in-game/option",
            internal_reqs::SetOption{world, ch->id, static_cast<uint8_t>(option), next});
        co_await this->server.threads.switching(weak);
        auto ptr = weak.lock();
        if (ptr == nullptr)
            co_return true;

        if (resp.success == false)
            ptr->message(_TEXT(MESSAGE_OPTION_UPDATE_FAILED));

        ptr->option(option, next);
        break;
    }
    co_return true;
}
