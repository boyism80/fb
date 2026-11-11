#include <fb/game/handler/protocol/chat.h>

#include <fb/encoding.h>
#include <fb/game/server.h>

#include <json/json.h>

#include <memory>
#include <tuple>
#include <utility>

namespace game_reqs = fb::protocol::game::request;

namespace fb::game::handler::protocol {

template <fb::protocol::CLIENT_VERSION V>
chat<V>::chat(fb::game::server& server) :
    fb::handler::protocol<fb::game::server, game_reqs::chat<V>>(server)
{ }

template <fb::protocol::CLIENT_VERSION V>
async::task<bool> chat<V>::handle(fb::socket<character>& session, game_reqs::chat<V>& request)
{
    auto ch = session.data();
    if (ch == nullptr || ch->inited() == false)
        co_return true;

    auto weak = ch->weak_from_this_as<character>();
    auto map  = ch->map();
    if (map == nullptr)
        co_return true;

    if (ch->role() == ROLE::USER && ENUM_IN(map->model().option, MAP_OPTION::DISABLE_TALK))
        co_return true;

    if (co_await try_command(ch, weak, request))
        co_return true;

    // Re-fetch map after switching in try_command; character may have changed map or disconnected
    map = ch->map();
    if (map == nullptr)
        co_return true;

    handle_normal_chat(ch, request, map);

    if (request.message.empty() || request.message[0] != '/')
    {
        auto lua = this->server.lua.open("scripts/interaction.lua", "on_npc_chat");
        if (lua)
        {
            lua->pushobject(ch);
            lua->pushstring(request.message);
            lua->pushboolean(request.shout);
            fb::lua::run_async(std::move(lua), 3);
        }
    }
    co_return true;
}

template <fb::protocol::CLIENT_VERSION V>
async::task<bool> chat<V>::try_command(character* ch, std::weak_ptr<character> weak, game_reqs::chat<V>& request)
{
    auto lua = this->server.lua.open("scripts/interaction.lua", "on_chat");
    if (!lua)
    {
        if (request.message.empty() == false && request.message[0] == '/')
        {
            fb::logger::warn("try_command: dropping slash command (lua unavailable) name={} msg={}",
                             ch->name(),
                             request.message);
            co_return true;
        }
        co_return false;
    }

    lua->pushobject(ch);
    lua->pushstring(request.message);
    auto results = co_await fb::lua::call(std::move(lua), weak, 2);

    if (weak.expired())
        co_return true;

    co_return results.empty() == false && results.front().is_true();
}

template <fb::protocol::CLIENT_VERSION V>
void chat<V>::handle_normal_chat(character* ch, game_reqs::chat<V>& request, const std::shared_ptr<fb::game::map>& map)
{
    if (map == nullptr)
        return;

    auto message = fb::model::table::blocked_word->filter(request.message);
    auto type    = request.shout ? CHAT_TYPE::SHOUT : CHAT_TYPE::NORMAL;
    ch->chat(message, type, true);
    ch->log_chat(message, request.shout ? "shout" : "normal");
}

template class chat<fb::protocol::CLIENT_VERSION::v550>;
template class chat<fb::protocol::CLIENT_VERSION::v565>;
template class chat<fb::protocol::CLIENT_VERSION::v651>;

} // namespace fb::game::handler::protocol
