#include <fb/game/handler/protocol/dialog.h>

#include <fb/game/item/base.h>
#include <fb/game/server.h>

#include <string>
#include <vector>

namespace fb::game::handler::protocol {

template <fb::protocol::CLIENT_VERSION V>
dialog<V>::dialog(fb::game::server& server) :
    fb::handler::protocol<fb::game::server, game_reqs::dialog<V>>(server)
{ }

template <fb::protocol::CLIENT_VERSION V>
lua_Integer dialog<V>::selected(const std::vector<std::string>& choices, const game_reqs::dialog<V>& request)
{
    auto count = static_cast<lua_Integer>(choices.size());
    if constexpr (V == fb::protocol::CLIENT_VERSION::v651)
    {
        if (request.type == fb::game::dialog::type::BUY)
        {
            auto found = static_cast<lua_Integer>(request.id);
            return found < 1 || found > count ? 0 : found;
        }
    }

    for (lua_Integer i = 0; i < count; i++)
    {
        if (choices[i].empty() == false && choices[i] == request.name)
            return i + 1;
    }

    return 0;
}

template <fb::protocol::CLIENT_VERSION V>
async::task<bool> dialog<V>::handle(fb::socket<character>& session, game_reqs::dialog<V>& request)
{
    auto ch = session.data();
    if (ch == nullptr || ch->inited() == false)
        co_return true;

    auto expected = fb::game::dialog::reply::MENU;
    switch (request.type)
    {
    case fb::game::dialog::type::INPUT:
    case fb::game::dialog::type::INPUT_NO_EXT:
        expected = fb::game::dialog::reply::INPUT_EXT;
        break;

    case fb::game::dialog::type::SLOT:
    case fb::game::dialog::type::SPELL:
        expected = fb::game::dialog::reply::SLOT;
        break;

    case fb::game::dialog::type::ITEM:
    case fb::game::dialog::type::PURSUIT:
        expected = fb::game::dialog::reply::ITEM;
        break;

    case fb::game::dialog::type::BUY:
        expected = fb::game::dialog::reply::BUY;
        break;

    default:
        break;
    }

    auto taken = ch->dialog.take(expected);
    auto lua   = taken.context;
    if (lua == nullptr)
        co_return true;

    auto reply = request;
    co_await lua->switching();

    switch (reply.type)
    {
    case fb::game::dialog::type::INPUT:
    case fb::game::dialog::type::INPUT_NO_EXT:
        lua->pushstring(reply.message);
        lua->resume(1);
        break;

    case fb::game::dialog::type::MENU:
    case fb::game::dialog::type::MENU_NO_EXT:
        lua->pushinteger(reply.index);
        lua->resume(1);
        break;

    case fb::game::dialog::type::SLOT:
    case fb::game::dialog::type::SPELL:
        lua->pushinteger(reply.index);
        lua->resume(1);
        break;

    case fb::game::dialog::type::ITEM:
    case fb::game::dialog::type::PURSUIT:
    {
        auto found = this->selected(taken.choices, reply);
        if (found != 0)
            lua->pushinteger(found);
        else
            lua->pushnil();

        lua->resume(1);
        break;
    }

    case fb::game::dialog::type::BUY:
    {
        // Yields: selected (1-based index or nil), count (v651 NEW quantity; else nil)
        auto found = this->selected(taken.choices, reply);
        if (found != 0)
        {
            lua->pushinteger(found);
            if constexpr (V == fb::protocol::CLIENT_VERSION::v651)
                lua->pushinteger(reply.count);
            else
                lua->pushnil();

            lua->resume(2);
        }
        else
        {
            lua->pushnil();
            lua->resume(1);
        }

        break;
    }

    default:
        lua->reject("unsupported dialog type");
        break;
    }

    co_return true;
}

template class dialog<fb::protocol::CLIENT_VERSION::v550>;
template class dialog<fb::protocol::CLIENT_VERSION::v565>;
template class dialog<fb::protocol::CLIENT_VERSION::v651>;

} // namespace fb::game::handler::protocol
