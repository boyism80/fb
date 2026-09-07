#include <fb/game/handler/protocol/update_option.h>
#include <fb/game/server.h>
#include <fb/amqp_route.h>

namespace game_reqs = fb::protocol::game::request;

namespace fb::game::handler::protocol {

template <fb::protocol::CLIENT_VERSION V>
update_option<V>::update_option(fb::game::server& server) :
    fb::handler::protocol<fb::game::server, game_reqs::update_option<V>>(server)
{ }

template <fb::protocol::CLIENT_VERSION V>
async::task<bool> update_option<V>::handle(fb::socket<character>& session, game_reqs::update_option<V>& request)
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
                co_await ch->unride();
            else
                co_await ch->ride();
        }
        else
        {
            ch->update_option();
        }
        break;

    default:
        auto next = !ch->option(option);

        if (option == OPTION::GROUP && !next)
        {
            auto group_id = ch->group_id();
            if (group_id.has_value())
            {
                try
                {
                    co_await this->server.groups.handle_action(*ch, ch->name());
                }
                catch (std::exception& e)
                {
                    fb::logger::warn("update_option: groups.handle_action failed: {}", e.what());
                }
            }
        }

        auto   world = ch->world();
        auto&& resp  = co_await this->server.http.post(
            "internal",
            "/in-game/option",
            internal_reqs::SetOption{world, ch->id, {{static_cast<uint8_t>(option), next}}});
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

template <>
async::task<bool> update_option<fb::protocol::CLIENT_VERSION::v651>::handle(
    fb::socket<character>&                                        session,
    game_reqs::update_option<fb::protocol::CLIENT_VERSION::v651>& request)
{
    auto ch = session.data();
    if (ch->inited() == false)
        co_return true;

    if (request.options.empty())
        co_return true;

    auto weak = ch->weak_from_this_as<character>();
    if (request.options.size() == 1 && request.options.front() == OPTION::EXTENSION)
    {
        if (request.ride)
        {
            if (ch->state() == STATE::RIDING)
                co_await ch->unride();
            else
                co_await ch->ride();
        }
        else
        {
            ch->update_option();
        }
    }
    else
    {
        auto world   = ch->world();
        auto current = weak.lock();
        auto changes = std::vector<internal::OptionChange>{};
        auto applied = std::vector<std::pair<OPTION, bool>>{};
        for (auto option : request.options)
        {
            if (current == nullptr)
                co_return true;

            if (option == OPTION::EXTENSION)
                continue;

            auto next = !current->option(option);
            if (option == OPTION::GROUP && !next)
            {
                auto group_id = current->group_id();
                if (group_id.has_value())
                {
                    try
                    {
                        co_await this->server.groups.handle_action(*current, current->name());
                    }
                    catch (std::exception& e)
                    {
                        fb::logger::warn("update_option: groups.handle_action failed: {}", e.what());
                    }

                    current = weak.lock();
                    if (current == nullptr)
                        co_return true;
                }
            }

            changes.emplace_back(static_cast<uint8_t>(option), next);
            applied.emplace_back(option, next);
        }

        if (changes.empty())
            co_return true;

        auto   uid  = current->id;
        auto&& resp = co_await this->server.http.post("internal",
                                                      "/in-game/option",
                                                      internal_reqs::SetOption{world, uid, std::move(changes)});
        co_await this->server.threads.switching(weak);
        current = weak.lock();
        if (current == nullptr)
            co_return true;

        if (resp.success == false)
            current->message(_TEXT(MESSAGE_OPTION_UPDATE_FAILED));

        for (auto& [option, next] : applied)
            current->option(option, next);
    }
    co_return true;
}

template class update_option<fb::protocol::CLIENT_VERSION::v550>;
template class update_option<fb::protocol::CLIENT_VERSION::v565>;
template class update_option<fb::protocol::CLIENT_VERSION::v651>;

} // namespace fb::game::handler::protocol
