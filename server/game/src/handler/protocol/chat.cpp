#include <fb/game/handler/protocol/chat.h>
#include <fb/game/server.h>
#include <fb/encoding.h>
#include <json/json.h>

using namespace fb::game::handler::protocol;

namespace game_reqs = fb::protocol::game::request;

chat::chat(fb::game::server& server) :
    fb::handler::protocol<fb::game::server, game_reqs::chat>(server)
{ }

async::task<bool> chat::handle(fb::socket<character>& session, game_reqs::chat& request)
{
    auto ch = session.data();
    if (ch->inited() == false)
        co_return true;

    auto weak = ch->weak_from_this_as<character>();
    auto map  = ch->map();
    if (map == nullptr)
        co_return true;

    if (ch->role() == ROLE::USER && ENUM_IN(map->model.option, MAP_OPTION::DISABLE_TALK))
        co_return true;

    if (co_await try_command(ch, weak, request))
        co_return true;

    // Re-fetch map after switching in try_command; character may have changed map or disconnected
    map = ch->map();
    if (map == nullptr)
        co_return true;

    handle_normal_chat(ch, request, map);
    co_return true;
}

async::task<bool> chat::try_command(character* ch, std::weak_ptr<character> weak, game_reqs::chat& request)
{
    auto lua = fb::lua::new_context(nullptr, {.auto_release = false});
    if (lua == nullptr)
        co_return false;

#if defined DEBUG || defined _DEBUG
    lua->load("scripts/interaction.lua");
    lua->load("scripts/server.lua");
    lua->load("scripts/command.lua");
    lua->load("scripts/npc.lua");
#endif
    lua->func("on_chat");
    lua->pushobject(ch);
    lua->pushstring(request.message);
    lua->pushboolean(request.shout);
    std::ignore = co_await lua->call(3);

    if (weak.expired())
    {
        lua->release();
        co_return true;
    }

    co_await this->server.threads.switching(weak);
    auto ptr = weak.lock();
    if (ptr == nullptr)
    {
        lua->release();
        co_return true;
    }

    auto stop = lua->toboolean(1);
    lua->release();

    if (stop == false)
        co_return false;

    auto log_data              = Json::Value();
    log_data["character_id"]   = static_cast<Json::Int64>(ptr->id);
    log_data["character_name"] = UTF8(ptr->name(), PLATFORM::WINDOWS);
    log_data["command"]        = UTF8(request.message, PLATFORM::WINDOWS);
    auto map                   = ptr->map();
    if (map != nullptr)
        log_data["map"] = map->model.id;
    this->server.log.write("command_execute", log_data);
    co_return true;
}

void chat::handle_normal_chat(character* ch, game_reqs::chat& request, const std::shared_ptr<fb::game::map>& map)
{
    if (map == nullptr)
        return;

    auto message = fb::model::table::blocked_word.filter(request.message);
    auto type    = request.shout ? CHAT_TYPE::SHOUT : CHAT_TYPE::NORMAL;
    ch->chat(message, type, true);

    auto log_data              = Json::Value();
    log_data["character_id"]   = static_cast<Json::Int64>(ch->id);
    log_data["character_name"] = UTF8(ch->name(), PLATFORM::WINDOWS);
    log_data["message"]        = UTF8(message, PLATFORM::WINDOWS);
    log_data["chat_type"]      = request.shout ? "shout" : "normal";
    log_data["map"]            = map->model.id;
    this->server.log.write("chat", log_data);
}
