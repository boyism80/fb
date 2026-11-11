#include <fb/game/handler/protocol/update_option.h>

#include <fb/amqp_route.h>
#include <fb/game/server.h>

#include <cstdint>
#include <tuple>
#include <utility>
#include <vector>

namespace game_reqs = fb::protocol::game::request;

namespace fb::game::handler::protocol {

template <fb::protocol::CLIENT_VERSION V>
update_option<V>::update_option(fb::game::server& server) :
    fb::handler::protocol<fb::game::server, game_reqs::update_option<V>, 1000, 10, true>(server)
{ }

template <fb::protocol::CLIENT_VERSION V>
async::task<bool> update_option<V>::handle(fb::socket<character>& session, game_reqs::update_option<V>& request)
{
    auto ch = session.data();
    if (ch == nullptr || ch->inited() == false)
        co_return true;

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

    case OPTION::SUPER_HIDE:
    case OPTION::VISIBLE_HELMET:
        break;

    default:
        std::ignore = co_await ch->options.change({
            {option, !ch->options.enabled(option)}
        });
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
    if (ch == nullptr || ch->inited() == false)
        co_return true;

    if (request.options.empty())
        co_return true;

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
        auto changes = std::vector<std::pair<OPTION, bool>>{};
        for (auto option : request.options)
        {
            if (option == OPTION::EXTENSION || option == OPTION::SUPER_HIDE)
                continue;

            changes.emplace_back(option, !ch->options.enabled(option));
        }

        std::ignore = co_await ch->options.change(std::move(changes));
    }
    co_return true;
}

template class update_option<fb::protocol::CLIENT_VERSION::v550>;
template class update_option<fb::protocol::CLIENT_VERSION::v565>;
template class update_option<fb::protocol::CLIENT_VERSION::v651>;

} // namespace fb::game::handler::protocol
